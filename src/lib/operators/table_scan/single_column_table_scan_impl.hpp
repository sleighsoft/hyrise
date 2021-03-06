#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "base_single_column_table_scan_impl.hpp"

#include "all_type_variant.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

/**
 * @brief Compares one column to a constant value
 *
 * - Value columns are scanned sequentially
 * - For dictionary columns, we basically look up the value ID of the constant value in the dictionary
 *   in order to avoid having to look up each value ID of the attribute vector in the dictionary. This also
 *   enables us to detect if all or none of the values in the column satisfy the expression.
 */
class SingleColumnTableScanImpl : public BaseSingleColumnTableScanImpl {
 public:
  SingleColumnTableScanImpl(std::shared_ptr<const Table> in_table, const ColumnID left_column_id,
                            const PredicateCondition& predicate_condition, const AllTypeVariant& right_value);

  PosList scan_chunk(ChunkID) override;

  void handle_column(const BaseValueColumn& base_column, std::shared_ptr<ColumnVisitableContext> base_context) override;

  void handle_column(const BaseDictionaryColumn& base_column,
                     std::shared_ptr<ColumnVisitableContext> base_context) override;

  void handle_column(const BaseEncodedColumn& base_column,
                     std::shared_ptr<ColumnVisitableContext> base_context) override;

  using BaseSingleColumnTableScanImpl::handle_column;

 private:
  /**
   * @defgroup Methods used for handling dictionary columns
   * @{
   */

  ValueID _get_search_value_id(const BaseDictionaryColumn& column) const;

  bool _right_value_matches_all(const BaseDictionaryColumn& column, const ValueID search_value_id) const;

  bool _right_value_matches_none(const BaseDictionaryColumn& column, const ValueID search_value_id) const;

  template <typename Functor>
  void _with_operator_for_dict_column_scan(const PredicateCondition predicate_condition, const Functor& func) const {
    switch (predicate_condition) {
      case PredicateCondition::Equals:
        func(std::equal_to<void>{});
        return;

      case PredicateCondition::NotEquals:
        func(std::not_equal_to<void>{});
        return;

      case PredicateCondition::LessThan:
      case PredicateCondition::LessThanEquals:
        func(std::less<void>{});
        return;

      case PredicateCondition::GreaterThan:
      case PredicateCondition::GreaterThanEquals:
        func(std::greater_equal<void>{});
        return;

      default:
        Fail("Unsupported comparison type encountered");
    }
  }
  /**@}*/

 private:
  const AllTypeVariant _right_value;
};

}  // namespace opossum
