#include <unistd.h>

typedef char ALIGN[16];

void* malloc(size_t size) {
  void* block;
  // sbrk() Set the allocation break value
  /*The sbrk() function has been used in specialized cases where no other memory
   * allocation function provided the same capability. Use mmap() instead
   * because it can be used portably with all other memory allocation functions
   * and with any function that uses other allocation functions.*/
  block = sbrk(size);
  //如果sbrk申请内存失败, 会返回(void*)-1
  if (block == (void*)-1) return;

}