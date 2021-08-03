#ifndef KINGDB_CONIFG_PARSER_H_
#define KINGDB_CONFIG_PARSER_H_

#include <cinttypes>
#include <map>
#include <regex>
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
  uint64_t GetMultiplier(std::string str) {
    //数字1个及以上,空格0个或者更多, 非空格多个
    std::regex regex_number{"([\\d]+)[\\s]*([^\\s])*"};
    std::smatch matches;
    //从str中查找匹配正则表达式的部分, 并把结果存储在matches中.
    //返回true如果匹配存在. matches.size()等于匹配个数+1, 因为第0个是匹配字符串.
    if (!std::regex_search(str, matches, regex_number) || matches.size() != 3) {
      return 1;
    }

    std::string number(matches[1]);
    std::string unit(matches[2]);
    //对unit执行tolower()方法, 也就是把单位转化成小写
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);

    if (unit == "") return 1;

    if (unit == "b" || unit == "byte" || unit == "bytes") {
      return 1;
    } else if (unit == "kb") {
      return 1024;
    } else if (unit == "mb") {
      return 1024 * 1024;
    } else if (unit == "gb") {
      return (uint64_t)1024 * (uint64_t)1024 * 1024;
    } else if (unit == "tb") {
      return (uint64_t)1024 * 1024 * 1024 * 1024;
    } else if (unit == "pb") {
      return (uint64_t)1024 * 1024 * 1024 * 1024 * 1024;
    }

    else if (unit == "ms" || unit == "millisecond" || unit == "milliseconds") {
      return 1;
    } else if (unit == "s" || unit == "second" || unit == "seconds") {
      return 1000;
    } else if (unit == "minute" || unit == "minutes") {
      return 1000 * 60;
    } else if (unit == "hour" || unit == "hours") {
      return 1000 * 60 * 60;
    }
    return 0;
  }
};

class FlagParameter : public Parameter {
 public:
  bool* is_present;
  FlagParameter(const std::string& name_in, bool* is_present_in,
                bool mandatory_in, const std::string& description_in) {
    name = name_in;
    is_mandatory = mandatory_in;
    description = description_in;
    default_value = "not set";
    is_present = is_present_in;
    *is_present = false;
  }
  virtual ~FlagParameter() {}
  virtual Status Parse(const std::string& config, const std::string& value,
                       const std::string& filepath, int line_number) {
    *is_present = true;
    return Status::OK();
  }
  virtual std::string Type() { return "Flag"; }
};

class BooleanParameter : public Parameter {
 public:
  bool* state;
  BooleanParameter(const std::string& name_in, bool default_in,
                   bool* is_present_in, bool mandatory_in,
                   const std::string& description_in) {
    name = name_in;
    is_mandatory = mandatory_in;
    description = description_in;
    default_value = default_in ? "True" : "False";
    state = is_present_in;
    *state = default_in;
  }

  virtual ~BooleanParameter() {}
  virtual Status Parse(const std::string& config, const std::string& value_in,
                       const std::string& filepath, int line_number) {
    std::string value(value_in);
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true" || value == "1") {
      *state = true;
    } else if (value == "false" || value == "0") {
      *state = false;
    } else {
      std::string str_line_number = "Invalid value for boolean parameter [" +
                                    config + "] in file [" + filepath + "]" +
                                    std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    return Status::OK();
  }

  virtual std::string Type() { return "Boolean"; }
};

class UnsignedInt32Parameter : public Parameter {
 public:
  uint32_t* ptr;
  UnsignedInt32Parameter(const std::string& name_in,
                         const std::string default_in, uint32_t* ptr_in,
                         bool mandatory_in, const std::string& description_in) {
    name = name_in;
    is_mandatory = mandatory_in;
    description = description_in;
    default_value = default_in;
    ptr = ptr_in;
    Status s = Parse(name, default_in, "default-value", 0);
    if (!s.IsOK()) {
      fprintf(stderr, "Error: invalid default value for parameter [%s]\n",
              name.c_str());
      exit(1);
    }
  }
  virtual ~UnsignedInt32Parameter() {}
  virtual uint32_t Get() { return *ptr; }
  virtual Status Parse(const std::string& config, const std::string& value,
                       const std::string& filepath, int line_number) {
    int num_scanned = sscanf(value.c_str(), "%u", ptr);
    if (num_scanned != 1) {
      std::string str_line_number =
          "Invalid value for unsigned 32-bit integer paramter [" + config +
          "] in file [" + filepath + "] on line " + std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    uint64_t multiplier = GetMultiplier(value);
    if (multiplier == 0) {
      std::string str_line_number = "Invalid unit for parameter [" + config +
                                    "] in file [" + filepath + "] on line " +
                                    std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    *ptr = *ptr * multiplier;
    return Status::OK();
  }

  virtual std::string Type() { return "Unsigned 32-bit integer"; }
};

class UnsignedInt64Parameter : public Parameter {
 public:
  uint64_t* ptr;
  UnsignedInt64Parameter(const std::string& name_in,
                         const std::string& default_in, uint64_t* ptr_in,
                         bool mandatory_in, const std::string& description_in) {
    name = name_in;
    is_mandatory = mandatory_in;
    description = description_in;
    default_value = default_in;
    ptr = ptr_in;
    Status s = Parse(name, default_in, "default_value", 0);
    if (!s.IsOK()) {
      fprintf(stderr, "Error: invalid default value for parameter [%s]\n",
              name.c_str());
      exit(1);
    }
  }

  virtual ~UnsignedInt64Parameter() {}
  virtual uint64_t Get() { return *ptr; }
  virtual Status Parse(const std::string& config, const std::string& value,
                       const std::string& filepath, int line_number) {
    // read formatted data from string
    //这里的意思是读取一个uint64_t的数据赋值给ptr, 同时返回成功填充数据的个数.
    int num_scanned = sscanf(value.c_str(), "%" PRIu64, ptr);
    if (num_scanned != 1) {
      std::string str_line_number =
          "invalid value for unsigned 64-bit integer parameter [" + config +
          "] in file [" + filepath + "] on line " + std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    uint64_t multiplier = GetMultiplier(value);
    if (multiplier == 0) {
      std::string str_line_number = "Invalid unit for parameter [" + config +
                                    "] in file [" + filepath + "] on line " +
                                    std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    *ptr = *ptr * multiplier;
    return Status::OK();
  }

  virtual std::string Type() { return "Unsigned 64-bit integer"; }
};

class DoubleParameter : public Parameter {
 public:
  double* ptr;
  DoubleParameter(const std::string& name_in, const std::string& default_in,
                  double* ptr_in, bool mandatory_in,
                  const std::string& description_in) {
    name = name_in;
    is_mandatory = mandatory_in;
    description = description_in;
    default_value = default_in;
    ptr = ptr_in;
    Status s = Parse(name, default_in, "default-value", 0);
    if (!s.IsOK()) {
      fprintf(stderr, "Error: invalid default value for parameter [%s]\n",
              name.c_str());
      exit(1);
    }
  }
  virtual ~DoubleParameter() {}
  virtual double Get() { return *ptr; }
  virtual Status Parse(const std::string& config, const std::string& value,
                       const std::string& filepath, int line_number) {
    int num_scanned = sscanf(value.c_str(), "%lf", ptr);
    if (num_scanned != 1) {
      std::string str_line_number =
          "Invalid value for double-precision number parameter [" + config +
          "] in file [" + filepath + "] on line " + std::to_string(line_number);
      return Status::IOError("ConfigParser", str_line_number);
    }
    return Status::OK();
  }

  virtual std::string Type() { return "Double-precision number"; }
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
    // 4, 74
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
      //然后加上到空格处的字符,
      str_aligned += str.substr(i, j - i);
      if (j + 1 < str.size()) str_aligned += "\n";
      //再然后, 跳转到空格之后一位, 开始遍历.
      i = j + 1;
    }
    return str_aligned;
  }

  void PrintUsage() {
    int margin = 4;
    int column = 74;
    std::string str_margin = "";
    for (int k = 0; k < margin; k++) str_margin += " ";
    for (auto& p : parameters_) {
      fprintf(stdout, "  --%s\n", p->name.c_str());
      std::string d_aligned = AlignString(margin, column, p->description);
      fprintf(stdout, "%s\n", d_aligned.c_str());
      if (mandatories_.find(p->name) == mandatories_.end()) {
        fprintf(stdout, "%sDefault value: %s (%s)\n", str_margin.c_str(),
                p->default_value.c_str(), p->Type().c_str());
      } else {
        fprintf(stdout, "%sThis parameter is *mandatory* (%s)\n",
                str_margin.c_str(), p->Type().c_str());
      }
      fprintf(stdout, "\n");
    }
  }

  void PrintMarkdown() {
    for (auto& p : parameters_) {
      fprintf(stdout, "`%s`  \n%s  \n", p->name.c_str(),
              p->description.c_str());
      if (mandatories_.find(p->name) == mandatories_.end()) {
        fprintf(stdout, "Default value: %s (%s)\n", p->default_value.c_str(),
                p->Type().c_str());
      } else {
        fprintf(stdout, "This parameter is *mandatory* (%s)\n",
                p->Type().c_str());
      }
      fprintf(stdout, "\n");
    }
  }

  Status LoadDefaultValue() { return ParseCommandLine(0, nullptr); }

  Status ParseCommandLine(int argc, char** argv) {
    std::map<std::string, Parameter*> parameters;
    for (auto& p : parameters_) {
      parameters[p->name] = p;
    }
    int i = 1;
    while (i < argc) {
      //参数必须以"--"开头
      if (strncmp(argv[i], "--", 2) != 0) {
        if (error_if_unknown_parameters) {
          std::string msg = "Invalide parameter [" + std::string[argv[i]] + "]";
          return Status::IOError("ConfigReader::ReadCommandLine()", msg);
        } else {
          i++;
          continue;
        }
      }
      std::string argument_raw(argv[i]);
      //截断参数的前两个字符"--", substr(pos, count = -1)
      std::string argument = argument_raw.substr(2);

      //判断参数中是否有等号
      bool has_equal_sign = false;
      std::string value;
      // find(str, pos=0): 从pos开始, 查找匹配的字符串, 并返回起始位置.
      // 如果失败返回npos(-1).
      int pos = argument.find("=");
      if (pos != std::string::npos) {
        value = argument.substr(pos + 1);
        argument = argument.substr(0, pos);
        has_equal_sign = true;
      }

      //根据key从集合中查找到对应的元素, 并返回一个Iterator.
      //如果没有找到会返回end()
      if (parameters.find(argument) == parameters.end()) {
        if (error_if_unknown_parameters) {
          std::string msg = "Unknown parameter: [" + argument + "]";
          return Status::IOError("ConfigReader::ReadCommandLIne()", msg);
        } else {
          i++;
          continue;
        }
      }

      // dynamic_cast 的优势就在于安全, 如果类型转换失败, 就会返回空指针.
      // static_cast: 和C-style的类型转换类似.
      bool is_flag_parameter =
          (dynamic_cast<FlagParameter*>(parameters[argument])) != nullptr;

      if (is_flag_parameter && has_equal_sign) {
        std::string msg = "The argument [" + std::string(argv[i]) +
                          "] is of type FlagParameter and has an equal sign";
        return Status::IOError("ConfigReader::ReadCommandLine()", msg);
      }

      if (!is_flag_parameter && !has_equal_sign) {
        if (i + 1 >= argc || strncmp(argv[i + 1], "--", 2) == 0) {
          std::string msg =
              "Missing value for parameter [" + std::string(argv[i]) + "]";
          return Status::IOError("ConfigReader::ReadCommandLine()", msg);
        }
        i++;
        value = argv[i];
      }

      Status s =
          parameters[argument]->Parse(argument, value, "command-line", 0);
      if (!s.IsOK()) return s;
      //移除某个元素.
      mandatories_.erase(argument);
      i++;
    }
    return Status::OK();
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