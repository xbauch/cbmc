#include <assert.h>
#include <string.h>

int main()
{
  char test[] = "abcdefghijklmnopqrstuvwyz";

  for(char c = 'A'; c <= 'Z'; c++)
  {
    char *first_o = strchr(test, c);
    assert(*first_o == test[0]);
    assert(*first_o == test[1]);
    assert(*first_o == test[2]);
    assert(*first_o == test[3]);
    assert(*first_o == test[4]);
    assert(*first_o == test[5]);
    assert(*first_o == test[6]);
    assert(*first_o == test[7]);
    assert(*first_o == test[8]);
    assert(*first_o == test[9]);
    assert(*first_o == test[10]);
    assert(*first_o == test[11]);
    assert(*first_o == test[12]);
    assert(*first_o == test[13]);
    assert(*first_o == test[14]);
    assert(*first_o == test[15]);
    assert(*first_o == test[16]);
    assert(*first_o == test[17]);
    assert(*first_o == test[18]);
    assert(*first_o == test[19]);
  }
  return 0;
}
