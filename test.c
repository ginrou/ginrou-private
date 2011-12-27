#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


int main( int argc, char* argv[]){

  printf("Deblurring method with Wiener deconvolution\n");

  if( argc == 1 ){
    printf("input arguments are\n");
    printf("arg[1] : left image\n");
    printf("arg[2] : right image\n");
    printf("arg[3] : left psf\n");
    printf("arg[4] : right psf\n");
    printf("psf size left  = arg[5] * disp + arg[6]\n");
    printf("psf size right = arg[7] * disp + arg[8]\n");
    printf("arg[9] : debug image dir\n");
    return 0;
  }else if( argc < 9 ){
    printf("input arguments are wrong\n");
    return 1;
  }

  IMG* left = readImage(argv[1]);
  IMG* right = readImage(argv[2]);
  IMG* apertureLeft = readImage(argv[3]);
  IMG* apertureRight = readImage(argv[4]);

  double paramLeft[2], paramRight[2];
  paramLeft[0]  = atof( argv[5] );
  paramLeft[1]  = atof( argv[6] );
  paramRight[0] = atof( argv[7] );
  paramRight[1] = atof( argv[8] );

  printf("param left  = %lf * disp + %lf\n", paramLeft[0], paramLeft[1]);
  printf("param right = %lf * disp + %lf\n", paramRight[0], paramRight[1]);

  // debug image dir
  strcpy( tmpImagesDir, argv[argc-2] ); 
  saveDebugImages = YES;

  freq* psfLeft[MAX_DISPARITY];
  freq* psfRight[MAX_DISPARITY];
  makeShiftBlurPSFFreq2x( left->height, left->width, LEFT_CAM,
			  psfLeft, apertureLeft, paramLeft);
  makeShiftBlurPSFFreq2x( right->height, right->width, RIGHT_CAM,
			  psfRight, apertureRight, paramRight);
			  
  deblurBaseEstimationFreqDebugOnly( left, right, psfLeft, psfRight );

  return 0;
}
