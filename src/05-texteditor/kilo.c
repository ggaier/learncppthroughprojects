#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/
/*
Holding down Control key while pressing another key zeroed the leftmost two
bits of the seven bits in the generated ASCII character.
所以才和0x1f(00011111)相与, 用来表示ctrl+按键的效果,
同时也包含在了ASCII的前三十二个控制字符中.
*/
#define CTRL_KEY(k) ((k)&0x1f)

/*** data ***/
struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/
void die(const char* s) {
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
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");
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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  // read(): posix接口, 从文件描述符中读出指定字节的数据到缓存中
  //从标准输入中读取一个字节大小的数据, 存储到c中. 如果没有读取到数据,
  //就返回0
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    //判断EAGAIN错误的原因: 在cygwin运行环境中, read()方法超时, 会返回-1,
    //并把errno 设置为EAGAIN. 所以这里不把EAGAIN当成是错误.
    // errno: 上次的错误代码.
    if (nread == 1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

int getCursorPosition(int* rows, int* cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  //从string中读取格式化的数据, 并把它们存储在提供的参数中.
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

int getWindowSize(int* rows, int* columns) {
  struct winsize ws;
  // ioctl 用来操作某些文件的底层设备参数,
  // 尤其是比如终端(terminals)可以被ioctl()请求控制.
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    //这里是一个fallback function, 当ioctl获取失败的时候,
    //使用光标位置的方式来计算总共有多少行列
    return getCursorPosition(rows, columns);
  } else {
    *rows = ws.ws_row;
    *columns = ws.ws_col;
    return 0;
  }
}

/*** append buffer ***/
struct abuf {
  char* b;
  int len;
}
#define ABUF_INIT \
  { NULL, 0 }

void abAppend(struct abuf *ab, const char *s, int len){
  char* new = realloc(ab->b, ab->len + len);
  if (new == NULL)
    return;
  // string.h memcpy(str1, str2, n): 把str2中的数据复制到str1中,
  // 被复制的字节数为n
  //该方法把字符串s复制到new 字符串数组的末尾.
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf* ab) {
  free(ab->b);
}

/*** output ***/
void editorDrawRows(struct abuf* ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    abAppend(ab, "~", 1);

    if (y < E.screenrows - 1) {
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  struct abuf ab = ABUF_INIT;
  //写四个字节的数据到STDOUT_FILENO, 也就是标准输出
  //四个字节分别是"\x1b"表示的数字27, 也就是Escapse键
  //剩下的三个字节分别是[2J.
  // escapse: \xnn 表示任何的16进制数, 后边两位.
  //在terminal中, escape+[表示的是control sequence
  // 2J: 表示的是清空全部屏幕.
  // https://vt100.net/docs/vt100-ug/chapter3.html#ED
  //这篇文章使用了VT100的控制序列. ncurses库是另外一个更好的选择.
  abAppend(&ab, "\x1b[2J", 4);

  //清空屏幕后, 光标位置移动到了屏幕的下方
  //这个方法把光标重新放置在左上角.
  // H: 是用来设置光标位置的. 语法格式是row;columnH, 默认是<esc>[1;1H
  //就是把光标设置到第一行第一列, 这里是使用了默认值.
  //行和列的起始值是从1开始, 不是0.
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);

  //绘制完行起始符号之后, 重新把cursor放置到开始的位置.
  abAppend(&ab, "\x1b[H", 3);
  
  //把数组内容输出到屏幕上.
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

/*** input ***/
void editProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(EXIT_SUCCESS);
      break;
    default:
      break;
  }
}

/*** init ***/
void initEditor() {
  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
}

int main(int argc, char const* argv[]) {
  // terminal有两种不同的模式, 一种是cooked mode, 输入数据会被预先处理,
  // 然后再统一给应用程序. 也就是一次性给一行; 另外一种是raw mode,
  //用户输入什么数据, terminal就给应用什么样的数据.
  //所以才需要开启Raw mode.
  enableRawMode();
  initEditor();

  while (1) {
    editorRefreshScreen();
    editProcessKeypress();
  }
  return 0;
}
