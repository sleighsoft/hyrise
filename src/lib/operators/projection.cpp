#include <algorithm>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "projection.hpp"
#include "storage/reference_column.hpp"
#include "termfactory.hpp"

#include "resolve_type.hpp"

namespace opossum {

Projection::Projection(const std::shared_ptr<const AbstractOperator> in, const ProjectionDefinitions& columns)
    : AbstractReadOnlyOperator(in), _projection_definitions(columns) {}

Projection::Projection(const std::shared_ptr<const AbstractOperator> in, const std::vector<ColumnID>& columns)
    : AbstractReadOnlyOperator(in), _simple_projection(columns) {}

const Projection::ProjectionDefinitions& Projection::projection_definitions() const { return _projection_definitions; }

const std::vector<ColumnID>& Projection::simple_projection() const { return _simple_projection; }

const std::string Projection::name() const { return "Projection"; }

uint8_t Projection::num_in_tables() const { return 1; }

uint8_t Projection::num_out_tables() const { return 1; }

std::shared_ptr<AbstractOperator> Projection::recreate(const std::vector<AllParameterVariant>& args) const {
  if (!_simple_projection.empty()) {
    return std::make_shared<Projection>(_input_left->recreate(args), _simple_projection);
  }
  return std::make_shared<Projection>(_input_left->recreate(args), _projection_definitions);
}

std::shared_ptr<const Table> Projection::on_execute() {
  if (!_simple_projection.empty()) {
    for (auto& column_id : _simple_projection) {
      // TODO(mp): fix this with new projection. E.g., $a does not signal whether it is the first or the second column a
      _projection_definitions.emplace_back(std::string("$") + input_table_left()->column_name(column_id),
                                           input_table_left()->column_type(column_id),
                                           input_table_left()->column_name(column_id));
    }
  }

  auto output = std::make_shared<Table>();

  // Prepare terms and output table for each column to project
  for (auto& definition : _projection_definitions) {
    output->add_column_definition(definition.name, definition.type);
  }

  for (ChunkID chunk_id{0}; chunk_id < input_table_left()->chunk_count(); ++chunk_id) {
    // fill the new table
    Chunk chunk_out;
    // if there is mvcc information, we have to link it
    if (input_table_left()->get_chunk(chunk_id).has_mvcc_columns()) {
      chunk_out.use_mvcc_columns_from(input_table_left()->get_chunk(chunk_id));
    }
    for (auto definition : _projection_definitions) {
      call_functor_by_column_type<ColumnCreator>(definition.type, chunk_out, chunk_id, definition.expression,
                                                 input_table_left());
    }
    output->add_chunk(std::move(chunk_out));
  }
  return output;
}

}  // namespace opossum
