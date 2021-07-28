#ifndef KINGDB_STATUS_H_
#define KINGDB_STATUS_H_

#include <string>

namespace kdb {

//使用KingDB对数据进行添加, 读取, 删除等操作的时候,
//把结果等其他信息都使用该类来封装.
class Status {
 public:
  Status() {
    code_ = kOK;
    message1_ = "";
  }
  ~Status() {}
  Status(int code) { code_ = code; }

  Status(int code, std::string message1, std::string message2) {
    code_ = code;
    message1_ = message1;
    message2_ = message2;
  }

  static Status OK() { return Status(); }
  static Status Done() { return Status(kDone); }
  static Status MultipartRequired() { return Status(kMultipartRequired); }
  static Status DeleteOrder() { return Status(kDeleteOrder); }
  static Status NotFound(const std::string& message1,
                         const std::string& message2 = "") {
    return Status(kNotFound, message1, message2);
  }
  static Status InvalidArgument(const std::string& message1,
                                const std::string& message2 = "") {
    return Status(kInvalidArgument, message1, message2);
  }

  //方法名应该是CamelCase的命名.
  bool IsOK() const { return code() == kOK; }
  bool IsNotFound() const { return code() == kNotFound; }
  bool IsDeleteOrder() const { return code() == kDeleteOrder; }
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }
  bool IsIOError() const { return code() == kIOError; }
  bool IsDone() const { return code() == kDone; }
  bool IsMultipartRequired() const { return code() == kMultipartRequired; }

  //只有方法实现简单(不超过10行)的时候, 才定义inline 方法.
  std::string ToString() const;

 private:
  //变量的命名应该是snake_case
  //如果是私有的成员变量, 后缀_
  int code_;
  std::string message1_;
  std::string message2_;

  //常函数可以确保不会修改对象中的数据.
  int code() const { return code_; }

  //枚举应该和常量的命名类似, 也就是以k开头的camelCase.
  enum Code {
    kOK = 0,
    kNotFound,
    kDeleteOrder,
    kInvalidArgument,
    kIOError,
    kDone,
    kMultipartRequired
  };
};
}  // namespace kdb

#endif