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

  /*** Get API ***/
  //纯虚函数, 不添加实现, 此时该类为抽象类, 无法被初始化.
  virtual Status Get(ReadOptions& read_options, ByteArray& key,
                     ByteArray* value_out) = 0;

  //下边是几个重载函数, 但最终的目的都是要通过上边的纯虚函数来从DB中获取
  //数据, 获取的数据以ByteArray的形式返回.
  virtual Status Get(ReadOptions& read_options, ByteArray& key,
                     std::string* value_out) {
    ByteArray value;
    Status s = Get(read_options, key, &value);
    if (!s.IsOK()) return s;
    *value_out = value.ToString();
    return s;
  }

  virtual Status Get(ReadOptions& read_options, const std::string& key,
                     ByteArray* value_out) {
    //这里是根据Key申请了一个ByteArray, 这个ByteArray其实是key.
    ByteArray byte_array_key = NewPointerByteArray(key.c_str(), key.size());
    Status s = Get(read_options, byte_array_key, value_out);
    return s;
  }

  virtual Status Get(ReadOptions& read_options, const std::string& key,
                     std::string* value_out) {
    ByteArray byte_array_key = NewPointerByteArray(key.c_str(), key.size());
    ByteArray value;
    Status s = Get(read_options, key, &value);
    if (!s.IsOK()) return s;
    *value_out = value.ToString();
    return s;
  }

  /*** Put API ***/

  virtual Status Put(WriteOptions& write_options, ByteArray& key,
                     ByteArray& chunk) = 0;

  virtual Status Put(WriteOptions& write_options, ByteArray& key,
                     const std::string& chunk) {
    ByteArray byte_array_chunk =
        NewDeepCopyByteArray(chunk.c_str(), chunk.size());
    return Put(write_options, key, byte_array_chunk);
  }

  virtual Status Put(WriteOptions& write_options, const std::string& key,
                     const std::string& chunk) {
    ByteArray byte_array_key = NewDeepCopyByteArray(key.c_str(), key.size());
    ByteArray byte_array_chunk =
        NewDeepCopyByteArray(chunk.c_str(), chunk.size());
    return Put(write_options, byte_array_key, byte_array_chunk);
  }

  virtual Status Delete(WriteOptions& write_options, ByteArray& key) = 0;
  virtual Iterator NewIterator(ReadOptions& read_options) = 0;
  virtual Status Open() = 0;
  virtual void Close() = 0;
  virtual void Flush() = 0;
  virtual void Compact() = 0;

 private:
  virtual Status Put(WriteOptions& write_options, ByteArray& key,
                     ByteArray& chunk, uint64_t offset_chunk,
                     uint64_t size_value) = 0;
};

}  // namespace kdb

#endif