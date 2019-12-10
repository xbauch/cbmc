#include <assert.h>
#include <string.h>

int main()
{
  char lhs_input[] = "sh\0ort";
  char rhs_input[] = "slightly_longer_but_still_more_or_less_ok";

  int compare_same = strncmp(lhs_input, rhs_input, 1);
  assert(compare_same == 0);
  int compare_diff = strncmp(lhs_input, rhs_input, 10);
  assert(compare_diff != 0);

  return 0;
}
