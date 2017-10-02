#pragma once

namespace opossum {

struct JoinPredicate {
  // TODO(moritz) ensure no crosses and naturals here
  // TODO(moritz) Create with constructor
  JoinMode mode;
  std::pair<ColumnID, ColumnID> column_ids;
  ScanType scan_type;
};

struct JoinEdge {
  JoinPredicate predicate;
  size_t node_indices[2] = {0, 0};
};

}