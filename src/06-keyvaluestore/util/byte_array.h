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
  virtual const char* data_const() = 0;
  virtual uint64_t size() = 0;
  virtual const uint64_t size_const() = 0;
  virtual uint64_t size_compressed() = 0;
  virtual const uint64_t size_compressed_const() = 0;
};

class AllocatedByteArrayResource : public ByteArrayResource {
  friend class ByteArray;

 public:
  AllocatedByteArrayResource(char* data, uint64_t size, bool deep_copy)
      : data_(nullptr), size_(0), size_compressed_(0) {
    if (deep_copy) {
      size_ = size;
      data_ = new char[size_];
      memcpy(data_, data, size_);
    } else {
      size_ = size;
      data_ = data;
    }
  }

  AllocatedByteArrayResource(uint64_t size)
      : data_(nullptr), size_(0), size_compressed_(0) {
    size_ = size;
    data_ = new char[size_];
  }

  virtual char* data() { return data_; }
  virtual const char* data_const() { return data_; }
  virtual uint64_t size() { return size_; }
  virtual const uint64_t size_const() { return size_; }
  virtual uint64_t size_compressed() { return size_compressed_; }
  virtual const uint64_t size_compressed_const() { return size_compressed_; }

 private:
  char* data_;
  uint64_t size_;
  uint64_t size_compressed_;
}

class PointerByteArrayResource : public ByteArrayResource {
  friend class ByteArray;

 public:
  PointerByteArrayResource(const char* data, uint64_t size)
      : data_(data), size_(size), size_compressed_(0) {}

  virtual ~PointerByteArrayResource() {}

  // const_cast是用来修改一个指针指向的对象是否是常量.
  //比如const_cast<char*>(const_variable_) 是转化成非常量
  //而const_cast<const char*>(variable_) 则是转化成常量.
  virtual char* data() { return const_cast<char*>(data_); }
  virtual const char* data_const() { return data_; }
  virtual uint64_t size() { return size_; }
  virtual const uint64_t size_const() { return size_; }
  virtual uint64_t size_compressed() { return size_compressed_; }
  virtual const uint64_t size_compressed_const() { return size_compressed_; }

 private:
  const char* data_;
  uint64_t size_;
  uint64_t size_compressed_;
};

class ByteArray {
 public:
  ByteArray()
      : size_(0),
        size_compressed_(0),
        offset_(0),
        checksum_(0),
        checksum_initial_(0) {}

  virtual ~ByteArray() {}
  //TODO

 private:
  //用来管理指针所指向的内存
  std::shared_ptr<ByteArrayResource> resource_;
  uint64_t size_;
  uint64_t size_compressed_;
  uint64_t offset_;

  uint32_t checksum_;
  uint32_t checksum_initial_;

  //对于正常的方法, 命名的规则是CamelCase, 但是对于访问器和修改器方法,
  //命名的规范可以参考 变量的命名方式, snake_case
  virtual uint64_t size_compressed() { return size_compressed_; }
  virtual uint64_t size_compressed_const() const { return size_compressed_; }
  virtual void set_size(uint64_t s) { size_ = s; }
  virtual void set_size_compressed(uint64_t s) { size_compressed_ = s; }
  virtual uint64_t is_compressed() { return (size_compressed_ != 0); }
  virtual void set_offset(uint64_t o) { offset_ = o; }
  virtual void increment_offset(uint64_t inc) { offset += inc; }

  virtual uint32_t checksum() { return checksum_; }
  virtual uint32_t checksum_initial() { return checksum_initial_; }
  virtual void set_checksum(uint32_t c) { checksum_ = c; }
  virtual void set_checksum_initial(uint32_t c) { checksum_initial_ = c; }

  static ByteArray NewEmptyByteArray() { return ByteArray(); }

  static ByteArray NewReferenceByteArray(ByteArray& byte_array_in) {
    // TODO
    ByteArray byte_array = byte_array_in;
    return byte_array;
  }

  static ByteArray NewAllocateMemoryByteArray(uint64_t size) {
    ByteArray byte_array;
    // make_shared申请并构造一个对象, 然后返回一个share_ptr对象, 这个返回的对象
    //包含了刚才构造的对象的指针. 参数会被当做构造器的参数传给T的构造器.
    byte_array.resource_ = std::make_shared<AllocatedByteArrayResource>(size);
    byte_array.size_ = size;
    return byte_array;
  }
};
}  // namespace kdb

#endif