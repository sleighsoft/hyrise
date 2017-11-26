#include "blob_store.hpp"
#include <iostream>

std::string BlobStore::get(const std::string& key) { return BlobStore::content[key]; }

std::unordered_map<std::string, std::string> BlobStore::content(
    {{"9Aggregate_cpp", std::string(&aggregate_cpp, aggregate_cpp_size)},
     {"9Aggregate_processed_cpp", std::string(&aggregate_processed_cpp, aggregate_processed_cpp_size)},
     {"8GetTable_cpp", std::string(&get_table_cpp, get_table_cpp_size)},
     {"8GetTable_processed_cpp", std::string(&get_table_processed_cpp, get_table_processed_cpp_size)},
     {"12GetTableMock_cpp", std::string(&get_table_mock_cpp, get_table_mock_cpp_size)},
     {"12GetTableMock_processed_cpp", std::string(&get_table_mock_processed_cpp, get_table_mock_processed_cpp_size)},
     {"9TableScan_cpp", std::string(&table_scan_cpp, table_scan_cpp_size)},
     {"9TableScan_processed_cpp", std::string(&table_scan_processed_cpp, table_scan_processed_cpp_size)},
     {"5Print_cpp", std::string(&print_cpp, print_cpp_size)},
     {"5Print_processed_cpp", std::string(&print_processed_cpp, print_processed_cpp_size)}});
