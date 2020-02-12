#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  int size = 100;
  char *str = malloc(sizeof(char) * size);
  for(int i = 0; i < size - 1; i++)
  {
    char nondet_char;
    __CPROVER_assume(nondet_char != '\0');
    str[i] = nondet_char;
  }
  /* for(int i = 0; i < size-1; ++i) */
  /* { */
  /*   char nondet_char; */
  /*   str[i] = nondet_char; */
  /* } */
  int at;
  __CPROVER_assume(at >= 0 && at < size - 1);
  str[10] = 'z';
  str[size - 1] = '\0';

  char *result = strchr(str, 'z');
  assert(result);
  assert(*result == str[10]);
  assert(*result == str[9]);

  return 0;
}
