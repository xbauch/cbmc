/*******************************************************************\

Module: GDB Machine Interface API

Author: Malte Mues <mail.mues@gmail.com>
        Daniel Poetzl

\*******************************************************************/

/// \file
/// Low-level interface to gdb
///
/// This provides an API to run a program under gdb up to some
/// breakpoint, and then query the values of expressions. It uses the
/// gdb machine interface (see section "The GDB/MI Interface" in the
/// gdb manual to communicate with gdb.

#ifndef CPROVER_MEMORY_ANALYZER_GDB_API_H
#define CPROVER_MEMORY_ANALYZER_GDB_API_H
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <forward_list>

#include <util/exception_utils.h>

/// Interface for running and querying GDB
class gdb_apit
{
public:
  using commandst = std::forward_list<std::string>;
  struct memory_addresst
  {
    bool null_address;
    std::string address_string;
    memory_addresst() : null_address(true)
    {
    }
    explicit memory_addresst(const std::string &address_string)
      : null_address(address_string == "0x0"), address_string(address_string)
    {
    }

    bool is_null() const
    {
      return null_address;
    }
    bool operator<(const memory_addresst &other) const
    {
      return address_string < other.address_string;
    }
    std::string string() const
    {
      return address_string;
    }
  };

  /// Create a \ref gdb_apit object
  /// \param binary: the binary to run with gdb
  /// \param log: boolean indicating whether gdb input and output should be
  ///   logged
  explicit gdb_apit(const char *binary, const bool log = false);

  /// Terminate the gdb process and close open streams (for reading from and
  /// writing to gdb)
  ~gdb_apit();

  struct pointer_valuet
  {
    pointer_valuet(
      const std::string &address = "",
      const std::string &pointee = "",
      const std::string &character = "",
      const optionalt<std::string> &string = nullopt,
      const bool valid = false)
      : address(address),
        pointee(pointee),
        character(character),
        string(string),
        valid(valid)
    {
    }

    const memory_addresst address;
    const std::string pointee;
    const std::string character;
    const optionalt<std::string> string;

    bool has_known_offset() const
    {
      return std::any_of(
        pointee.begin(), pointee.end(), [](char c) { return c == '+'; });
    }

    bool valid;
  };

  /// Get the allocated size estimate for a pointer by evaluating
  /// `malloc_usable_size'. The function returns the number of usable bytes in
  /// the block pointed to by the pointer to a block of memory allocated by
  /// `malloc' or a related function. The value may be greater than the
  /// requested size of the allocation because of alignment and minimum size
  /// constraints.
  /// \param pointer_expr: expression with a pointer name
  /// \return 1 if the pointer was not allocated with malloc otherwise return
  ///   the result of calling `malloc_usable_size'
  size_t query_malloc_size(const std::string &pointer_expr);

  /// Create a new gdb process for analysing the binary indicated by the member
  /// variable `binary`
  void create_gdb_process();

  /// Run gdb to the given breakpoint
  /// \param breakpoint the breakpoint to set (can be e.g. a line number or a
  ///   function name)
  /// \return true if something failed
  bool run_gdb_to_breakpoint(const std::string &breakpoint);

  /// Run gdb with the given core file
  /// \param corefile: core dump
  void run_gdb_from_core(const std::string &corefile);

  /// Get the memory address pointed to by the given pointer expression
  /// \param expr: an expression of pointer type (e.g., `&x` with `x` being of
  ///   type `int` or `p` with `p` being of type `int *`)
  /// \return memory address in hex format
  optionalt<std::string> get_value(const std::string &expr);

  pointer_valuet get_memory(const std::string &expr);

  /// Return the vector of commands that have been written to gdb so far
  /// \return the list of commands
  const commandst &get_command_log();

protected:
  const char *binary;

  FILE *response_stream;
  FILE *command_stream;

  const bool log;
  commandst command_log;

  enum class gdb_statet
  {
    NOT_CREATED,
    CREATED,
    STOPPED // valid state, reached e.g. after breakpoint was hit
  };

  gdb_statet gdb_state;

  typedef std::map<std::string, std::string> gdb_output_recordt;
  static gdb_output_recordt parse_gdb_output_record(const std::string &s);

  void write_to_gdb(const std::string &command);

  std::string read_next_line();
  std::string read_most_recent_line();

  std::string eval_expr(const std::string &expr);

  gdb_output_recordt
  get_most_recent_record(const std::string &tag, const bool must_exist = false);

  bool most_recent_line_has_tag(const std::string &tag);
  bool was_command_accepted();
  void check_command_accepted();

  static std::string r_opt(const std::string &regex);

  static std::string
  r_or(const std::string &regex_left, const std::string &regex_right);

  // regex group for hex memory address (part of the output of gdb when printing
  // a pointer), matches e.g. 0x601040 and extracts 0x601040
  const std::string r_hex_addr = R"((0x(?:0|[1-9a-f][0-9a-f]*)))";

  // regex group for identifier (optional part of the output of gdb when
  // printing a pointer), matches e.g. <abc> and extracts abc
  const std::string r_id = R"(<([^<>]+)>)";

  // regex group for octal encoded char (optional part of the output of gdb when
  // printing a pointer), matches e.g. \"\\003\" and extracts \\003
  const std::string r_char = R"(\\"(\\\\[0-7]{3})\\")";

  // regex group for string (optional part of the output of gdb when printing a
  // pointer), matches e.g. \"abc\" and extracts \"abc\"
  const std::string r_string = R"((\\".*\\"))";
};

class gdb_interaction_exceptiont : public cprover_exception_baset
{
public:
  explicit gdb_interaction_exceptiont(std::string reason) : reason(reason)
  {
  }
  std::string what() const override
  {
    return reason;
  }

private:
  std::string reason;
};

#endif // CPROVER_MEMORY_ANALYZER_GDB_API_H
