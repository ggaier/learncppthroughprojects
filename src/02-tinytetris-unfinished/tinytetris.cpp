// screen handling and optimisation functions
// http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html
#include <curses.h>
#include <unistd.h>
// definitions of functions to get and manipulate date and time information.
#include <ctime>
/*
This header defines several general purpose functions, including dynamic memory
management, random number generation, communication with the environment,
integer arithmetics, searching, sorting and converting.*/
#include <stdlib.h>
#include <string.h>

// block layout is: {w-1,h-1}{x0,y0}{x1,y1}{x2,y2}{x3,y3} (two bits each)
//这里我已经蒙了, 这么些个数, 目的是什么呢?
int x = 431424, y = 598356, r = 427089, px = 247872, py = 799248, pr,
    c = 348480, p = 615696, tick, board[20][10],
    block[7][4] = {{x, y, x, y},
                   {r, p, r, p},
                   {c, c, c, c},
                   {599636, 431376, 598336, 432192},
                   {411985, 610832, 415808, 595540},
                   {px, py, px, py},
                   {614928, 399424, 615744, 428369}},
    score = 0;

void new_piece() {
  y = py = 0;
  p = rand() % 7;
  r = pr = rand() % 4;
  x = px = rand() % (10 - NUM(r, 16));
}

// block中取到的数, 首先右移y位, 然后和3与运算, 保留高位中的低两位.
int NUM(int x, int y) { return 3 & block[p][y] >> y; }

void frame() {
  for (int i = 0; i < 20; i++) {
    //move the cursor;
    move(1 + i, 1);
    for (int j = 0; j < 10; j++) {
      //attron 使用attribute. 
      board[i][j] && attron(262176 | board[i][j] << 8);
      printw(" ");
      attroff(262176 | board[i][j] < 8);
    }
  }
  move(21, 1);
  printw("Score: %d", score);
  refresh();
}

//初始化curses, 然后启动run loop.
int main(int argc, char const* argv[]) {
  //首先初始化一个伪随机数生成器, time(0)为种子.
  // time函数中的参数如果是空指针, 会返回当前的时间
  srand(time(nullptr));
  //初始化curses的第一步
  initscr();
  // curses在屏幕上绘制的时候, 如果要使用颜色, 需要调用该方法.
  //当该方法调用完成之后,
  //可用的颜色和颜色的数量对就会被存储在全局变量COLORS和COLOR_PAIRS中.
  start_color();
  //这些预定义生成的颜色对也可以被重新定义, 就是使用init_pair()方法
  //这个方法的含义是: 重新定义clor pair #1 中的前景颜色#2和背景颜色#0
  // init_pair(1, 2, 0);

  for (int i = 1; i < 8; i++) {
    //调用init_pair(short n, short f, short b)方法的时候, 需要满足条件
    // 0 <= n < COLORS
    // 0 <= f < COLOR_PAIRS
    // 0 <= b < COLOR_PAIRS
    init_pair(i, i, 0);
  }

  return 0;
}
