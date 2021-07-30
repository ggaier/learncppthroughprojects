#ifndef KINGDB_CONIFG_PARSER_H_
#define KINGDB_CONFIG_PARSER_H_

#include <set>
#include <vector>

#include "util/status.h"

namespace kdb {

class Parameter {
 public:
  std::string name;
  std::string description;
  std::string default_value;
  bool is_mandatory;
  virtual ~Parameter() {}
  virtual Status Parse(const std::string& config, const std::string& value,
                       const std::string& filepath, int line_number) = 0;
  virtual std::string Type() = 0;
  uint64_t GetMultiplier(std::string str) { return 0; }
};

class ConfigParser {
 public:
  ConfigParser() : error_if_unknown_parameters(true) {}

  ~ConfigParser() {
    for (size_t i = 0; i < parameters_.size(); i++) {
      delete parameters_[i];
    }
  }

  void AddParameter(Parameter* parameter) {
    parameters_.push_back(parameter);
    if (parameter->is_mandatory) {
      mandatories_.insert(parameter->name);
    }
  }

  void SetDefaultValue(const std::string name,
                       const std::string default_value) {
    //除了使用iterator遍历外, 还可以使用[]下标访问的方式
    //遍历查找对应的parameter, 并修改它的默认参数
    for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
      if ((*it)->name == name) {
        (*it)->default_value = default_value;
        break;
      }
    }
  }

  bool FoundAllMandatoryParameters() { return (mandatories_.size() == 0); }

  void PrintAllMissingMandatoryParameters() {
    if (mandatories_.size() == 0) return;
    fprintf(stderr,
            "Error: the following mandatory parameters are missing: \n");
    //这个需要引用吗? 不用引用也没关系吧
    for (auto& name : mandatories_) {
      fprintf(stderr, "%s\n", name.c_str());
    }
  }

  int min_int(int a, int b) { return a < b ? a : b; }

  //暂时不知道这个方法是干什么的
  std::string AlignString(int margin, int column, std::string& str) {
    std::string str_aligned;
    size_t i = 0;
    while (i < str.size()) {
      size_t j_start = i + column;
      size_t j = (j_start <= str.size()) ? j_start : i;
      //遍历到空格处
      while (j > i) {
        if (str[j] == ' ') break;
        j--;
      }
      if (j <= i) j = str.size();
      //前缀上margin数量的空格
      for (int k = 0; k < margin; k++) str_aligned += " ";
      //然后加上到空格处的字符
      str_aligned += str.substr(i, j - i);
      //再然后换行.
      if (j + 1 < str.size()) str_aligned += "\n";
      //再然后, 跳转到空格之后一位, 开始遍历.
      i = j + 1;
    }
    return str_aligned;
  }

  bool error_if_unknown_parameters;

 private:
  //遵循插入顺序, 连续存储, 除了可以用iterator遍历外, 还可以
  //使用指针偏移来访问元素.
  std::vector<kdb::Parameter*> parameters_;
  //关联式容器. 按照Key排序的集合, 且不允许重复
  std::set<std::string> mandatories_;
};

}  // namespace kdb

#endif