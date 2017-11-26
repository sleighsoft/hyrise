#pragma once

struct fn_utils {
  template <class _InputIterator, class _BinaryOperation>
  static void pairwise(_InputIterator __first, _InputIterator __last, _BinaryOperation __binary_op) {
    for (; __first != __last && __first + 1 != __last; ++__first) __binary_op(*__first, *(__first + 1));
  }
};
