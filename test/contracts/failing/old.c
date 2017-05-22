#include <stdio.h>
#include <stdlib.h>
#include <smack.h>
#include <smack-contracts.h>

// @expect verified

int g;

void p() {
  ensures(g > old(g));
  g++;
}

int main(void) {
  p();
  return 0;
}
