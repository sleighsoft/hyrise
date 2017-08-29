#include "aggregate_node.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "optimizer/expression/expression.hpp"

#include "common.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

AggregateNode::AggregateNode(const std::vector<std::shared_ptr<Expression>>& aggregate_expressions,
                             const std::vector<ColumnID>& groupby_column_ids)
    : AbstractASTNode(ASTNodeType::Aggregate),
      _aggregate_expressions(aggregate_expressions),
      _groupby_column_ids(groupby_column_ids) {
  for (const auto& expression : aggregate_expressions) {
    DebugAssert(expression->type() == ExpressionType::FunctionIdentifier, "Aggregate expression must be a function.");
  }
}

const std::vector<std::shared_ptr<Expression>>& AggregateNode::aggregate_expressions() const {
  return _aggregate_expressions;
}

const std::vector<ColumnID>& AggregateNode::groupby_column_ids() const { return _groupby_column_ids; }

std::string AggregateNode::description() const {
  std::ostringstream s;

  auto stream_aggregate = [&](const std::shared_ptr<Expression>& aggregate_expr) {
    s << aggregate_expr->to_string();
    if (aggregate_expr->alias()) s << " AS \"" << (*aggregate_expr->alias()) << "\"";
  };

  auto it = _aggregate_expressions.begin();
  if (it != _aggregate_expressions.end()) stream_aggregate(*it);
  for (; it != _aggregate_expressions.end(); ++it) {
    s << ", ";
    stream_aggregate(*it);
  }

  if (!_groupby_column_ids.empty()) {
    s << " GROUP BY [";
    for (const auto& column_name : _groupby_column_ids) {
      s << column_name << ", ";
    }
    s << "]";
  }

  return s.str();
}

void AggregateNode::_on_child_changed() {
  DebugAssert(!!left_child(), "AggregateNode needs a child.");

  _output_column_names.clear();
  _output_column_ids.clear();

  _output_column_names.reserve(_groupby_column_ids.size() + _aggregate_expressions.size());
  _output_column_ids.reserve(_groupby_column_ids.size() + _aggregate_expressions.size());

  /**
   * Set output column ids and names.
   *
   * The Aggregate operator will put all GROUP BY columns in the output table at the beginning,
   * so we first handle those, and afterwards add the column information for the aggregate functions.
   */
  for (const auto groupby_column_id : _groupby_column_ids) {
    _output_column_ids.emplace_back(groupby_column_id);
    _output_column_names.emplace_back(left_child()->output_column_names()[groupby_column_id]);
  }

  for (const auto& aggregate_expression : _aggregate_expressions) {
    DebugAssert(aggregate_expression->type() == ExpressionType::FunctionIdentifier, "Expression must be a function.");

    std::string column_name;

    if (aggregate_expression->alias()) {
      column_name = *aggregate_expression->alias();
    } else {
      /**
       * If the aggregate function has no alias defined in the query, we simply parse the expression back to a string.
       * SQL in the standard does not specify a name to be given.
       * This might result in multiple output columns with the same name, but we accept that.
       * Other DBs behave similarly (e.g. MySQL).
       */
      column_name = aggregate_expression->to_string(left_child());
    }

    _output_column_names.emplace_back(column_name);
    _output_column_ids.emplace_back(INVALID_COLUMN_ID);
  }
}

const std::vector<std::string>& AggregateNode::output_column_names() const { return _output_column_names; }

const std::vector<ColumnID>& AggregateNode::output_column_ids() const { return _output_column_ids; }

optional<ColumnID> AggregateNode::find_column_id_for_column_identifier_name(
    const ColumnIdentifierName& column_identifier_name) const {
  DebugAssert(!!left_child(), "AggregateNode needs a child.");

  // TODO(mp) Handle column_identifier_name having a table that is this node's alias

  /*
   * Search for ColumnIdentifierName in Aggregate columns ALIASes, if the column_identifier_name has no table.
   * These columns are created by the Aggregate Operator, so we have to look through them here.
   */
  optional<ColumnID> column_id_aggregate;
  if (!column_identifier_name.table_name) {
    for (auto aggregate_idx = 0u; aggregate_idx < _aggregate_expressions.size(); aggregate_idx++) {
      const auto& aggregate_expression = _aggregate_expressions[aggregate_idx];

      // If AggregateDefinition has no alias, column_name will not match.
      if (column_identifier_name.column_name == aggregate_expression->alias()) {
        // Check that we haven't found a match yet.
        Assert(!column_id_aggregate, "Column name " + column_identifier_name.column_name + " is ambiguous.");
        // Aggregate columns come after Group By columns in the Aggregate's output
        column_id_aggregate = ColumnID{static_cast<ColumnID::base_type>(aggregate_idx + _groupby_column_ids.size())};
      }
    }
  }

  /*
   * Search for ColumnIdentifierName in Group By columns.
   * These columns have been created by another node. Since Aggregates can only have a single child node,
   * we just have to check the left_child for the ColumnIdentifierName.
   */
  optional<ColumnID> column_id_groupby;
  const auto column_id_child = left_child()->find_column_id_for_column_identifier_name(column_identifier_name);
  if (column_id_child) {
    const auto iter = std::find(_groupby_column_ids.begin(), _groupby_column_ids.end(), *column_id_child);
    if (iter != _groupby_column_ids.end()) {
      column_id_groupby = ColumnID{static_cast<ColumnID::base_type>(std::distance(_groupby_column_ids.begin(), iter))};
    }
  }

  // Max one can be set, both not being set is fine, as we are in a find_* method
  Assert(!column_id_aggregate || !column_id_groupby,
         "Column name " + column_identifier_name.column_name + " is ambiguous.");

  if (column_id_aggregate) {
    return column_id_aggregate;
  }

  // Optional might not be set.
  return column_id_groupby;
}

ColumnID AggregateNode::get_column_id_for_expression(const std::shared_ptr<Expression>& expression) const {
  const auto column_id = find_column_id_for_expression(expression);
  DebugAssert(!!column_id, "Expression could not be resolved.");
  return *column_id;
}

optional<ColumnID> AggregateNode::find_column_id_for_expression(
    const std::shared_ptr<Expression>& expression) const {
  /**
   * This function does NOT need to check whether an expression is ambiguous.
   * It is only used when translating the HAVING clause.
   * If two expressions are equal, they must refer to the same result.
   * Not checking ambiguity allows perfectly valid queries like:
   *  SELECT a, MAX(b), MAX(b) FROM t GROUP BY a HAVING MAX(b) > 0
   */
  if (expression->type() == ExpressionType::ColumnIdentifier) {
    const auto iter = std::find_if(_groupby_column_ids.begin(), _groupby_column_ids.end(),
                                   [&](const auto& rhs) { return expression->column_id() == rhs; });

    if (iter != _groupby_column_ids.end()) {
      const auto idx = std::distance(_groupby_column_ids.begin(), iter);
      return ColumnID{static_cast<ColumnID::base_type>(idx)};
    }
  } else if (expression->type() == ExpressionType::FunctionIdentifier) {
    const auto iter = std::find_if(_aggregate_expressions.begin(), _aggregate_expressions.end(), [&](const auto& rhs) {
      DebugAssert(!!rhs, "Aggregate expressions can not be nullptr!");
      return *expression == *rhs;
    });

    if (iter != _aggregate_expressions.end()) {
      const auto idx = std::distance(_aggregate_expressions.begin(), iter);
      return ColumnID{static_cast<ColumnID::base_type>(idx + _groupby_column_ids.size())};
    }
  } else {
    Fail("Expression type is not supported.");
  }

  // Return unset optional if expression was not found.
  return nullopt;
}

std::vector<ColumnID> AggregateNode::get_column_ids_for_table(const std::string& table_name) const {
  DebugAssert(!!left_child(), "AggregateNode needs a child.");

  if (!left_child()->manages_table(table_name)) {
    return {};
  }

  const auto input_column_ids_for_table = left_child()->get_column_ids_for_table(table_name);

  std::vector<ColumnID> output_column_ids_for_table;

  for (const auto input_column_id : input_column_ids_for_table) {
    const auto iter = std::find(_groupby_column_ids.begin(), _groupby_column_ids.end(), input_column_id);

    if (iter != _groupby_column_ids.end()) {
      const auto index = std::distance(_groupby_column_ids.begin(), iter);
      output_column_ids_for_table.emplace_back(static_cast<ColumnID::base_type>(index));
    }
  }

  return output_column_ids_for_table;
}

}  // namespace opossum
