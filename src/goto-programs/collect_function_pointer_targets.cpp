/*******************************************************************\

Module: Program Transformation

Author: diffblue

\*******************************************************************/

/// \file
/// Program Transformation

#include "collect_function_pointer_targets.h"
collect_function_pointer_targetst::collect_function_pointer_targetst(
  message_handlert &message_handler,
  const symbol_tablet &symbol_table,
  bool only_resolve_const_fps)
  : log(message_handler),
    ns(symbol_table),
    symbol_table(symbol_table),
    only_resolve_const_fps(only_resolve_const_fps),
    initialised(false)
{
}

void collect_function_pointer_targetst::initialise_taken_addresses(
  const goto_functionst &goto_functions)
{
  for(const auto &s : symbol_table.symbols)
    compute_address_taken_functions(s.second.value, address_taken);

  compute_address_taken_functions(goto_functions, address_taken);
}

void collect_function_pointer_targetst::initialise_type_map(
  const goto_functionst &goto_functions)
{
  for(const auto &fmap_pair : goto_functions.function_map)
    type_map.emplace(fmap_pair.first, fmap_pair.second.type);
}

code_typet collect_function_pointer_targetst::refine_call_type(
  const typet &type,
  const code_function_callt &code)
{
  PRECONDITION(can_cast_type<code_typet>(type));
  code_typet call_type = to_code_type(type);

  if(call_type.has_ellipsis() && call_type.parameters().empty())
  {
    call_type.remove_ellipsis();
    for(const auto &argument : code.arguments())
      call_type.parameters().push_back(code_typet::parametert{argument.type()});
  }
  return call_type;
}

irep_idt
collect_function_pointer_targetst::get_callee_id(const exprt &called_function)
{
  irep_idt callee_id;
  bool contains_code = false;
  auto type_contains_code = [&contains_code](const typet &t) {
    if(t.id() == ID_code)
      contains_code = true;
  };

  called_function.visit_post(
    [&callee_id, &type_contains_code, &contains_code](const exprt &e) {
      if(e.id() == ID_symbol)
      {
        e.type().visit(type_contains_code);
        if(contains_code)
        {
          callee_id = to_symbol_expr(e).get_identifier();
          return;
        }
      }
      if(e.id() == ID_dereference)
      {
        const auto &pointer = to_dereference_expr(e).pointer();
        if(pointer.id() == ID_symbol)
          callee_id = to_symbol_expr(pointer).get_identifier();
        if(pointer.id() == ID_member)
        {
          pointer.type().visit(type_contains_code);
          if(contains_code)
            callee_id = to_member_expr(pointer).get_component_name();
        }
      }
    });
  return callee_id;
}

bool collect_function_pointer_targetst::arg_is_type_compatible(
  const typet &call_type,
  const typet &function_type,
  const namespacet &name_space)
{
  if(call_type == function_type)
    return true;

  // any integer-vs-enum-vs-pointer is ok
  if(
    call_type.id() == ID_signedbv || call_type.id() == ID_unsigned ||
    call_type.id() == ID_bool || call_type.id() == ID_c_bool ||
    call_type.id() == ID_c_enum_tag || call_type.id() == ID_c_enum ||
    call_type.id() == ID_pointer)
  {
    return function_type.id() == ID_signedbv ||
           function_type.id() == ID_unsigned || function_type.id() == ID_bool ||
           function_type.id() == ID_c_bool ||
           function_type.id() == ID_pointer ||
           function_type.id() == ID_c_enum ||
           function_type.id() == ID_c_enum_tag;
  }

  return pointer_offset_bits(call_type, name_space) ==
         pointer_offset_bits(function_type, name_space);
}

bool collect_function_pointer_targetst::is_type_compatible(
  bool return_value_used,
  const code_typet &call_type,
  const code_typet &function_type,
  const namespacet &name_space)
{
  // we are willing to ignore anything that's returned
  // if we call with 'void'
  if(
    return_value_used && call_type.return_type() != empty_typet() &&
    !arg_is_type_compatible(
      call_type.return_type(), function_type.return_type(), name_space))
    return false;

  // let's look at the parameters
  const auto &call_parameters = call_type.parameters();
  const auto &function_parameters = function_type.parameters();

  if(function_type.has_ellipsis() && function_parameters.empty())
    return true;
  if(call_type.has_ellipsis() && call_parameters.empty())
    return true;

  // we are quite strict here, could be much more generous
  if(call_parameters.size() != function_parameters.size())
    return false;

  for(std::size_t i = 0; i < call_parameters.size(); i++)
    if(!arg_is_type_compatible(
         call_parameters[i].type(), function_parameters[i].type(), name_space))
      return false;
  return true;
}

possible_fp_targets_mapt get_function_pointer_targets(
  message_handlert &message_handler,
  const goto_functionst &goto_functions,
  const symbol_tablet &symbol_table,
  bool only_resolve_const_fps)
{
  collect_function_pointer_targetst collector(
    message_handler, symbol_table, only_resolve_const_fps);

  return collector(goto_functions);
}
