/*******************************************************************\

Module:

Author: Daniel Poetzl

\*******************************************************************/

#include "string_utils.h"
#include "exception_utils.h"

#include <cassert>
#include <cctype>

void split_string(
  const std::string &s,
  char delim,
  std::string &left,
  std::string &right,
  bool strip)
{
  // delim can't be a space character if using strip
  PRECONDITION(!std::isspace(delim) || !strip);

  std::vector<std::string> result;

  split_string(s, delim, result, strip);
  if(result.size() != 2)
  {
    throw deserialization_exceptiont{"expected string `" + s +
                                     "' to contain two substrings "
                                     "delimited by " +
                                     delim + " but has " +
                                     std::to_string(result.size())};
  }

  left=result[0];
  right=result[1];
}

std::vector<std::string> split_string(
  const std::string &s,
  char delim,
  bool strip,
  bool remove_empty)
{
  std::vector<std::string> result;
  split_string(s, delim, result, strip, remove_empty);
  return result;
}

std::string trim_from_last_delimiter(
  const std::string &s,
  const char delim)
{
  std::string result="";
  const size_t index=s.find_last_of(delim);
  if(index!=std::string::npos)
    result=s.substr(0, index);
  return result;
}

std::string escape(const std::string &s)
{
  std::string result;

  for(std::size_t i=0; i<s.size(); i++)
  {
    if(s[i]=='\\' || s[i]=='"')
      result+='\\';

    result+=s[i];
  }

  return result;
}
