#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/
struct termios orig_termios;

/*** terminal ***/
void die(const char* s) {
  perror(s);
  exit(EXIT_FAILURE);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  //读取和设置terminal的attributes
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  //当应用进程被结束的时候, 制动会执行方法指针所指向的函数.
  atexit(disableRawMode);

  struct termios raw = orig_termios;
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

/*** init ***/
int main(int argc, char const* argv[]) {
  // terminal有两种不同的模式, 一种是cooked mode, 输入数据会被预先处理,
  // 然后再统一给应用程序. 也就是一次性给一行; 另外一种是raw mode,
  //用户输入什么数据, terminal就给应用什么样的数据.
  //所以才需要开启Raw mode.
  enableRawMode();
  char c;
  // read(): posix接口, 从文件描述符中读出指定字节的数据到缓存中
  //从标准输入中读取一个字节大小的数据, 存储到c中. 如果没有读取到数据,
  //就返回0
  while (1) {
    char c = '\0';
    // errno: 上次的错误代码.
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");
    //检查一个字符是否是control字符. 控制字符是不会显示在屏幕上的,
    //用来控制文本格式的字符.
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q')
      break;
  }
  return 0;
}
