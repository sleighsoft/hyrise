#pragma once

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "abstract_join_operator.hpp"
#include "storage/index/base_index.hpp"
#include "types.hpp"

namespace opossum {
/**
   * This operator joins two tables using one column of each table.
   * A speedup compared to the Nested Loop Join is achieved by avoiding the inner loop, and instead
   * finding the right values utilizing the index.
   *
   * Note: An index needs to be present on the right table in order to execute an index join.
   * Note: Cross joins are not supported. Use the product operator instead.
**/
class JoinIndex : public AbstractJoinOperator {
 public:
  JoinIndex(const std::shared_ptr<const AbstractOperator> left, const std::shared_ptr<const AbstractOperator> right,
            const JoinMode mode, const std::pair<ColumnID, ColumnID>& column_ids, const ScanType scan_type);

  const std::string name() const override;
  std::shared_ptr<AbstractOperator> recreate(const std::vector<AllParameterVariant>& args = {}) const override;

 protected:
  std::shared_ptr<const Table> _on_execute() override;

  void _perform_join();

  template <typename BinaryFunctor, typename LeftIterator, typename RightIterator>
  void _join_two_columns(const BinaryFunctor& func, LeftIterator left_it, LeftIterator left_end,
                         RightIterator right_begin, RightIterator right_end, const ChunkID chunk_id_left,
                         const ChunkID chunk_id_right, std::vector<bool>& left_matches);

  void append_matches(const BaseIndex::Iterator& range_begin, const BaseIndex::Iterator& range_end,
                      const ChunkOffset chunk_offset, std::vector<bool>& left_matches, ChunkID chunk_id_left,
                      std::function<RowID(ChunkOffset)> to_row_id);

  void _create_table_structure();

  void _write_output_chunk(Chunk& output_chunk, const std::shared_ptr<const Table> input_table,
                           std::shared_ptr<PosList> pos_list);

  std::shared_ptr<Table> _output_table;
  std::shared_ptr<const Table> _left_in_table;
  std::shared_ptr<const Table> _right_in_table;
  ColumnID _left_column_id;
  ColumnID _right_column_id;

  bool _is_outer_join;
  std::shared_ptr<PosList> _pos_list_left;
  std::shared_ptr<PosList> _pos_list_right;
  bool _fallback;

  // for Full Outer, remember the matches on the right side
  std::set<RowID> _right_matches;
};

}  // namespace opossum
