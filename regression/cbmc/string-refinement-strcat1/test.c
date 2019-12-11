#include <assert.h>
#include <string.h>

int main()
{
  //                     01234567890123456789012
  //                                   _extended
  char dest_input[40] = "testing_string";
  char src_input[] = "_extended";

  char *result = strcat(dest_input, src_input);
  assert(result == (char *)dest_input);
  assert(result[14] == '_');

  return 0;
}
