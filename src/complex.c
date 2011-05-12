#include <stdio.h>
#include <math.h>
#include <complex.h>

Complex compAdd(Complex a, Complex b) {
  Complex r;

  r.Re = a.Re + b.Re;
  r.Im = a.Im + b.Im;

  return r;
}

Complex compSub(Complex a, Complex b) {
  Complex r;

  r.Re = a.Re - b.Re;
  r.Im = a.Im - b.Im;

  return r;
}

Complex compMul(Complex a, Complex b) {
  Complex r;

  r.Re = a.Re * b.Re - a.Im * b.Im;
  r.Im = a.Im * b.Re + a.Re * b.Im;

  return r;
}

Complex compDiv(Complex a, Complex b) {
  double x;
  Complex r;

  x = b.Re * b.Re + b.Im * b.Im;
  r.Re = (a.Re * b.Re + a.Im * b.Im) / x;
  r.Im = (a.Im * b.Re - a.Re * b.Im) / x;

  return r;
}

double compAbs(Complex a) {
  return sqrt(a.Re * a.Re + a.Im * a.Im);
}

double compAbs2(Complex a) {
  return a.Re * a.Re + a.Im * a.Im;
}

void compDisp(Complex a) {
  if(a.Im >= 0) {
    printf("%lf+%lfi", a.Re, a.Im);
  }
  else {
    printf("%lf%lfi", a.Re, a.Im);
  }
}
