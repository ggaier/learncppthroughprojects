#ifndef TINYSTL_UTILS_H
#define TINYSTL_UTILS_H

#include <cstddef>

#include "type_traits.hpp"

namespace mystl {

template <class T>
// typename: 跟在它后边的是一个type.
typename std::remove_reference<T>::type&& move(T&& arg) noexcept {
  //获取T的非引用类型: 如果T是一个引用类型,那么就返回它引用的类型, 否则就返回T.
  /* int main() {
      typedef int&& rval_int;
      typedef std::remove_reference<int>::type A;
      typedef std::remove_reference<int&>::type B;
      typedef std::remove_reference<int&&>::type C;
      typedef std::remove_reference<rval_int>::type D;

      std::cout << std::boolalpha;
      std::cout << "typedefs of int:" << std::endl;
      std::cout << "A: " << std::is_same<int,A>::value << std::endl;
      std::cout << "B: " << std::is_same<int,B>::value << std::endl;
      std::cout << "C: " << std::is_same<int,C>::value << std::endl;
      std::cout << "D: " << std::is_same<int,D>::value << std::endl;

      return 0;
  }
  输出:
      typedefs of int:
      A: true
      B: true
      C: true
      D: true
  */
  //为了´保证返回的引用一定是右值引用.
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

template <class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept {
  //把一个左值引用转化成一个右值引用
  return static_cast<T&&>(arg);
}

template <class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept {
  static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
  return static_cast<T&&>(arg);
}

}  // namespace mystl

#endif