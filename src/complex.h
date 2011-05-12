#ifndef __COMPLEX__
#define __COMPLEX__

typedef struct {
  double Re;
  double Im;
} Complex;

Complex compAdd(Complex a, Complex b);
Complex compSub(Complex a, Complex b);
Complex compMul(Complex a, Complex b);
Complex compDiv(Complex a, Complex b);
double compAbs(Complex a);
double compAbs2(Complex a);
void compDisp(Complex a);

#endif
