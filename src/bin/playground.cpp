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
  opossum::StorageManager::get().add_table("customer", opossum::load_table("tpch_tables/customer_head.tbl", 2000));
  opossum::StorageManager::get().add_table("lineitem", opossum::load_table("tpch_tables/lineitem_head.tbl", 2000));
  opossum::StorageManager::get().add_table("nation", opossum::load_table("tpch_tables/nation_head.tbl", 2000));
  opossum::StorageManager::get().add_table("orders", opossum::load_table("tpch_tables/orders_head.tbl", 2000));
  opossum::StorageManager::get().add_table("part", opossum::load_table("tpch_tables/part_head.tbl", 2000));
  opossum::StorageManager::get().add_table("partsupp", opossum::load_table("tpch_tables/partsupp_head.tbl", 2000));
  opossum::StorageManager::get().add_table("region", opossum::load_table("tpch_tables/region_head.tbl", 2000));
  opossum::StorageManager::get().add_table("supplier", opossum::load_table("tpch_tables/supplier_head.tbl", 2000));

 // const auto query = std::string(opossum::tpch_queries[6]);
 // const auto query = R"(SELECT * FROM supplier, lineitem WHERE s_suppkey = l_suppkey AND s_name = 'Hans' ;)";

  const char* const modded_query =
    R"(SELECT
          supp_nation, cust_nation, l_year, SUM(volume) as revenue
      FROM
          (SELECT
                n1.n_name as supp_nation,
                n2.n_name as cust_nation,
                l_shipdate as l_year,
                l_extendedprice * (1 - l_discount) as volume
          FROM
                supplier, lineitem, orders, customer, nation n1, nation n2
          WHERE
                s_suppkey = l_suppkey AND
                o_orderkey = l_orderkey AND
                c_custkey = o_custkey AND
                s_nationkey = n1.n_nationkey AND
                c_nationkey = n2.n_nationkey AND
                ((n1.n_name = 'GERMANY' AND n2.n_name = 'FRANCE') OR
                 (n1.n_name = 'FRANCE' AND n2.n_name = 'GERMANY'))
                AND l_shipdate between '1995-01-01' AND '1996-12-31') as shipping
          GROUP BY
              supp_nation, cust_nation, l_year
      ORDER BY
          supp_nation, cust_nation, l_year;)";

  hsql::SQLParserResult parser_result;
  hsql::SQLParser::parse(modded_query, &parser_result);

  opossum::DotConfig config;
  config.background_color = opossum::GraphvizColor::Black;
  config.render_format = opossum::GraphvizRenderFormat::PNG;

  auto ast = opossum::SQLToASTTranslator(false).translate_parse_result(parser_result);
  opossum::ASTVisualizer(config).visualize(ast, "ast");
  ast[0]->print();

  auto join_graphs = opossum::JoinGraphBuilder::build_all_join_graphs(ast[0]);

  if (!join_graphs.empty()) {
    for (size_t join_graph_idx = 0; join_graph_idx < join_graphs.size(); ++join_graph_idx) {
      opossum::JoinGraphVisualizer(config).visualize(join_graphs[join_graph_idx], "join_graph" + std::to_string(join_graph_idx));
    }
  }

  auto astopt = opossum::Optimizer::get().optimize(ast[0]);
  opossum::ASTVisualizer(config).visualize({astopt}, "astopt");
  astopt->print();

  return 0;
}
