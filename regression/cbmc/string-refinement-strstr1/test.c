#include <assert.h>
#include <string.h>

int main()
{
  //                  01234
  char str_input[] = "testing_string";
  char subsrc_input[] = "ing_s";

  char *result = strstr(str_input, subsrc_input);
  assert(result == (char *)str_input + 4);
  assert(*result == 'i');

  return 0;
}
