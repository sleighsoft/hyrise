#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

extern char aggregate_cpp;
extern size_t aggregate_cpp_size;
extern char aggregate_processed_cpp;
extern size_t aggregate_processed_cpp_size;

extern char get_table_cpp;
extern size_t get_table_cpp_size;
extern char get_table_processed_cpp;
extern size_t get_table_processed_cpp_size;

extern char get_table_mock_cpp;
extern size_t get_table_mock_cpp_size;
extern char get_table_mock_processed_cpp;
extern size_t get_table_mock_processed_cpp_size;

extern char print_cpp;
extern size_t print_cpp_size;
extern char print_processed_cpp;
extern size_t print_processed_cpp_size;

extern char table_scan_cpp;
extern size_t table_scan_cpp_size;
extern char table_scan_processed_cpp;
extern size_t table_scan_processed_cpp_size;

class BlobStore {
 public:
  static std::string get(const std::string& key);

 private:
  static std::unordered_map<std::string, std::string> content;
};
