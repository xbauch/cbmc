/*******************************************************************\

  Module: Unit tests for goto_program::validate

  Author: Diffblue Ltd.

 \*******************************************************************/

#include <goto-programs/goto_function.h>
#include <testing-utils/catch.hpp>
#include <util/arith_tools.h>

SCENARIO(
  "Validation of well-formed assert/assume codes",
  "[core][goto-programs][validate]")
{
  GIVEN("A program with one assertion")
  {
    symbol_tablet symbol_table;
    const typet type1 = signedbv_typet(32);
    symbolt symbol;
    irep_idt symbol_name = "a";
    symbol.name = symbol_name;
    symbol_exprt varx(symbol_name, type1);
    exprt val10 = from_integer(10, type1);
    plus_exprt x_plus_10(varx, val10);
    binary_relation_exprt x_le_10(varx, ID_le, val10);

    goto_functiont goto_function;
    auto &instructions = goto_function.body.instructions;
    instructions.emplace_back(goto_program_instruction_typet::ASSERT);

    symbol.type = type1;
    symbol_table.insert(symbol);
    namespacet ns(symbol_table);

    WHEN("Condition evaluates to a Boolean")
    {
      instructions.back().make_assertion(x_le_10);
      THEN("The consistency check succeeds")
      {
        goto_function.body.validate(symbol_table, validation_modet::INVARIANT);
        REQUIRE(true);
      }
    }

    WHEN("Condition does not evaluate to a Boolean")
    {
      instructions.back().make_assertion(x_plus_10);

      THEN("The consistency check fails")
      {
        bool caught = false;
        try
        {
          goto_function.body.validate(
            symbol_table, validation_modet::EXCEPTION);
        }
        catch(incorrect_goto_program_exceptiont &e)
        {
          caught = true;
        }
        REQUIRE(caught);
      }
    }
  }
}
