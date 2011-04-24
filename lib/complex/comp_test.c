#include <stdio.h>
#include <math.h>
#include <complex.h>

main () {
  Complex a, b, c, d;

  a.Re = 10;
  a.Im = 20;

  b.Re = 20;
  b.Im = 30;

  compDisp(a);
  printf("\n");
  compDisp(b);
  printf("\n");

  compDisp(compAdd(a, b));
  printf("\n");
  compDisp(compSub(a, b));
  printf("\n");
  compDisp(compMul(a, b));
  printf("\n");
  compDisp(compDiv(a, b));
  printf("\n");
  compDisp(compMul(compDiv(a,b), b));
  printf("\n");

  printf("%lf\n", compAbs(a));
}

