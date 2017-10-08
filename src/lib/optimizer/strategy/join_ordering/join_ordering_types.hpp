#pragma once

#include <limits>

namespace opossum {

using JoinVertexId = size_t;  // TODO(moritz) Strong typedef

constexpr static JoinVertexId INVALID_JOIN_VERTEX_ID = std::numeric_limits<JoinVertexId>::max();

struct JoinPredicate {
  // TODO(moritz) ensure no crosses and naturals here
  // TODO(moritz) Create with constructor
  JoinMode mode;
  std::pair<ColumnID, ColumnID> column_ids;
  ScanType scan_type;
};

struct JoinEdge {
  JoinPredicate predicate;
  std::pair<JoinVertexId, JoinVertexId> node_indices{INVALID_JOIN_VERTEX_ID, INVALID_JOIN_VERTEX_ID};
};
}