#ifndef TINYSTL_ALLOCATOR
#define TINYSTL_ALLOCATOR
//这部分是Header guard, 用来防止当一个头文件多次被引入时, 产生的重复代码复制的问题.

#include "utils.hpp"

namespace mystl
{

    //allocator 模板类.
    template <class T>
    class allocator
    {
    public:
        //typedef的命名习惯是名字后缀_t, 用来避免命名冲突, 同时用来表示名字是一个typedef
        typedef T value_t; //用来创建一个数据的类型的别名, 就可以用value_t来表示类型 T了.
        typedef T *pointer_t;
        typedef const T *const_pointer_t; //指向一个常量值的指针, 通过指针无法修改这个常量值
        typedef T &reference_t;
        typedef const T &const_reference_t; //指向一个常量值的引用.
        typedef size_t size_type;           // 用来表示一个对象的字节大小的类型.
        typedef ptrdiff_t difference_type;  //用来表示指针相减结果的类型.

    public:
        //声明静态的成员函数
        static T *allocate();
        static T *allocate(size_type n);

        static void deallocate(T *ptr);
        static void deallocate(T *ptr, size_type n);

        static void construct(T *ptr);
        static void construct(T *ptr, const T &value);
        //新语法 rvalue reference, 用于move constructor
        static void construct(T *ptr, T &&value);

        //可变参数
        template <class... Args>
        static void construct(T *ptr, Args &&...args);

        static void destroy(T *ptr);
        static void destroy(T *first, T *last);
    };

    //模板类的类外实现
    template <class T>
    T *allocator<T>::allocate()
    {
        //1. 相比传统的C风格的类型转化, C++提供了四种类型操作符, 分别是
        //dynamic_cast, reinterpret_cast, static_cast, const_cast
        //2. operator new() 默认的allocation 函数. 这个函数只会申请内存, 但是不会调用类的构造器.
        // :: 使用来告诉编译器使用全名namespace. 
        return static_cast<T *>(::operator new(sizeof(T)));
    }

}
#endif