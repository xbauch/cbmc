/*******************************************************************\

Module:

Author: Michael Tautschnig

\*******************************************************************/

#include "cprover_library.h"

#include <sstream>

#include <util/config.h>

#include <ansi-c/cprover_library.h>

static std::string get_cprover_library_text(
  const std::set<irep_idt> &functions,
  const symbol_tablet &symbol_table)
{
  std::ostringstream library_text;

  library_text << "#line 1 \"<builtin-library>\"\n"
               << "#undef inline\n";

  if(config.ansi_c.string_abstraction)
    library_text << "#define " CPROVER_PREFIX "STRING_ABSTRACTION\n";

  if(
    config.ansi_c.malloc_failure_mode ==
    config.ansi_c.malloc_failure_mode_return_null)
  {
    library_text << "#define " CPROVER_PREFIX
                    "MALLOC_FAILURE_MODE_RETURN_NULL\n";
  }
  if(
    config.ansi_c.malloc_failure_mode ==
    config.ansi_c.malloc_failure_mode_assert_then_assume)
  {
    library_text << "#define " CPROVER_PREFIX
                    "MALLOC_FAILURE_MODE_ASSERT_THEN_ASSUME\n";
  }

  // cprover_library.inc may not have been generated when running Doxygen, thus
  // make Doxygen skip this part
  /// \cond
  const struct cprover_library_entryt cprover_library[] =
#include "cprover_library.inc"
    ; // NOLINT(whitespace/semicolon)
  /// \endcond

  return get_cprover_library_text(
    functions, symbol_table, cprover_library, library_text.str());
}

void cprover_cpp_library_factory(
  const std::set<irep_idt> &functions,
  symbol_tablet &symbol_table,
  message_handlert &message_handler)
{
  if(config.ansi_c.lib == configt::ansi_ct::libt::LIB_NONE)
    return;

  const std::string library_text =
    get_cprover_library_text(functions, symbol_table);

  add_library(library_text, symbol_table, message_handler);
}
