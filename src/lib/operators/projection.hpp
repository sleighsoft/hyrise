#pragma once

#include <cstdint>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "abstract_read_only_operator.hpp"
#include "boost/variant.hpp"
#include "optimizer/expression.hpp"
#include "storage/chunk.hpp"
#include "storage/dictionary_column.hpp"
#include "storage/reference_column.hpp"
#include "types.hpp"

namespace opossum {

struct CaseDefinition {
  struct When {
    ColumnID condition_column = INVALID_COLUMN_ID;
    AllParameterVariant then_value;
  };

  std::vector<When> whens;
  std::optional<AllParameterVariant> else_value;
  std::string result_type;

  std::string to_string() const;
};

struct ProjectionColumnDefinition final {
  ProjectionColumnDefinition(const std::shared_ptr<Expression> & expression, const std::optional<std::string> & alias = std::nullopt);
  ProjectionColumnDefinition(const CaseDefinition & expression, const std::optional<std::string> & alias = std::nullopt);

  boost::variant<std::shared_ptr<Expression>, CaseDefinition> value;
  std::optional<std::string> alias;
};

using ProjectionColumnDefinitions = std::vector<ProjectionColumnDefinition>;

/**
 * Operator to select subsets of columns in any order and to perform arithmetic operations on them.
 *
 * Note: Projection does not support null values at the moment
 */
class Projection : public AbstractReadOnlyOperator {
 public:
  Projection(const std::shared_ptr<const AbstractOperator> in, const ProjectionColumnDefinitions& column_definitions);

  const std::string name() const override;

  const ProjectionColumnDefinitions& column_definitions() const;

  std::shared_ptr<AbstractOperator> recreate(const std::vector<AllParameterVariant>& args) const override;

  /**
   * The dummy table is used for literal projections that have no input table.
   * This was introduce to allow queries like INSERT INTO tbl VALUES (1, 2, 3);
   * Because each INSERT uses a projection as input, the above case needs to project the three
   * literals (1, 2, 3) without any specific input table. Therefore, this dummy table is used instead.
   *
   * The dummy table contains one (value) column with one row. This way, the above projection
   * contains exactly one row with the given literals.
   */
  class DummyTable : public Table {
   public:
    DummyTable() : Table(0) {
      add_column("dummy", "int");
      append(std::vector<AllTypeVariant>{0});
    }
  };

  static std::shared_ptr<Table> dummy_table();

 protected:
  ProjectionColumnDefinitions _column_definitions;

  template <typename T>
  static void _create_column(boost::hana::basic_type<T> type, Chunk& o_chunk, const ChunkID chunk_id,
                             const std::shared_ptr<Expression>& expression,
                             std::shared_ptr<const Table> input_table_left);

  static const std::string _get_type_of_expression(const std::shared_ptr<Expression>& expression,
                                                   const std::shared_ptr<const Table>& table);

  /**
   * This function evaluates the given expression on a single chunk.
   * It returns a vector containing the materialized values resulting from the expression.
   */
  template <typename T>
  static const tbb::concurrent_vector<T> _evaluate_expression(const std::shared_ptr<Expression>& expression,
                                                              const std::shared_ptr<const Table> table,
                                                              const ChunkID chunk_id);

  /**
   * Operators that all numerical types support.
   */
  template <typename T>
  static std::function<T(const T&, const T&)> _get_base_operator_function(ExpressionType type);

  /**
   * Operators that integral types support.
   */
  template <typename T>
  static std::function<T(const T&, const T&)> _get_operator_function(ExpressionType type);

  std::shared_ptr<const Table> _on_execute() override;
};

}  // namespace opossum
