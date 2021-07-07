## Write a simple memory allocator

* 源码使用C编写, 并且内容不多, 应该容易理解.
* 原文地址: [Write a simple memory allocator](https://arjunsreedharan.org/post/148675821737/write-a-simple-memory-allocator)

### 准备知识

在学习该项目之前, 应该先要了解一个应用的内存布局. 一个应用执行在一个虚拟的地址空间中, 它包括五个部分:

* **Text Section**: 代码段, 又叫文本段, 用来存放编译后的二进制文件.
* **Data Section**: 数据段, 已初始化的数据, 包括全局和静态变量.
* **BSS**: Bloc Started by Symbol, 包括未初始化的静态数据.
* **Heap**: 堆区, 包括动态申请的数据.
* **Stack**: 栈区, 包括变量, 函数参数, 指针的拷贝等等.

有时候Data, BSS, Heap统称为数据段.

TODO: 如何写一个测试验证一下呢?
