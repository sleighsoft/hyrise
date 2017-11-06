#include <iostream>

#include <chrono>
#include <planviz/ast_visualizer.hpp>
#include <planviz/join_graph_visualizer.hpp>
#include <optimizer/abstract_syntax_tree/abstract_ast_node.hpp>
#include <optimizer/optimizer.hpp>
#include <optimizer/join_graph.hpp>
#include <optimizer/join_graph_builder.hpp>

#include "sql/sql_to_ast_translator.hpp"
#include "operators/import_csv.hpp"
#include "tpch/tpch_table_generator.hpp"
#include "storage/storage_manager.hpp"
#include "storage/table.hpp"
#include "scheduler/topology.hpp"
#include "scheduler/abstract_task.hpp"
#include "scheduler/abstract_scheduler.hpp"
#include "scheduler/current_scheduler.hpp"
#include "scheduler/node_queue_scheduler.hpp"
#include "scheduler/job_task.hpp"
#include "utils/load_table.hpp"

#include "tpch/tpch_queries.hpp"
#include "SQLParser.h"
#include "SQLParserResult.h"

int main() {
  opossum::StorageManager::get().add_table("customer", opossum::load_table("src/test/tables/tpch/customer.tbl", 2));
  opossum::StorageManager::get().add_table("lineitem", opossum::load_table("src/test/tables/tpch/lineitem.tbl", 2));
  opossum::StorageManager::get().add_table("nation", opossum::load_table("src/test/tables/tpch/nation.tbl", 2));
  opossum::StorageManager::get().add_table("orders", opossum::load_table("src/test/tables/tpch/orders.tbl", 2));
  opossum::StorageManager::get().add_table("part", opossum::load_table("src/test/tables/tpch/part.tbl", 2));
  opossum::StorageManager::get().add_table("partsupp", opossum::load_table("src/test/tables/tpch/partsupplier.tbl", 2));
  opossum::StorageManager::get().add_table("region", opossum::load_table("src/test/tables/tpch/region.tbl", 2));
  opossum::StorageManager::get().add_table("supplier", opossum::load_table("src/test/tables/tpch/supplier.tbl", 2));

  const auto query = std::string(opossum::tpch_queries[6]);

 // const auto query = R"(SELECT * FROM supplier, lineitem WHERE s_suppkey = l_suppkey AND s_name = 'Hans' ;)";

  hsql::SQLParserResult parser_result;
  hsql::SQLParser::parse(query, &parser_result);

  opossum::DotConfig config;
  config.background_color = opossum::GraphvizColor::Black;
  config.render_format = opossum::GraphvizRenderFormat::SVG;

  auto ast = opossum::SQLToASTTranslator(false).translate_parse_result(parser_result);
  opossum::ASTVisualizer(config).visualize(ast, "vis");
  ast[0]->print();

  auto join_graphs = opossum::JoinGraphBuilder::build_all_join_graphs(ast[0]);

  if (!join_graphs.empty()) {
    for (size_t join_graph_idx = 0; join_graph_idx < join_graphs.size(); ++join_graph_idx) {
      opossum::JoinGraphVisualizer(config).visualize(join_graphs[join_graph_idx], "join_graph" + std::to_string(join_graph_idx));
    }
  }

  auto astopt = opossum::Optimizer::get().optimize(ast[0]);
  opossum::ASTVisualizer(config).visualize({astopt}, "visopt");
  astopt->print();

  return 0;
}
