#ifndef KINGDB_OPTIONS_H_
#define KINGDB_OPTIONS_H_

#include "util/config_parser.h"

namespace kdb {

enum HashType { kMurmurHash3_64 = 0x0, kxxHash_64 = 0x1 };

enum CompressionType { kNoCompressions = 0x0, kLZ4Compression = 0x1 };

enum ChecksumType { kNoChecksum = 0x0, kCRC32C = 0x1 };

enum WriteBufferMode {
  kWriteBufferModeDirect = 0x0,
  kWriteBufferModeAdaptive = 0x1
}

struct CompressionOptions {
  CompressionType type;
  CompressionOptions(CompressionType ct) : type(ct) {}
};

struct DatabaseOptions {
 public:
  DatabaseOptions()
      : internal__hstable_header_size(8192),  //单位字节
        internal__num_iterations_per_lock(10),
        internal__close_timeout(500),                    //单位ms
        internal__open_file_retry_delay(5000),           // ms
        internal__size_multipart_required(1024 * 1024),  // bytes
        internal__compaction_check_interval(500),        // ms
        hash(kxxHash_64),
        compression(kLZ4Compression),
        checksum(kCRC32C),
        write_buffer__mode(kWriteBufferModeDirect) {
    DatabaseOptions& db_options = *this;
    ConfigParser parser;
  }
  uint64_t internal__hstable_header_size;
  uint64_t internal__num_iterations_per_lock;
  uint64_t internal__close_timeout;
  uint64_t internal__open_file_retry_delay;
  uint64_t internal__size_multipart_required;
  uint64_t internal__compaction_check_interval;

  // DB被创建之后, 无法修改的属性
  HashType hash;
  CompressionOptions compression;
  ChecksumType checksum;
  uint64_t storage__hstable_size;
  std::string storage__compression_algorithm;
  std::string storage__hashing_algorithm;

  // Instance options: 当db打开之后, 可以用来修改实例状态
  bool create_if_missing;
  bool error_if_exists;
  uint32_t max_open_files;
  uint64_t rate_limit_incoming;

  uint64_t write_buffer__size;
  uint64_t write_buffer__flush_timeout;
  std::string write_buffer__mode_str;
  WriteBufferMode write_buffer__mode;

  uint64_t storage__inactivity_timeout;
  uint64_t storage__statistics_polling_interval;
  uint64_t storage__minimum_free_space_accept_orders;
  uint64_t storage__maximum_part_size;

  uint64_t compaction__force_interval;
  uint64_t compaction__filesystem__survival_mode_threshold;
  uint64_t compaction__filesystem__normal_batch_size;
  uint64_t compaction__filesystem__survival_batch_size;
  uint64_t compaction__filesystem__free_space_required;

  //日志
  std::string log_level;
  std::string log_target;

  static std::string GetPath(const std::string& dirpath) {
    return dirpath + "/db_options";
  }

  static std::string GetFilename() { return "db_options"; }

  static void AddParameterToConfigParser(DatabaseOptions& db_options,
                                         ConfigParser& parser) {
    parser.AddParameter(new kdb::)
  }
};

//这个struct封装了读取数据的参数
struct ReadOptions {
  bool verify_checksums;
  ReadOptions() : verify_checksums(false) {}
};

//封装写入数据的参数
struct WriteOptions {
  bool sync;
  WriteOptions() : sync(false) {}
};

}  // namespace kdb

#endif