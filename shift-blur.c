#include "include.h"

int main( int argc, char* argv[]){


  printf("shit-blur psf model depth estimation\n");

  /*----------------------------------------*/
  /*        read inputs arguments           */
  /*----------------------------------------*/
  IMG* inputLeft     = readImage( argv[1] );
  IMG* inputRight    = readImage( argv[2] );
  IMG* apertureLeft  = readImage( argv[3] );
  IMG* apertureRight = readImage( argv[4] );
  
  if( inputLeft && inputRight && apertureLeft && apertureRight == NULL){
    printf("input argumetns are wrong\n");
    printf("arg[0] : process name %s\n", argv[0]);
    printf("arg[1] : left input image : %s\n", argv[1]);
    printf("arg[2] : right input image : %s\n", argv[2]);
    printf("arg[3] : left psf image : %s\n", argv[3]);
    printf("arg[4] : right psf image : %s\n", argv[4]);
    return 0;
  }

  double paramLeft[2], paramRight[2];
  paramLeft[0] = atof( argv[5] );
  paramLeft[1] = atof( argv[6] );
  paramRight[0] = atof( argv[7] );
  paramRight[1] = atof( argv[8] );

  printf( "parameter left = %lf , %lf\n", paramLeft[0], paramLeft[1]);
  printf( "parameter right = %lf , %lf\n", paramRight[0], paramRight[1]);

  // save images to debug directry (dirname : argv[argc-2] )
  if( isalpha( argv[argc-2][0] )){
    strcpy( tmpImagesDir, argv[argc-2] ); 
    saveDebugImages = YES;
  }else{
    saveDebugImages = NO;
  }

  printf("save result at %s\n", argv[argc-1]);

  /*----------------------------------------*/
  /*           make psf in freq             */
  /*----------------------------------------*/
  freq *psfLeft[MAX_DISPARITY];
  freq *psfRight[MAX_DISPARITY];
  makeShiftBlurPSFFreq2x( inputLeft->height, inputLeft->width, LEFT_CAM,
			psfLeft, apertureLeft, paramLeft);
  makeShiftBlurPSFFreq2x( inputRight->height, inputRight->width, RIGHT_CAM,
			psfRight, apertureRight, paramRight);
  printf("psf create done\n");
  
  PSFSaveForDebug( psfLeft, inputLeft->height, inputLeft->width, 
		   MIN_DISPARITY, MAX_DISPARITY, "psfLeft");
  
  PSFSaveForDebug( psfRight, inputRight->height, inputRight->width, 
		   MIN_DISPARITY, MAX_DISPARITY, "psfRight");


  /*----------------------------------------*/
  /*           depth estimation             */
  /*----------------------------------------*/
  IMG* dst = latentBaseEstimationIMG( inputLeft, inputRight, psfLeft, psfRight);
  //IMG* dst = deblurBaseEstimationIMGFreq( inputLeft, inputRight, psfLeft, psfRight );
  saveImage( dst, argv[argc-1] );
    
  return 0;
}
