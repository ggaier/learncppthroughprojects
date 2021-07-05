//按照Google的C++风格, header guard <PROJECT>_<PATH>_<FILE>_H_
#ifndef TINYSTL_CONSTRUCTOR_H_
#define TINYSTL_CONSTRUCTOR_H_

#include <new>
#include "utils.hpp"

#ifdef _MSG_VER

#endif  //_MSG_VER

namespace mystl {
// construct 构造对象.
template <class T>
void construct(T* ptr) {
  //这个方法实际上是调用的void* operator new (std::size_t size, void* ptr)
  // noexcept; 它是placemeng new 方法,
  //它是在已经分配内存的地方创建一个对象的时候, 才会使用该方法.

  /*
  placement new：只是operator
  new重载的一个版本。它并不分配内存，只是返回指向已经分配好的某段内存的一个指针。因此不能删除它，但需要调用对象的析构函数。
  如果你想在已经分配的内存中创建一个对象，使用new时行不通的。也就是说placement
  new允许你在一个已经分配好的 内存中（栈或者堆中）构造一个新的对象。原型中void*
  p实际上就是指向一个已经分配好的内存缓冲区的的首地址。
  */
  new ((void*)ptr) T();
}

template <class T1, class T2>
void construct(T1* ptr, const T2* value) {
  ::new ((void*)ptr) T1(value)
}

template <class Ty, class... Args>
// Foo&& 这个是RValue 引用, 而不是引用的引用.
void construct(Ty* ptr, Args&&... args) {
  //...用在class 后边, 表示可变参数; 用在变量后面, 表示展开可变参数.
  ::new ((void*)ptr) Ty(mystl::forward(args)...)
}

}  // namespace mystl

#endif  // FOO_BAR_BAZ_H_