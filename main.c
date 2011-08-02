#include <stdio.h>
#include "include.h"

int main(int argc, char* argv[])
{
  IMG* cir = readImage("img/MBP/circle.png");
  Mat psf[MAX_DISPARITY];
  Mat dst[MAX_DISPARITY];
  makeShiftPSF(psf, LEFT_CAM);
  double par[2] = {1.0, 1.0};
  
  makeBlurPSF( psf, dst, cir, par );

  printf("make psf done\n");

  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    IMG* img = createImage( dst[disp].row, dst[disp].clm );
    convertMat2IMG( &(psf[disp]), img);
    char filename[256];
    sprintf(filename, "img/MBP/psf%02d.png", disp);
    saveImage( img, filename);
    normalizeMat( psf[disp], psf[disp]);
  }

  return 0;

}
