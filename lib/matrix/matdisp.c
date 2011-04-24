#include <stdio.h>
#include "matrix.h"

int main(int argc, char *argv[]) {
  if(argc != 2) {
    printf("%s <matrix file name>\n", argv[0]);
  }
  else {
    matrixDisp(matrixAllocLoad(argv[1]));
  }
  return 0;
}
