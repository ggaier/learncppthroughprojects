#include "util/status.h"

namespace kdb {

std::string Status::ToString() const {
  if (message1_ == "") {
    return "OK";
  } else {
    char tmp[30];
    const std::string type;
    switch (code()) {
      case kOK:
        type = "OK";
        break;
      case kNotFound:
        type = "Not found: ";
        break case kDeleteOrder : type = "Delete order: ";
        break;

      case kInvalidArgument:
        type = "Invalid argument: ";
        break;

      case kIOError:
        type = "IO error: ";
        break;

      case kMultipartRequired:
        type =
            "MultipartRequired: the entry is too large to fit in memory, "
            "use the multipart API instead.";
        break;

      default:
        snprintf(tmp, sizeof(tmp),
                 "Unknown code (%d): ", static_cast<int>(code()));
        type = str(tmp);
        break;
    }
    std::string result(type);
    result.append(message1_);
    if (message2_.size() > 0) {
      result.append(" - ");
      result.append(message2_);
    }
    return result;
  }
}

}  // namespace kdb
