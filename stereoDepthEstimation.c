#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


int main( int argc, char* argv[]){

  if( argc == 1 ){
    printf("input arguments : \n");
    printf("arg[1] : input left image \n");
    printf("arg[2] : input right image \n");
    printf("arg[3] : debugging images\n");
    printf("arg[4] : disparity map\n");
    return 0;
  }

  printf("stereo matching depth estimation\n");
  IMG* tmp = readImage( argv[1] );
  IMG_COL* inputLeft = convertIMG2IMG_COL( tmp );
  tmp = readImage( argv[2] );
  IMG_COL* inputRight = convertIMG2IMG_COL( tmp );
  Mat fundMat = createHorizontalFundMat();

  // save images to debug directry (dirname : argv[argc-2] )
  if( isalpha( argv[argc-2][0] )){
    strcpy( tmpImagesDir, argv[argc-2] ); 
    saveDebugImages = YES;
  }else{
    saveDebugImages = NO;
  }


  IMG* dst = stereoRecursive( inputLeft, inputRight, &fundMat, MAX_DISPARITY, MIN_DISPARITY);
  saveImage( dst, argv[argc-1] );

  return 0;
}
