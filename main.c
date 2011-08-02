#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "blur.h"
#include "batch.h"
#include "psf.h"

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES

#define LEFT_CAM 0
#define CENTER_CAM 1
#define RIGHT_CAM 2

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
    convertMat2IMG( psf[disp], img);
    char filename[256];
    sprintf(filename, "img/MBP/psf%02d.png", disp);
    saveImage( img, filename);
    normalizeMat( psf[disp], psd[disp]);
  }

  return 0;

}
