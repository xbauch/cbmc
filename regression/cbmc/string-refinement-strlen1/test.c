#include <assert.h>
#include <string.h>

int main()
{
  char src1_input[] = "testing_string";
  char src2_input[] = "testi\0ng_string";

  int src1_length = strlen(src1_input);
  assert(src1_length == 10);
  int src2_length = strlen(src2_input);
  assert(src2_length == 5);

  return 0;
}
