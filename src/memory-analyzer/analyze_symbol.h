/*******************************************************************\

Module: Symbol Analyzer

Author: Malte Mues <mail.mues@gmail.com>
        Daniel Poetzl

\*******************************************************************/

// clang-format off
#if defined(__linux__) || \
    defined(__FreeBSD_kernel__) || \
    defined(__GNU__) || \
    defined(__unix__) || \
    defined(__CYGWIN__) || \
    defined(__MACH__)
// clang-format on

#ifndef CPROVER_MEMORY_ANALYZER_ANALYZE_SYMBOL_H
#define CPROVER_MEMORY_ANALYZER_ANALYZE_SYMBOL_H

#include <map>
#include <string>

#include "gdb_api.h"

#include <ansi-c/expr2c_class.h>

#include <util/allocate_objects.h>
#include <util/message.h>
#include <util/namespace.h>
#include <util/std_code.h>
#include <util/symbol_table.h>

class gdb_apit;
class exprt;
class source_locationt;

class symbol_analyzert
{
public:
  symbol_analyzert(const symbol_tablet &symbol_table, gdb_apit &gdb_api);

  /// For each input symbol in \p symbols: map its value address to its
  ///   \ref symbol_exprt (via the `values` map) and then call
  ///   \ref analyze_symbol on it.
  /// \param symbols: names of symbols to be analysed
  void analyze_symbols(const std::vector<std::string> &symbols);

  std::string get_snapshot_as_c_code();
  symbol_tablet get_snapshot_as_symbol_table();

  using pointer_valuet = gdb_apit::pointer_valuet;

private:
  gdb_apit &gdb_api;
  symbol_tablet symbol_table;
  const namespacet ns;
  expr2ct c_converter;
  allocate_objectst allocate_objects;

  std::vector<std::pair<exprt, exprt>> assignments;

  /// Mapping memory location (from \ref gdb_apit) to pointer expression for
  ///   which \ref get_non_char_pointer_value returned nil expression.
  std::map<exprt, std::string> outstanding_assignments;

  /// Storing pairs <address, symbol> such that at `address` is stored the
  ///   value of `symbol`.
  std::map<std::string, exprt> values;

  /// Assign the gdb-extracted value for \p symbol_name to its symbol
  ///   expression and then process outstanding assignments that this
  ///   extraction introduced.
  /// \param symbol_name: symbol table name to be analysed
  void analyze_symbol(const std::string &symbol_name);

  /// Create assignment \p lhs := \p value (see \ref analyze_symbol)
  /// \param lhs: the left-hand side of the assignment; expected to be a
  ///             \ref symbol_exprt
  /// \param value: the value to be assigned; the result of \ref get_expr_value
  void add_assignment(const exprt &lhs, const exprt &value);

  /// Iterate over \p array and fill its operands with the results of calling
  ///   \ref get_expr_value on index expressions into \p expr.
  /// \param expr: the expression to be analysed
  /// \param array: array with zero-initialised elements
  /// \param location: not used
  /// return an array of the same type as \p expr filled with values from gdb
  exprt get_array_value(
    const exprt &expr,
    const exprt &array,
    const source_locationt &location);

  /// Case analysis on the \ref typet of \p expr
  /// 1) For integers, booleans, and enums: call \ref gdb_apit::get_value
  ///    directly
  /// 2) For chars: return the \p zero_expr
  /// 3) For structs, arrays, and pointers: call their dedicated functions
  /// \param expr: the expression to be analysed
  /// \param zero_expr: zero of the same type as \p expr
  /// \param location: not used
  /// \return the value of the expression extracted from \ref gdb_apit
  exprt get_expr_value(
    const exprt &expr,
    const exprt &zero_expr,
    const source_locationt &location);

  /// For each of the members of the struct: call \ref get_expr_value
  /// \param expr: struct expression to be analysed
  /// \param zero_expr: struct with zero-initialised members
  /// \param location: not used
  /// \return the value of the struct from \ref gdb_apit
  exprt get_struct_value(
    const exprt &expr,
    const exprt &zero_expr,
    const source_locationt &location);

  /// Call \ref gdb_apit::get_memory on \p expr then split based on the
  ///   points-to type being `char` type or not. These have dedicated functions.
  /// \param expr: the input pointer expression
  /// \param zero_expr: null-pointer (or its equivalent)
  /// \param location: not used
  /// \return symbol expression associated with the gdb-extracted memory
  ///    location
  exprt get_pointer_value(
    const exprt &expr,
    const exprt &zero_expr,
    const source_locationt &location);

  /// Similar to \ref get_char_pointer_value. Doesn't re-call
  ///   \ref gdb_apit::get_memory, calls \ref get_expr_value on _dereferenced_
  ///   \p expr (the result of which is assigned to a new symbol).
  /// \param expr: the pointer expression to be analysed
  /// \param memory_location: pointer value from \ref gdb_apit::get_memory
  /// \param location: not used
  /// \return symbol expression associated with \p memory_location
  exprt get_non_char_pointer_value(
    const exprt &expr,
    const std::string memory_location,
    const source_locationt &location);

  /// If \p memory_location is found among \ref values then return the symbol
  ///   expression associated with it.
  /// Otherwise we add the appropriate \ref values mapping:
  /// 1) call \ref gdb_apit::get_memory on the \p expr
  ///    ??? Doesn't this have to be the same as \p memory_location
  /// 2) allocate new symbol and assign it with the memory string from 1)
  /// 3) fill \ref values (mapping \p memory_location to the new symbol)
  /// \param expr: the pointer expression to be analysed
  /// \param memory_location: pointer value from \ref gdb_apit::get_memory
  /// \param location: not used
  /// \return symbol expression associated with \p memory_location
  exprt get_char_pointer_value(
    const exprt &expr,
    const std::string &memory_location,
    const source_locationt &location);

  /// Call \ref add_assignment for each pair in \ref outstanding_assignments
  void process_outstanding_assignments();
};

#endif // CPROVER_MEMORY_ANALYZER_ANALYZE_SYMBOL_H
#endif
