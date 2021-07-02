//按照Google的C++风格, header guard <PROJECT>_<PATH>_<FILE>_H_
#ifndef TINYSTL_CONSTRUCTOR_H_
#define TINYSTL_CONSTRUCTOR_H_

#include <new>

#ifdef _MSG_VER

#endif //_MSG_VER

namespace mystl
{
    template <class T>
    void construct(T *ptr)
    {   
        //这个方法实际上是调用的void* operator new (std::size_t size, void* ptr) noexcept;
        new ((void *)ptr) T();
    }

}

#endif // FOO_BAR_BAZ_H_