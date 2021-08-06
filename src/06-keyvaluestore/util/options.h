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
    AddParameterToConfigParser(db_options, parser);
    parser.LoadDefaultValues();
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
    //日志选项
    parser.AddParameter(new kdb::StringParameter(
        "log.level", "info", &db_options.log_level, false,
        "Level of the logging, can be: silent, emerg, alert, crit, error, "
        "warn, notice, info, debug, trace."));

    parser.AddParameter(new kdb::StringParameter(
        "log.target", "kingdb", &db_options.log_target, false,
        "Target of the logs, can be 'stderr' to log to stderr, or any custom "
        "string that will be used as the 'indent' parameter for syslog."));

    //数据库选项.
    parser.AddParameter(new kdb::BooleanParameter(
        "db.create-if-missing", true, &db_options.create_if_missing, false,
        "Will create the database if it does not already exits"));
    parser.AddParameter(new kdb::BooleanParameter(
        "db.error-if-exits", false, &db_options.error_if_exists, false,
        "Will exit if the database already exists"));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.incoming-rate-limit", "0", &db_options.rate_limit_incoming, false,
        "Limit the rate of incoming traffic, in bytes per second. Unlimited if "
        "equal to 0."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.write-buffer.size", "64MB", &db_options.write_buffer__size, false,
        "Size of the Write Buffer."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.write-buffer.flush-timeout", "500 milliseconds",
        &db_options.write_buffer__flush_timeout,
        "The timeout after which the write buffer will flush its cache"));
    parser.AddParameter(new kdb::StringParameter(
        "db.write-buffer.mode", "direct", &db_options.write_buffer__mode_str,
        false,
        "The mode with which the write buffer handles incoming traffic, can be "
        "'direct' or 'adaptive'. With the 'direct' mode, once the Write Buffer "
        "is full other incoming Write and Delete operations will block until "
        "the buffer is persisted to secondary storage. The Direct mode should "
        "be used when the clients are not subjects to timeouts. When choosing "
        "the 'adaptive' mode, incoming orders will be made slower, down to the "
        "speed of writes on the secondary storage, so that they are almost "
        "just as fast as when using as the direct mode, but are never "
        "blocking. The adaptive mode is exprected to introduce a small "
        "performance decrease, but required for cases where clients timeout "
        "must be avoided, for example when the data base is used over a "
        "network."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.storage.hstable-size", "32MB", &db_options.storage__hstable_size,
        false,
        "Maximum size a HSTable can have. Entries with keys and values beyond "
        "that size are considered to be large entries."));
    parser.AddParameter(new kdb::StringParameter(
        "db.storage.compression", "lz4",
        &db_options.storage__compression_algorithm, false,
        "Compression algorithm used by the storage engine. Can be 'disabled' "
        "or 'lz4'."));
    parser.AddParameter(new kdb::StringParameter(
        "db.storage.hashing", "xxhash-64",
        &db_options.storage__hashing_algorithm, false,
        "Hashing algorithm used by the storage engine. Can be 'xxhash-64' or "
        "'murmurhash3-64'."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.storage.minimum-free-space-accept-orders", "192MB",
        &db_options.storage__minimum_free_space_accept_orders, false,
        "Minimum free disk space required to accept incoming orders. It is "
        "recommended that for this value to be at least (2 x "
        "'db.write-buffer.size' + 4 x 'db.hstable.maximum-size'), so that when "
        "the file system fills up, the two write buffers can be flushed to "
        "secondary storage safely and the survival-mode compaction process can "
        "be run."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.storage.maximum-part-size", "1MB",
        &db_options.storage__maximum_part_size, false,
        "The maximum part size is used by the storage engine to split entries "
        "into smaller parts -- important for the compression and hashing "
        "algorithms, can never be more than (2^32 - 1) as the algorihms used "
        "do not support sizes above that value."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.storage.inactivity-streaming", "60 seconds",
        &db_options.storage__inactivity_timeout, false,
        "The time of inactivity after which an entry stored with the streaming "
        "API is considered left for dead, and any subsequent incoming parts "
        "for that entry are rejected."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.storage.statistics-polling-interval", "5 seconds",
        &db_options.storage__statistics_polling_interval, false,
        "The frequency at which statistics are polled in the Storage Engine "
        "(free disk space, etc.)."));

    // Compaction options
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.compaction.force-interval", "5 minutes",
        &db_options.compaction__force_interval, false,
        "Duration after which, if no compaction process has been performed, a "
        "compacted is started. Set to 0 to disable."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.compaction.filesystem.free-space-required", "128MB",
        &db_options.compaction__filesystem__free_space_required, false,
        "Minimum free space on the file system required for a compaction "
        "process to be started."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.compaction.filesystem.survival-mode-threshold", "2GB",
        &db_options.compaction__filesystem__survival_mode_threshold, false,
        "If the free space on the file system is above that threshold, the "
        "compaction is in 'normal mode'. Below that threshold, the compaction "
        "is in 'survival mode'. Each mode triggers the compaction process for "
        "different amount of uncompacted data found in the database."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.compaction.filesystem.normal-batch-size", "1GB",
        &db_options.compaction__filesystem__normal_batch_size, false,
        "If the compaction is in normal mode and the amount of uncompacted "
        "data is above that value of 'normal-batch-size', then the compaction "
        "will start when the compaction conditions are checked."));
    parser.AddParameter(new kdb::UnsignedInt64Parameter(
        "db.compaction.filesystem.survival-batch-size", "256MB",
        &db_options.compaction__filesystem__survival_batch_size, false,
        "If the compaction is in survival mode and the amount of uncompacted "
        "data is above that value of 'survival-batch-size', then the "
        "compaction will start when the compaction conditions are checked."));
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