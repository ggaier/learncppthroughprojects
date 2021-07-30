#ifndef KINGDB_DATABASE_H_
#define KINGDB_DATABASE_H_

#include "interface/kingdb.h"
#include "util/byte_array.h"
#include "util/options.h"
#include "util/status.h"

namespace kdb {
class Database : public KingDB {
 private:
  KingDB* NewSnapshotPointer();
  Status GetRaw(ReadOptions& read_options, ByteArray& key, ByteArray* value_out,
                bool want_raw_data);
  Status PutPartValidSize(WriteOptions& write_options, ByteArray& key,
                          ByteArray& chunk, uint64_t offset_chunk,
                          uint64_t size_value);
  kdb::Da
};

}  // namespace kdb

#endif