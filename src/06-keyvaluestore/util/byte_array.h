#ifndef KINGDB_BYTE_ARRAY_H_
#define KINGDB_BYTE_ARRAY_H_

#include <memory>

#include "interface/kingdb.h"

namespace kdb {

class ByteArrayResource {
 public:
  ByteArrayResource() {}
  virtual ~ByteArrayResource() {}
  virtual char* data() = 0;
  
};

class ByteArray {
 private:
  //用来管理指针所指向的内存
  std::shared_ptr<ByteArrayResource> resource_;
};
}  // namespace kdb

#endif