# 实现一个KV Store.

这是一个C++的项目. 教程地址[Implementing a Key Value Store](https://codecapsule.com/2012/11/07/ikvs-implementing-a-key-value-store-table-of-contents/)

虽然这个项目中我又些不懂的东西, 但是先学习起来, 模仿是学习的第一步.

**暂时学习到`ConfigParser`**, 但是觉得对C++的学习过于超前, 所以我决定转向学习QT, 图形界面的东西更能提高自己的学习兴趣.

## 代码风格

项目的代码风格使用了Google的[C++风格](https://google.github.io/styleguide/cppguide.html). 包括:

* 文件名后缀使用`.cc`, 头文件名使用`.h`
* 类格式: `public` 区放在最开头, 后边跟着`protected`, `private`.