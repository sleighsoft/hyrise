#pragma once

#include <string>

namespace opossum {

class BaseVisualizer {
 public:
  virtual ~BaseVisualizer() = default;

 protected:
  std::string _escape_label(const std::string& in) const;
};

}