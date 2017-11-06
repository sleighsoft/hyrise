#pragma once

#include <memory>
#include <vector>

#include "join_graph.hpp"

namespace opossum {

class AbstractASTNode;

/**
 * For building JoinGraphs from ASTs
 */
class JoinGraphBuilder final {
 public:
  /**
   * From the subtree of root, build a join graph.
   * The AST is not modified during this process.
   */
  std::shared_ptr<JoinGraph> build_join_graph(const std::shared_ptr<AbstractASTNode>& node);

  /**
   * Recursively search node for JoinGraphs
   */
  static std::vector<std::shared_ptr<JoinGraph>> build_all_join_graphs(const std::shared_ptr<AbstractASTNode>& node);

 private:
  bool _is_root_invocation = true;
  JoinGraph::Vertices _vertices;
  JoinGraph::Edges _edges;

  /**
   * Implementation backend of the public function build_all_join_graphs().
   */
  static std::vector<std::shared_ptr<JoinGraph>> _build_all_join_graphs(const std::shared_ptr<AbstractASTNode>& node,
                                                                        std::unordered_set<std::shared_ptr<AbstractASTNode>> & visited_nodes);

  /**
   * Helper method/actual implementation for build_join_graph().
   * @param node                The subtree to investigate
   * @param o_vertices          Output parameter, collecting all the vertex-AST-nodes
   * @param o_edges             Output parameter, collecting all edges/predicates in the tree
   * @param is_root_invocation  In the root multiple parents are allowed, everywhere else they are not and such nodes
   *                            will be considered vertices
   */
  void _traverse_ast_for_join_graph(const std::shared_ptr<AbstractASTNode>& node);

  // @{
  /**
   * When building a JoinGraph, these handle (i.e build vertices and edges for) specific node types
   */
  void _traverse_inner_join_node(const std::shared_ptr<JoinNode>& node);
  void _traverse_cross_join_node(const std::shared_ptr<JoinNode>& node);
  void _traverse_column_predicate_node(const std::shared_ptr<PredicateNode>& node);
  void _traverse_value_predicate_node(const std::shared_ptr<PredicateNode>& node);
  // @}

  /**
   * Within the index range [vertex_range_begin, vertex_range_end) in vertices, look for the `column_id`th column and
   * return the index of the Vertex it belongs to, as well as the ColumnID in that vertex
   */
  std::pair<JoinVertexID, ColumnID> _find_vertex_and_column_id(ColumnID column_id,
                                                                      JoinVertexID vertex_range_begin,
                                                                      JoinVertexID vertex_range_end);

  void _remove_redundant_cross_joins();
};

}