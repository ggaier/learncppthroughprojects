#ifndef KINGDB_INTERFACE_H_
#define KINGDB_INTERFACE_H_

#include "util/status.h"
#include "util/options.h"

//定义数据库的接口
namespace kdb {

class KingDB {
 public:
  virtual ~KingDB() {}
  virtual Status Get(ReadOptions& read_options, const std::string& key, ByteArray* value_out){
    
  }
};

}  // namespace kdb

#endif