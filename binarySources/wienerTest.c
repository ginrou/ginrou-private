#include <stdio.h>
#include "include.h"

#include <cv.h>

int main(int argc, char* argv[])
{
  printf("wiener test\n");
  
  setbuf( stdout, NULL); // 改行をまたないように
  
  if( argc <= 4 ){
    printf("input arguments are\n");
    printf("arg[0] : process name %s\n", argv[0]);
    printf("arg[1] : input left  \n");
    printf("arg[2] : input right \n");
    printf("arg[3] : aperture left  \n");
    printf("arg[4] : left param  \n");
    printf("left psf size = arg[5] * disparity + arg[6]\n");
    printf("right psf size = arg[7] * disparity + arg[8]\n");
    printf("arg[9] : tmp directry to save debugging images ( optional )\n");
    return 0;
  }

  /*----------------------------------------*/
  /*         read input arguments           */
  /*----------------------------------------*/
  IMG* inputLeft = readImage( argv[1] );
  IMG* inputRight = readImage( argv[2] );
  IMG* apertureLeft = readImage( argv[3] );
  IMG* apertureRight = readImage( argv[4] );

  double paramLeft[2], paramRight[2];
  paramLeft[0] = atof( argv[5] );
  paramLeft[1] = atof( argv[6] );
  paramRight[0] = atof( argv[7] );
  paramRight[1] = atof( argv[8] );

  if( 1 ){
    //save debugging images
    strcpy( tmpImagesDir, argv[argc-1]);
    saveDebugImages = YES;
    printf("save images to %s\n", tmpImagesDir);
  }else{
    saveDebugImages = NO;
  }


  
  /*----------------------------------------*/
  /*        make psf in freq domain         */
  /*----------------------------------------*/
  freq *psfLeft[MAX_DISPARITY];
  freq *psfRight[MAX_DISPARITY];
  makeShiftBlurPSFFreq( inputLeft->height, inputLeft->width, LEFT_CAM,
			psfLeft, apertureLeft, paramLeft);
  makeShiftBlurPSFFreq( inputRight->height, inputRight->width, RIGHT_CAM,
			psfRight, apertureRight, paramRight);
  printf("psf create done\n");


  /*----------------------------------------*/
  /*          depth estimation              */
  /*----------------------------------------*/
  IMG* disparityMap;
  //disparityMap= latentBaseEstimationIMG( inputLeft, inputRight, psfLeft, psfRight);
  //disparityMap= deblurBaseEstimationIMGFreq( inputLeft, inputRight, psfLeft, psfRight);
  deblurBaseEstimationFreqDebugOnly( inputLeft, inputRight, psfLeft, psfRight );

  return 0;

}

