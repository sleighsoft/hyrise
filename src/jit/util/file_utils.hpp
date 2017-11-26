#pragma once

#include <boost/filesystem.hpp>

struct file_utils {
  static boost::filesystem::path temp_file(const std::string& extension) {
    return boost::filesystem::temp_directory_path() /
           boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%." + extension);
  }

  template <typename T>
  static void write_to_file(const boost::filesystem::path& path, const T& content) {
    boost::filesystem::ofstream os(path);
    os << content;
  }

  static std::string read_from_file(const boost::filesystem::path& path) {
    boost::filesystem::ifstream is(path);
    std::stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
  }
};
