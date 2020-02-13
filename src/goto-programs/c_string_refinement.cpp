/*******************************************************************\

Module: C String Refinement

Author: diffblue

\*******************************************************************/

/// \file
/// C String Refinement

#include "c_string_refinement.h"

#include <util/c_types.h>
#include <util/mathematical_expr.h>
#include <util/mathematical_types.h>
#include <util/message.h>
#include <util/string2int.h>
#include <util/string_expr.h>

#include <goto-programs/goto_model.h>

struct c_string_refinementt : public messaget
{
  c_string_refinementt(
    symbol_tablet &symbol_table,
    message_handlert &message_handler,
    size_t max_nondet_string_length)
    : messaget{message_handler},
      symbol_table(symbol_table),
      ns(symbol_table),
      max_nondet_string_length(max_nondet_string_length)
  {
  }

  void operator()(goto_functionst &goto_functions);

private:
  symbol_tablet &symbol_table;
  namespacet ns;
  size_t max_nondet_string_length;
  size_t symbol_counter = 0;
  const typet string_size_type = size_type();
  const typet string_index_type = index_type();

  auxiliary_symbolt new_aux_string_symbol(
    const std::string &name,
    const typet &type,
    symbol_table_baset &symbol_table);

  refined_string_exprt char_ptr_to_refined_string(
    const exprt &char_ptr,
    const symbol_exprt &length_symbol_expr,
    goto_programt &program);

  void do_string_index_of(goto_functiont &string_index_of);
};

auxiliary_symbolt c_string_refinementt::new_aux_string_symbol(
  const std::string &name,
  const typet &type,
  symbol_table_baset &symbol_table)
{
  const auto decorated_name =
    std::string{name} + std::to_string(++symbol_counter);
  auxiliary_symbolt new_symbol;
  new_symbol.base_name = decorated_name;
  new_symbol.pretty_name = decorated_name;
  new_symbol.is_static_lifetime = false;
  new_symbol.is_state_var = false;
  new_symbol.mode = ID_C;
  new_symbol.name = decorated_name;
  new_symbol.type = type;
  symbol_table.add(new_symbol);

  return new_symbol;
}

void c_string_refinementt::operator()(goto_functionst &goto_functions)
{
  for(auto it = goto_functions.function_map.begin();
      it != goto_functions.function_map.end();
      it++)
  {
    if(it->first == "__CPROVER_math_func_string_index_of")
    {
      do_string_index_of(it->second);
    }
  }
}

void c_string_refinementt::do_string_index_of(goto_functiont &string_index_of)
{
  const auto &params = to_code_type(string_index_of.type).parameters();
  PRECONDITION(params.size() == 3);
  const auto &str_param = params.at(0);
  const auto &length_param = params.at(1);
  const auto &ch_param = params.at(2);
  // const auto refined_string_type =
  //   refined_string_typet{size_type(), to_pointer_type(str_param.type())};
  const auto str_param_symbol =
    symbol_exprt{str_param.get_identifier(), str_param.type()};
  const auto length_param_symbol =
    symbol_exprt{length_param.get_identifier(), length_param.type()};
  const auto ch_param_symbol =
    symbol_exprt{ch_param.get_identifier(), ch_param.type()};
  goto_programt new_body;
  const auto refined_string =
    char_ptr_to_refined_string(str_param_symbol, length_param_symbol, new_body);

  const auto cprover_string_index_of_func_symbol = symbol_exprt{
    ID_cprover_string_index_of_func,
    mathematical_function_typet(
      {refined_string.type(), ch_param_symbol.type()}, string_index_type)};
  const auto apply_index_of = function_application_exprt{
    cprover_string_index_of_func_symbol, {refined_string, ch_param_symbol}};
  const auto return_symbol = new_aux_string_symbol(
    "StringIndexOf::return", string_index_type, symbol_table);
  const auto return_expr = return_symbol.symbol_expr();
  new_body.add(goto_programt::make_decl(return_expr));
  new_body.add(goto_programt::make_assignment(return_expr, apply_index_of));
  new_body.add(goto_programt::make_return(code_returnt{return_expr}));
  new_body.add(goto_programt::make_end_function());

  string_index_of.body = std::move(new_body);
}

/// Helper function to produce the necessary associations related to
/// char-pointers needed for string solver.
refined_string_exprt c_string_refinementt::char_ptr_to_refined_string(
  const exprt &char_ptr,
  const symbol_exprt &length_symbol_expr,
  goto_programt &program)
{
  // char *string_content = src;
  const auto content_symbol = new_aux_string_symbol(
    "cprover_string_index_of_func::string_content",
    char_ptr.type(),
    symbol_table);
  const auto content_symbol_expr = content_symbol.symbol_expr();
  program.add(goto_programt::make_decl(content_symbol_expr));
  program.add(goto_programt::make_assignment(content_symbol_expr, char_ptr));

  // char (*nondet_infinite_array_ponter)[\infty];
  const symbolt nondet_infinite_array_pointer = new_aux_string_symbol(
    "cprover_string_index_of_func::nondet_infinite_array_pointer",
    pointer_type(array_typet{char_type(), infinity_exprt(string_size_type)}),
    symbol_table);
  const symbol_exprt nondet_infinite_array_pointer_expr =
    nondet_infinite_array_pointer.symbol_expr();
  program.add(goto_programt::make_decl(nondet_infinite_array_pointer_expr));
  program.add(goto_programt::make_assignment(
    nondet_infinite_array_pointer_expr,
    typecast_exprt{address_of_exprt{char_ptr},
                   nondet_infinite_array_pointer_expr.type()}));

  // cprover_associate_length_to_array_func(nondet_infinite_array_pointer,
  //                                        string_length);
  const auto refined_string_expr = refined_string_exprt{
    length_symbol_expr,
    content_symbol_expr,
    refined_string_typet{length_symbol_expr.type(),
                         to_pointer_type(char_ptr.type())}};
  const symbolt return_array_length = new_aux_string_symbol(
    "cprover_string_index_of_func::return_array_length",
    string_index_type,
    symbol_table);
  dereference_exprt nondet_array_expr{nondet_infinite_array_pointer_expr};
  const auto cprover_associate_length_to_array_func_symbol =
    symbol_exprt{ID_cprover_associate_length_to_array_func,
                 mathematical_function_typet{
                   {nondet_array_expr.type(), length_symbol_expr.type()},
                   return_array_length.type}};
  const auto apply_associate_length_to_array = function_application_exprt{
    cprover_associate_length_to_array_func_symbol,
    {nondet_array_expr, refined_string_expr.length()},
    return_array_length.type};
  const auto return_length_expr = return_array_length.symbol_expr();
  program.add(goto_programt::make_decl(return_length_expr));
  program.add(goto_programt::make_assignment(
    return_length_expr, apply_associate_length_to_array));

  // cprover_associate_array_to_pointer_func(nondet_infinite_array_pointer,
  //                                         src);
  const address_of_exprt array_pointer(
    index_exprt(nondet_array_expr, from_integer(0, string_index_type)));
  program.add(goto_programt::make_assignment(array_pointer, char_ptr));
  const symbolt return_sym_pointer = new_aux_string_symbol(
    "return_array_pointer", string_index_type, symbol_table);
  const auto cprover_associate_array_to_pointer_func_symbol =
    symbol_exprt{ID_cprover_associate_array_to_pointer_func,
                 mathematical_function_typet{
                   {nondet_array_expr.type(), array_pointer.type()},
                   return_sym_pointer.type}};
  const auto apply_associate_pointer_to_array =
    function_application_exprt{cprover_associate_array_to_pointer_func_symbol,
                               {nondet_array_expr, char_ptr}};
  const auto return_pointer_expr = return_sym_pointer.symbol_expr();
  program.add(goto_programt::make_decl(return_pointer_expr));
  program.add(goto_programt::make_assignment(
    return_pointer_expr, apply_associate_pointer_to_array));

  return refined_string_exprt{refined_string_expr.length(), char_ptr};
}

void c_string_refinement(
  goto_modelt &goto_model,
  message_handlert &message_handler,
  const std::string &maybe_max_nondet_string_length_string)
{
  size_t max_nondet_string_length = 1000; // default
  if(!maybe_max_nondet_string_length_string.empty())
  {
    auto maybe_max_nondet_string_length =
      string2optional_size_t(maybe_max_nondet_string_length_string);
    if(!maybe_max_nondet_string_length.has_value())
    {
      throw invalid_command_line_argument_exceptiont{
        "max string length should be a number", "--max-nondet-string-length"};
    }
    max_nondet_string_length = *maybe_max_nondet_string_length;
  }
  auto c_string_refinement = c_string_refinementt{
    goto_model.symbol_table, message_handler, max_nondet_string_length};

  c_string_refinement(goto_model.goto_functions);
}
