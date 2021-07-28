#ifndef KINGDB_OPTIONS_H_
#define KINGDB_OPTIONS_H_
namespace kdb {

struct ReadOptions {
  bool verify_checksums;
  ReadOptions() : verify_checksums(false) {}
};
}  // namespace kdb

#endif