#include <assert.h>
#include <string.h>

int main()
{
  char lhs_input[] = "testing_string";
  char rhs_input[] = "testing_char_array";

  int compare_same = strncmp(lhs_input, lhs_input, 10);
  assert(compare_same == 0);
  int compare_precedes = strncmp(lhs_input, rhs_input, 10);
  assert(compare_precedes == 1);
  int compare_succedes = strncmp(rhs_input, lhs_input, 10);
  assert(compare_succedes == -1);

  return 0;
}
