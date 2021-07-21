// feature test macros: 特性测试宏. 用来控制编译的程序所包含的特性.
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** defines ***/
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3
/*
Holding down Control key while pressing another key zeroed the leftmost two
bits of the seven bits in the generated ASCII character.
所以才和0x1f(00011111)相与, 用来表示ctrl+按键的效果,
同时也包含在了ASCII的前三十二个控制字符中.
*/
#define CTRL_KEY(k) ((k)&0x1f)

enum editorKey {
  BACK_SPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY
};

/*** data ***/
typedef struct erow {
  int size;
  // render中字符串的长度.
  int rsize;
  char *chars;
  //实际上要绘制的一行文字.
  char *render;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  //总共有多少行.
  int numrows;
  //多行的字符串.
  erow *row;
  int dirty;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct termios orig_termios;
};

struct editorConfig E;

/*** prototypes ***/
// variadic funtion, 可变参数方法, va_start, va_arg, va_end, 来获取可变参数
void editorSetStatusMessage(const char *fmt, ...);

/*** terminal ***/
void die(const char *s) {
  //退出编辑器的时候, 同时清空屏幕
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(EXIT_FAILURE);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  //读取和设置terminal的attributes
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  //当应用进程被结束的时候, 制动会执行方法指针所指向的函数.
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  // c_lflag: Local mode
  //关闭echo 模式, 和 cooked mode(canonical模式).
  /*
  In canonical mode:
  Input is made available line by line. An input line is available when one of
  the line delimiters is typed
  ISIG: When any of the characters INTR, QUIT, SUSP, or DSUSP are received,
  generate the corresponding signal.
  */
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  /*
  IXON: Enable XON/XOFF flow control on output. 关闭ctrl+s, ctrl+q的响应.
  ICRNL: CR-> carriage return; NL-> new line;
  */
  raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= ~(CS8);

  // VMIN: 一次性最少读取的字节数, 在非cooked mode 模式下.
  raw.c_cc[VMIN] = 0;
  // VTIME: 非canonical 读取的情况下超时时间, 单位为1/10s.
  raw.c_cc[VTIME] = 1;
  // tcsetattr: 第二个参数表示的是新的attr生效的时机.
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
  int nread;
  char c;
  // read(): posix接口, 从文件描述符中读出指定字节的数据到缓存中
  //从标准输入中读取一个字节大小的数据, 存储到c中. 如果没有读取到数据,
  //就返回0
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    //判断EAGAIN错误的原因: 在cygwin运行环境中, read()方法超时, 会返回-1,
    //并把errno 设置为EAGAIN. 所以这里不把EAGAIN当成是错误.
    // errno: 上次的错误代码.
    if (nread == 1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    //读取两个字节到seq中.
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    // write(STDOUT_FILENO, str, strlen(str));
    //方向键会产生escape队列, 格式分别是'[A','[B','[C','[D'
    //按照左上右下的顺序分别是: D, A, C, B
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1':
              return HOME_KEY;
            case '3':
              return DEL_KEY;
              break;
            case '4':
              return END_KEY;
            case '5':
              return PAGE_DOWN;
            case '6':
              return PAGE_UP;
            case '7':
              return HOME_KEY;
            case '8':
              return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A':
            return ARROW_UP;
          case 'B':
            return ARROW_DOWN;
          case 'C':
            return ARROW_RIGHT;
          case 'D':
            return ARROW_LEFT;
          case 'H':
            return HOME_KEY;
          case 'F':
            return END_KEY;
        }
      }
    } else if (seq[0] == '0') {
      switch (seq[1]) {
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
      }
    }
    return '\x1b';
  }

  return c;
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  //从string中读取格式化的数据, 并把它们存储在提供的参数中.
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  return 0;
}

int getWindowSize(int *rows, int *columns) {
  struct winsize ws;
  // ioctl 用来操作某些文件的底层设备参数,
  // 尤其是比如终端(terminals)可以被ioctl()请求控制.
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    //这里是一个fallback function, 当ioctl获取失败的时候,
    //使用光标位置的方式来计算总共有多少行列
    return getCursorPosition(rows, columns);
  } else {
    *rows = ws.ws_row;
    *columns = ws.ws_col;
    return 0;
  }
}

/*** row operations ***/
int editorRowCxToRx(erow *row, int cx) {
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
    }
    rx++;
  }
  return rx;
}

void editorUpdateRow(erow *row) {
  int tabs = 0;
  int j;
  for (j = 0; j < row->rsize; j++) {
    if (row->chars[j] == '\t') tabs++;
  }
  free(row->render);
  //这里tab*7,是因为在row->size中计算字符串长度时, 已经把'\t'
  //算了一个字节的大小了.
  row->render = malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1);

  //把要绘制的文字拷贝给render.
  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      //这是因为一个tab采用8个空格渲染.
      while (idx % KILO_TAB_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorAppendRow(char *s, size_t len) {
  //由于要新增一行字符串, 所以要把E.row数组扩大一个元素.
  E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

  //把新增的字符串加入到数组的末尾.
  int at = E.numrows;
  E.row[at].size = len;
  //被数组中最后一个字符串申请内存
  E.row[at].chars = malloc(len + 1);
  //然后把新增字符串拷贝到刚才申请的内存中.
  memcpy(E.row[at].chars, s, len);
  //添加结束符号.
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  editorUpdateRow(&E.row[at]);
  E.numrows++;
  E.dirty++;
}

void editorRowInsertChar(erow *row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  //从src中拷贝指定数量的字符串到dst
  //之所以选择memmove, 而不是memcpy, 是因为memmove
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  //然后在at位置插入一个字符.
  row->chars[at] = c;
  editorUpdateRow(row);
}

void editorRowDelChar(erow *row, int at) {
  if (ac < 0 || at > row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}

/*** editor operations ***/
void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    //新增一行长度为0字符串
    editorAppendRow("", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
  E.dirty++;
}

void editorDelChar() {
  if (E.cy == E.numrows) return;
  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    if (E.cx > 0) {
      editorRowDelChar(row, E.cx - 1);
      E.cx--;
    }
  }

  /*** file i/o ***/

  char *editorRowsToString(int *buflen) {
    int totlen = 0;
    int j;
    for (j = 0; j < E.numrows; j++) {
      totlen += E.row[j].size + 1;
    }
    *buflen = totlen;

    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < E.numrows; j++) {
      memcpy(p, E.row[j].chars, E.row[j].size);
      p += E.row[j].size;
      //指针移动到最后一位, 然后加上一个换行符.
      *p = '\n';
      p++;
    }
    return buf;
  }

  void editorOpen(char *filename) {
    free(E.filename);
    // strdup(const char* s): 复制一份字符串, 并返回指针.
    E.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    // getline(char* lineptr, size_t* n, FILE * stream)
    //从指定的stream中读取一行数据, 并存储在*lineptr中, 这是一个POSIX的接口.
    //返回读取到的字节数. 把*lineptr 设置为空指针, n设置为0, 也是允许的,
    //会使用推荐的 方式读取这个文件.
    //这个方法在没有兼容POSIX接口的设备上, 会出现问题.
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
      while (linelen > 0 &&
             (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
        linelen--;
      editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
  }

  void editorSave() {
    if (E.filename == NULL) return;
    int len;
    char *buf = editorRowsToString(&len);
    // POSIX方法.
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
      if (ftruncate(fd, len) != -1) {
        if (write(fd, buf, len) == len) {
          close(fd);
          free(buf);
          E.dirty = 0;
          editorSetStatusMessage("%d bytes written to disk", len);
          return;
        }
      }
      close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save I/O error: %s", strerror(errno));
  }

  /*** append buffer ***/
  struct abuf {
    char *b;
    int len;
  };
#define ABUF_INIT \
  { NULL, 0 }

  void abAppend(struct abuf * ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    // string.h memcpy(str1, str2, n): 把str2中的数据复制到str1中,
    // 被复制的字节数为n
    //该方法把字符串s复制到new 字符串数组的末尾.
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
  }

  void abFree(struct abuf * ab) { free(ab->b); }

  /*** output ***/
  void editorScroll() {
    E.rx = 0;
    if (E.cy < E.numrows) {
      E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }
    // cy是光标的行数. 这里在修正rowoff
    //检查光标是否在window上方, 如果是, 就减小rowoff
    if (E.cy < E.rowoff) {
      E.rowoff = E.cy;
    }

    //检查光标是否在window下方, 如果是就增大rowoff.
    if (E.cy >= E.rowoff + E.screenrows) {
      E.rowoff = E.cy - E.screenrows + 1;
    }

    if (E.rx < E.coloff) {
      E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
      E.coloff = E.rx - E.screencols - 1;
    }
  }

  void drawWelcomeMessage(struct abuf * ab) {
    char welcome[80];
    //组合一个格式化的字符串, 存储到参数welcome中, 而不是打印到标准输出.
    int welcomelen = snprintf(welcome, sizeof(welcome),
                              "Kilo editor -- version %s", KILO_VERSION);
    if (welcomelen > E.screencols) welcomelen = E.screencols;
    int padding = (E.screencols - welcomelen) / 2;
    if (padding) {
      abAppend(ab, "~", 1);
      padding--;
    }
    while (padding--) {
      abAppend(ab, " ", 1);
    }
    abAppend(ab, welcome, welcomelen);
  }

  void editorDrawRows(struct abuf * ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
      int filerow = y + E.rowoff;
      if (filerow >= E.numrows) {
        //在屏幕的1/3处, 绘制一个欢迎信息.
        if (E.numrows == 0 && y == E.screenrows / 3) {
          drawWelcomeMessage(ab);
        } else {
          abAppend(ab, "~", 1);
        }
      } else {
        int len = E.row[filerow].rsize - E.coloff;
        if (len < 0) len = 0;
        if (len > E.screencols) len = E.screencols;
        abAppend(ab, &E.row[filerow].render[E.coloff], len);
      }

      //清除一行, 这样的话, 可以移除清空屏幕的命令了.
      //这样的话, 会更加的高效.
      abAppend(ab, "\x1b[K", 3);
      abAppend(ab, "\r\n", 2);
    }
  }

  void editorDrawStatusBar(struct abuf * ab) {
    // m命令能够让打印出的文字带有各种属性, 比如加粗(1), 下划线(4), 闪烁(5),
    // 反转色(7)
    abAppend(ab, "\x1b[7m", 4);

    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                       E.filename ? E.filename : "[No Name]", E.numrows,
                       E.dirty ? "(modified)" : "");
    //当前行数的格式化字符串.
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numrows);
    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);

    while (len < E.screencols) {
      //把当前行状态至于屏幕最右侧
      if (E.screencols - len == rlen) {
        abAppend(ab, rstatus, rlen);
        break;
      } else {
        abAppend(ab, " ", 1);
        len++;
      }
    }

    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
  }

  void editorDrawMessageBar(struct abuf * ab) {
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
      abAppend(ab, E.statusmsg, msglen);
  }

  void editorRefreshScreen() {
    editorScroll();

    struct abuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);
    //写四个字节的数据到STDOUT_FILENO, 也就是标准输出
    //四个字节分别是"\x1b"表示的数字27, 也就是Escapse键
    //剩下的三个字节分别是[2J.
    // escapse: \xnn 表示任何的16进制数, 后边两位.
    //在terminal中, escape+[表示的是control sequence
    // 2J: 表示的是清空全部屏幕.
    // https://vt100.net/docs/vt100-ug/chapter3.html#ED
    //这篇文章使用了VT100的控制序列. ncurses库是另外一个更好的选择.
    // abAppend(&ab, "\x1b[2J", 4);

    //清空屏幕后, 光标位置移动到了屏幕的下方
    //这个方法把光标重新放置在左上角.
    // H: 是用来设置光标位置的. 语法格式是row;columnH, 默认是<esc>[1;1H
    //就是把光标设置到第一行第一列, 这里是使用了默认值.
    //行和列的起始值是从1开始, 不是0.
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    //移动cursor到指定的cx和cy位置.
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
             (E.rx - E.coloff) + 1);
    //返回字符串的长度.
    abAppend(&ab, buf, strlen(buf));

    //这个和上边的l命令是用了隐藏和显示光标的. 但是终端不一定会支持这个功能.
    abAppend(&ab, "\x1b[?25h", 6);

    //把数组内容输出到屏幕上.
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
  }

  void editorSetStatusMessage(const char *fmt, ...) {
    //用来获取一个方法的额外的参数.
    // va: variadic
    va_list ap;
    va_start(ap, fmt);
    //从vlist中加载数据, 并转化成之身穿, 并写入到对应的fmt位置中.
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    //获取当前时间
    E.statusmsg_time = time(NULL);
  }

  /*** input ***/
  void editorMoveCursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {
      case ARROW_LEFT:
        if (E.cx != 0) {
          E.cx--;
        } else if (E.coloff > 0) {
          //如果在行头, 继续左移, 会移动到上一行的末尾.
          E.cy--;
          E.cx = E.row[E.cy].size;
        }
        break;
      case ARROW_RIGHT:
        //限制光标移动超出一行的末尾.
        if (row && E.cx < row->size) {
          E.cx++;
        } else if (E.cx == row->size) {
          E.cy++;
          E.cx = 0;
        }
        break;
      case ARROW_UP:
        if (E.cy != 0) {
          E.cy--;
        }
        break;
      case ARROW_DOWN:
        if (E.cy < E.numrows) {
          E.cy++;
        }
        break;
    }
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
      E.cx = rowlen;
    }
  }

  void editorProcessKeypress() {
    static int quit_times = KILO_QUIT_TIMES;

    int c = editorReadKey();
    switch (c) {
      case '\r':
        /* TODO */
        break;
      case CTRL_KEY('q'):
        if (E.dirty && quit_times > 0) {
          editorSetStatusMessage(
              "WARNING!!! File has unsaved changes. Press Ctrl-q %d more times "
              "to quit",
              quit_times);
          quit_times--;
          return;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(EXIT_SUCCESS);
        break;
      case CTRL_KEY('s'):
        editorSave();
        break;
      case HOME_KEY:
        E.cx = 0;
        break;
      case END_KEY:
        if (E.cy < E.numrows) {
          E.cx = E.row[E.cy].size;
        }
        break;
      case BACK_SPACE:
      case CTRL_KEY('h'):
      case DEL_KEY:
        if (c == DEL_KEY) {
          editorMoveCursor(ARROW_RIGHT);
        }
        editorDelChar();
        break;
      case PAGE_UP:
      case PAGE_DOWN: {
        //使用PAGE_UP, PAGE_DOWN翻屏;
        if (c == PAGE_UP) {
          E.cy = E.rowoff;
        } else if (c == PAGE_DOWN) {
          E.cy = E.rowoff + E.screenrows - 1;
          if (E.cy > E.numrows) E.cy = E.numrows;
        }
        int times = E.screenrows;
        while (times--) {
          editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
      } break;
      case ARROW_LEFT:
      case ARROW_UP:
      case ARROW_RIGHT:
      case ARROW_DOWN:
        editorMoveCursor(c);
        break;
      case CTRL_KEY('l'):
      case '\x1b':
        // ctrl-l, ESC键置之不理.
        break;
      default:
        editorInsertChar(c);
        break;
    }
    quit_times = KILO_QUIT_TIMES;
  }

  /*** init ***/
  void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    //绘制每一行文字的时候, 留下最后两行用来显示statusbar.
    E.screenrows -= 2;
  }

  int main(int argc, char const *argv[]) {
    // terminal有两种不同的模式, 一种是cooked mode, 输入数据会被预先处理,
    // 然后再统一给应用程序. 也就是一次性给一行; 另外一种是raw mode,
    //用户输入什么数据, terminal就给应用什么样的数据.
    //所以才需要开启Raw mode.
    enableRawMode();
    initEditor();
    // main方法解释: argc, argument count, 表示的是argc表示的是传递给程序的参数
    // argv是后边的实际参数. 其中第一个参数是程序本身.
    if (argc >= 2) {
      editorOpen(argv[1]);
    }
    editorSetStatusMessage("HELP: Ctrl-s = save | Ctrl-Q = quit");

    while (1) {
      editorRefreshScreen();
      editorProcessKeypress();
    }
    return 0;
  }
