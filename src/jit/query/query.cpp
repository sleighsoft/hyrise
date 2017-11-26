#include "query.hpp"

#include <iostream>

#include "operators/source.hpp"

const std::vector<Operator::Ptr>& Query::operators() const { return _operators; }

const Source::Ptr Query::source() const { return std::dynamic_pointer_cast<const Source>(_operators.back()); }
