
#include <iostream>

#include "SQLParserResult.h"
#include "SQLParser.h"
#include "optimizer/abstract_syntax_tree/ast_to_operator_translator.hpp"
#include "sql/sql_to_ast_translator.hpp"
#include "storage/storage_manager.hpp"
#include "utils/load_table.hpp"
#include "tpch/tpch_queries.hpp"

using namespace opossum;

int main() {
  StorageManager::get().add_table("customer", load_table("src/test/tables/tpch/customer.tbl", 2));
  StorageManager::get().add_table("lineitem", load_table("src/test/tables/tpch/lineitem.tbl", 2));
  StorageManager::get().add_table("nation", load_table("src/test/tables/tpch/nation.tbl", 2));
  StorageManager::get().add_table("orders", load_table("src/test/tables/tpch/orders.tbl", 2));
  StorageManager::get().add_table("part", load_table("src/test/tables/tpch/part.tbl", 2));
  StorageManager::get().add_table("partsupp", load_table("src/test/tables/tpch/partsupplier.tbl", 2));
  StorageManager::get().add_table("region", load_table("src/test/tables/tpch/region.tbl", 2));
  StorageManager::get().add_table("supplier", load_table("src/test/tables/tpch/supplier.tbl", 2));

  const auto query = tpch_queries[6];

  hsql::SQLParserResult parse_result;
  hsql::SQLParser::parseSQLString(query, &parse_result);

  if (!parse_result.isValid()) {
    std::cout << parse_result.errorMsg() << std::endl;
    std::cout << "ErrorLine: " << parse_result.errorLine() << std::endl;
    std::cout << "ErrorColumn: " << parse_result.errorColumn() << std::endl;
    throw std::runtime_error("Query is not valid.");
  }

  auto result_node = SQLToASTTranslator{false}.translate_parse_result(parse_result)[0];

  result_node->print();

  return 0;
}
