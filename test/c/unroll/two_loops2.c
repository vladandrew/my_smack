#include "smack.h"

// @expect verified
// @flag --unroll=10

int main(void) {
  int c = 0;

  for (int a = 0; a < 10; a++)
    c++;

  for (int a = 0; a < 5; a++)
    c++;

  return c;
}
