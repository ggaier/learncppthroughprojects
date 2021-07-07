#include <stdio.h>   //fprintf, printf, stderr, getchar, perror
#include <stdlib.h>  //malloc, realloc, free, exit, execvp
#include <string.h>  //strcmp, strtok

// waitpid()和相关的宏
#include <sys/wait.h>

#include <unistd.h>  //chdir, fork, exec, pid_t

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*
为什么char** 可以用来表示字符串数组.
首先字符串在c中的表示方式是: char str[] = "abcd";
由于array在C和C++中会被隐式的表达为指针, 所以也可以写成: char* str = "abcd"
所以一个字符串数据就可以表示成 char* * strArray = {"abcd", "efg", "hij"}
*/

char* lsh_read_line();
char** lsh_split_line();
int lsh_execute(char** args);

int lsh_cd(char** args);
int lsh_help(char** args);
int lsh_exit(char** args);

char* builtin_str[] = {"cd", "help", "exit"};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char*);
}

void lsh_loop() {
  char* line;
  char** args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char* lsh_read_line() {
#ifdef LSH_USE_STD_GETLINE
  char* line = NULL;
  // signed int
  ssize_t bufsize = 0;
  // getline: 从输入流中取出字符串, 并存储在str中, 知道出现
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;

  char* buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    //向指定的stream中写入格式化后的string
    fprintf(stderr, "lsh: allocation error \n");
    exit(EXIT_FAILURE);
  }
  while (1) {
    c = getchar();
    if (c == EOF) {
      //退出当前进程, 并执行清理程序
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    //检查是否刚才申请的内存空间足够, 如果不够, 需要重新申请.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

char** lsh_split_line(char* line) {
  int bufsize = LSH_TOK_BUFSIZE;
  int position = 0;
  char** tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  // strtok spilit string into tokens.
  //依照给定的界定符, 分割匹配到的连续字符串.
  printf("split line: %s\n", line);
  token = strtok(line, LSH_TOK_DELIM);
  printf("split line2222 token: %s\n", token);
  while (token != NULL) {
    tokens[position] = token;
    position++;
    if (position >= bufsize) {
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, LSH_TOK_DELIM);
    printf("token: %s\n", token);
  }
  tokens[position] = NULL;
  return tokens;
}

//这是一个方法指针数组, 通过&funcName的方式, 获取到方法指针
//通过方法指针调用方法有两种方式:
// 1. 显示解引用: (*fcnPtr)(args)
// 2. 隐式解引用: fcnPtr(args)
int (*builtin_func[])(char**) = {&lsh_cd, &lsh_help, &lsh_exit};

int lsh_cd(char** args) {
  printf("lsh cd");
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"n");
  } else {
    // chdir: change working directory., 一个posix 接口, 如果切换当前目录成功,
    //返回值为0.
    printf("cd path: %s", args[1]);
    if (chdir(args[1]) != 0) {
      //打印错误日志.
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char** args) {
  int i;

  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Use the man command for information on other programs.\n");
}

int lsh_exit(char** args) {
  return 0;
}

//启动shell.
int lsh_launch(char** args) {
  printf("lsh launch\n");
  pid_t pid;
  int status;

  //创建一个子进程, 成功的话, 父进程返回进程的id, 子进程返回0. 失败的话,
  //父进程返回-1, 没有子进程被创建成功.
  //父子进程在创建成功后会运行在不同的内存空间, 但是有一样的内存内容,
  //因为子进程基本完全就是父进程的拷贝复制. 由于创建进程的时候,
  //父子进程都会执行fork()命令的下一个指令. 所以区分出父子进程是非常重要的.
  //而区分父子进程, 可以通过fork()命令的返回值来确定.
  pid = fork();
  if (pid == 0) {
    //此时表示的是子进程正在运行, 所以执行命令.
    //创建的子进程可以不必执行父进程的程序. 这就是exec类的系统调用的用途了.
    //它允许一个进程执行任何的程序文件, 包括二进制文件,或者是脚本文件
    // execvp包含两个参数, 第一个是待执行的文件的名字. 第二个字符串数组的指针,
    // 也就是char**. 如果执行失败了, 就返回-1.
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    //小于0, 表示进程创建失败.
    perror("lsh");
  } else {
    //大于0, 表示父进程返回了子进程的id, 此时父进程正在执行.
    do {
      //父进程需要等待子进程的执行结果.
      // waitpid()等在子进程的状态变化.
      waitpid(pid, &status, WUNTRACED);
      //当waitpid()中的子进程状态发生变化的时候, 可以通过几个宏来检查这个状态
      // WIFEXITED(status): 如果是子进程正常结束, 返回true
      // WIFSIGNALED(status): 如果是子进程被信号结束.
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

int lsh_execute(char** args) {
  int i;
  if (args[0] == NULL) {
    return 1;
  }
  printf("lsh execute111: %s\n", args[0]);

  for (i = 0; i < lsh_num_builtins(); i++) {
    //比较两个字符串. 如果相等, 返回0, 如果不等, 返回0或者大于0, 比较的是ASCII码
    printf("lsh execute222: %s\n", builtin_str[i]);
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return builtin_func[i](args);
    }
  }
  return lsh_launch(args);
}

int main(int argc, char const* argv[]) {
  // 1. 加载配置文件, 如果有的话

  // 2. 执行命令循环
  lsh_loop();

  // 3. 执行关闭和清理操作
  return 0;
}
