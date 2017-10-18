#pragma once

#include <memory>
#include <optional>

#include "gtest/gtest.h"

#include "all_parameter_variant.hpp"
#include "all_type_variant.hpp"
#include "types.hpp"

namespace opossum {

class AbstractASTNode;
class Table;

::testing::AssertionResult check_table_equal(const Table& tleft, const Table& tright, bool order_sensitive,
                                             bool strict_types);
::testing::AssertionResult check_predicate_node(const std::shared_ptr<AbstractASTNode>& node, ColumnID column_id,
                                                ScanType scan_type, const AllParameterVariant& value,
                                                const std::optional<AllTypeVariant>& value2 = std::nullopt);

void EXPECT_TABLE_EQ(const Table& tleft, const Table& tright, bool order_sensitive = false, bool strict_types = true);
void ASSERT_TABLE_EQ(const Table& tleft, const Table& tright, bool order_sensitive = false, bool strict_types = true);

void EXPECT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
                     bool order_sensitive = false, bool strict_types = true);
void ASSERT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
                     bool order_sensitive = false, bool strict_types = true);

void ASSERT_INNER_JOIN_NODE(const std::shared_ptr<AbstractASTNode>& node, ScanType scanType, ColumnID left_column_id,
                            ColumnID right_column_id);

void ASSERT_CROSS_JOIN_NODE(const std::shared_ptr<AbstractASTNode>& node);

void ASSERT_PREDICATE_NODE(const std::shared_ptr<AbstractASTNode>& node, ColumnID column_id,
                           ScanType scan_type, const AllParameterVariant& value,
                           const std::optional<AllTypeVariant>& value2 = std::nullopt);
}  // namespace opossum
