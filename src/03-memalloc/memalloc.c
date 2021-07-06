#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef char ALIGN[16];

// union 和 struct的语法相似, 但是功能却完全不同.
//在union中, 它的所有的成员共享同一块物理内存地址,
//这块内存地址的大小取决于最大的成员.
//所以当前这个union的大小为16个字节.
union header {
  struct {
    size_t size;
    // unsigned == unsigned int
    unsigned is_free;
    union header* next;
  } s;
  ALIGN stub;
};

typedef union header header_t;

//定义链表的头部和尾部.
header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;

//从链表中查找出已经标记为free的内存区域, 并返回
header_t* get_free_block(size_t size) {
  header_t* curr = head;
  while (curr) {
    //查找出可用的free block
    if (curr->s.is_free && curr->s.size >= size) return curr;
    curr = curr->s.next;
  }
  return NULL;
}

//申请内存.
void* malloc(size_t size) {
  size_t total_size;
  void* block;
  header_t* header;

  if (!size) return NULL;

  pthread_mutex_lock(&global_malloc_lock);
  //链表的遍历查询需要加线程同步保护.
  header = get_free_block(size);
  if (header) {
    header->s.is_free = 0;
    pthread_mutex_unlock(&global_malloc_lock);
    //查找到可用的内存之后, 通过header+1, 返回实际的可用堆区内存的起始指针
    return (void*)(header + 1);
  }
  //如果从当前的链表中没有查找到可用的, 已经申请的内存空间,
  //那就需要从系统中申请对应大小的内存了.
  total_size = sizeof(header_t) + size;
  // sbrk() Set the allocation break value
  /*The sbrk() function has been used in specialized cases where no other memory
   * allocation function provided the same capability. Use mmap() instead
   * because it can be used portably with all other memory allocation functions
   * and with any function that uses other allocation functions.*/
  // sbrk()方法也是线程不安全的.
  // sbrk()返回的是前一个break 值
  //关于break:The break is the first address after the end of the process's
  // uninitialized data segment (also known as the "BSS").
  //所以如果从头开始申请内存的话,
  // brk指向的永远是上一次申请heap区内存的指针的末尾.
  block = sbrk(total_size);
  //如果sbrk申请内存失败, 会返回(void*)-1
  if (block == (void*)-1) {
    //不要忘记这里释放互斥锁.
    pthread_mutex_unlock(&global_malloc_lock);
    return NULL;
  }
  header = block;
  header->s.size = size;
  header->s.is_free = 0;
  header->s.next = NULL;

  //如果还不存在链表头, 表示第一次申请, 那么新申请的header应该作为链表头.
  if (!head) {
    head = header;
  }
  //如果已经存在链表, 则把新申请的作为链表的尾部加入到链表中.
  if (tail) {
    tail->s.next = header;
  }
  //同时把尾指针指向新申请的内存区.
  tail = header;
  return (void*)(header + 1);
}

//释放内存
void free(void* block) {
  header_t *header, *tmp;
  void* programbreak;
  if (!block) return;
  pthread_mutex_lock(&global_malloc_lock);
  //指针的加减是依照指针所指元素类型的大小为单位的.
  //所以这里转化成header指针之后, 减去一个单位, 刚好指向了header_t的开始的地址.
  header = (header_t*)block - 1;
  // sbrk() is sometimes used to monitor heap use by calling with an argument of
  // 0. The result is unlikely to reflect actual utilization in combination with
  // an mmap(2) based malloc.
  programbreak = sbrk(0);

  //判断要释放的block是否是在链表的结尾. 如果是, 则释放这块内存.
  if ((char*)block + header->s.size == programbreak) {
    if (head == tail) {
      header = tail = NULL;
    } else {
      tmp = head;
      while (tmp) {
        //只能释放链表结尾的一块内存.
        //所以这里的内存释放不够高效.
        if (tmp->s.next == tail) {
          tmp->s.next = NULL;
        }
        tmp = tmp->s.next;
      }
    }
    //收缩brk指针. sbrk()指针是线程不安全的, 所以这里可能出现问题.
    sbrk(0 - header->s.size - sizeof(header_t));
    pthread_mutex_unlock(&global_malloc_lock);
    return;
  }
  //如果无法是否链表结尾的内存, 那么就把该块内存在链表中标记为free, 可以被占用.
  header->s.is_free = 1;
  pthread_mutex_unlock(&global_malloc_lock);
}

//申请元素数组的内存空间.
void* calloc(size_t num, size_t nsize) {
  size_t size;
  void* block;
  if (!num || !nsize) return NULL;
  size = num * nsize;
  //检查是否乘法溢出
  if (nsize != size / num) return NULL;
  //申请指定大小字节的内存, 并返回这块内存的起始指针.
  block = malloc(size);
  if (!block) return NULL;
  //设置指定字节大小的内存为指定的值.
  //这里是把刚才申请的内存的值全部设置为0.
  memset(block, 0, size);
  return block;
}

//把已经申请的内存空间大小修改成给定的大小.
void* realloc(void* block, size_t size) {
  header_t* header;
  void* ret;
  //如果block是一个空指针, 或者size为0, 返回一个默认的实现.
  if (!block || !size) return malloc(size);
  header = (header_t*)block - 1;
  //如果要申请的内存大小小于当前block的内存大小, 直接返回该block
  if (header->s.size >= size) {
    return block;
  }
  ret = malloc(size);
  if (ret) {
    //从指定的内存地址, 拷贝指定字节大小的内存的值, 到指定的内存地址.
    //底层是二进制的数据拷贝.
    memcpy(ret, block, header->s.size);
    //解除内存分配. 之前由malloc, calloc, realloc 申请的内存块被解除分配
    //以便将来能够继续被重新申请.
    free(block);
  }
  return ret;
}

void print_mem_list() {
  header_t* curr;
  printf("header = %p, tail = %p \n", (void*)head, (void*)tail);
  while (curr) {
    printf("addr= %p, size = %zu, is_free = %u, next = %p\n", (void*)curr,
           curr->s.size, curr->s.is_free, (void*)curr->s.next);
    curr = curr->s.next;
  }
}