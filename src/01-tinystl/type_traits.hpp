#ifndef TINYSTL_TYPE_TRAITS_H_
#define TINYSTL_TYPE_TRAITS_H_

//源自type support 库中的一部分. 用来获取编译时类型信息.
// http://www.cplusplus.com/reference/type_traits/?kw=type_traits
#include <type_traits>

namespace mystyl {

template <class T, T v>
struct m_intergral_constant {
  // constexpr 表示的是编译时的常量,
  // 只有这种类型的变量能够用在要求编译时常量的上下文中.
  static constexpr T value = v;
};

}  // namespace mystyl

#endif