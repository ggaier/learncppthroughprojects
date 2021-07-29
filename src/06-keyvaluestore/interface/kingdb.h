#ifndef KINGDB_INTERFACE_H_
#define KINGDB_INTERFACE_H_

#include "util/byte_array.h"
#include "util/options.h"
#include "util/status.h"

//定义数据库的接口
namespace kdb {

class KingDB {
 public:
  virtual ~KingDB() {}

  //纯虚函数, 不添加实现, 此时该类为抽象类, 无法被初始化.
  virtual Status Get(ReadOptions& read_options, ByteArray& key,
                     ByteArray* value_out) = 0;

  virtual Status Get(ReadOptions& read_options, ByteArray& key, std::string* value_out){
    return Status::Done()
  }
};

}  // namespace kdb

#endif