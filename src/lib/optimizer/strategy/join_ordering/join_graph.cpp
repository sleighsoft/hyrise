#include "join_graph.hpp"

#include <utility>

#include "constant_mappings.hpp"

namespace opossum {

JoinGraph::JoinGraph(Vertices && vertices, Edges && edges):
  _vertices(std::move(vertices)),
  _edges(std::move(edges))
{}

const JoinGraph::Vertices& JoinGraph::vertices() const {
  return _vertices;
}

const JoinGraph::Edges& JoinGraph::edges() const {
  return _edges;
}

void JoinGraph::print(std::ostream& out) const {
  out << "==== JoinGraph ====" << std::endl;
  out << "==== Vertices ====" << std::endl;
  for (size_t vertex_idx = 0; vertex_idx < _vertices.size(); ++vertex_idx) {
    const auto & vertex = _vertices[vertex_idx];
    std::cout << vertex_idx << ":  " << vertex->description() << std::endl;
  }
  out << "==== Edges ====" << std::endl;
  for (const auto & edge : _edges) {
    std::cout << edge.node_indices.first << " <-- "
              << edge.predicate.column_ids.first << " "
              << scan_type_to_string.left.at(edge.predicate.scan_type) << " "
              << edge.predicate.column_ids.second << " --> " << edge.node_indices.second << std::endl;
  }

  out << "===================" << std::endl;
}

}