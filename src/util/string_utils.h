/*******************************************************************\

Module:

Author: Daniel Poetzl

\*******************************************************************/


#ifndef CPROVER_UTIL_STRING_UTILS_H
#define CPROVER_UTIL_STRING_UTILS_H

#include <algorithm>
#include <iosfwd>
#include <string>
#include <vector>

#include "invariant.h"

/// Remove all whitespace characters from either end of a string. Whitespace
/// in the middle of the string is left unchanged
/// \param s: the string to strip
/// \return The stripped string
template <typename String>
String strip_string(const String &s)
{
  auto pred = [](char c) { return std::isspace(c); };

  auto left = std::find_if_not(s.begin(), s.end(), pred);
  if(left == s.end())
    return String{};

  auto reverse_right = std::find_if_not(s.rbegin(), s.rend(), pred);
  auto right = s.end();
  right -= std::distance(s.rbegin(), reverse_right);

  return String(left, right);
}

/// Given a string s, split into a sequence of substrings when separated by
/// specified delimiter.
/// \param s: The string to split up
/// \param delim: The character to use as the delimiter
/// \param [out] result: The sub strings. Must be empty.
/// \param strip: If true, strip_string will be used on each element, removing
///   whitespace from the beginning and end of each element
/// \param remove_empty: If true, all empty-string elements will be removed.
///   This is applied after strip so whitespace only elements will be removed if
///   both are set to true.
template <typename String>
void split_string(
  const String &s,
  char delim,
  std::vector<String> &result,
  bool strip = false,
  bool remove_empty = false)
{
  PRECONDITION(result.empty());
  // delim can't be a space character if using strip
  PRECONDITION(!std::isspace(delim) || !strip);

  if(s.empty())
  {
    result.push_back(String{});
    return;
  }

  auto from = s.begin();
  auto to = s.end();
  do
  {
    to = std::find(from, s.end(), delim);

    String new_s(from, to);
    if(strip)
      new_s = strip_string(new_s);
    if(!remove_empty || !new_s.empty())
      result.push_back(new_s);

    from = to + 1;
  } while(to != s.end());

  if(result.empty())
    result.push_back(String{});
}

void split_string(
  const std::string &s,
  char delim,
  std::string &left,
  std::string &right,
  bool strip=false);

std::vector<std::string> split_string(
  const std::string &s,
  char delim,
  bool strip = false,
  bool remove_empty = false);

std::string trim_from_last_delimiter(
  const std::string &s,
  const char delim);

/// Prints items to an stream, separated by a constant delimiter
/// \tparam It: An iterator type
/// \tparam Delimiter: A delimiter type which supports printing to ostreams
/// \param os: An ostream to write to
/// \param b: Iterator pointing to first item to print
/// \param e: Iterator pointing past last item to print
/// \param delimiter: Object to print between each item in the iterator range
/// \param transform_func: Transform to apply to the value returned by the
///   iterator
/// \return A reference to the ostream that was passed in
template <
  typename Stream,
  typename It,
  typename Delimiter,
  typename TransformFunc>
Stream &join_strings(
  Stream &&os,
  const It b,
  const It e,
  const Delimiter &delimiter,
  TransformFunc &&transform_func)
{
  if(b==e)
  {
    return os;
  }
  os << transform_func(*b);
  for(auto it=std::next(b); it!=e; ++it)
  {
    os << delimiter << transform_func(*it);
  }
  return os;
}

/// Prints items to an stream, separated by a constant delimiter
/// \tparam It: An iterator type
/// \tparam Delimiter: A delimiter type which supports printing to ostreams
/// \param os: An ostream to write to
/// \param b: Iterator pointing to first item to print
/// \param e: Iterator pointing past last item to print
/// \param delimiter: Object to print between each item in the iterator range
/// \return A reference to the ostream that was passed in
template <typename Stream, typename It, typename Delimiter>
Stream &
join_strings(Stream &&os, const It b, const It e, const Delimiter &delimiter)
{
  using value_type = decltype(*b);
  // Call auxiliary function with identity function
  return join_strings(
    os, b, e, delimiter, [](const value_type &x) { return x; });
}

/// Generic escaping of strings; this is not meant to be a particular
/// programming language.
std::string escape(const std::string &);

#endif
