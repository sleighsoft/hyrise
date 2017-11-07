#include "base_visualizer.hpp"

#include <boost/algorithm/string.hpp>

namespace opossum {

std::string BaseVisualizer::_escape_label(const std::string& in) const {
  auto escaped = in;

  escaped = boost::replace_all_copy(escaped, "\"", "\\\"");
  escaped = boost::replace_all_copy(escaped, "<", "\\<");
  escaped = boost::replace_all_copy(escaped, ">", "\\>");

  return escaped;
}

}