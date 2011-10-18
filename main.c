#include <stdio.h>
#include "include.h"

#include <cv.h>

int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  int h, w;  

  IMG *inputLeft, *inputRight;
  IMG *apertureLeft, *apertureRight;
  double paramLeft[2], paramRight[2];
  fftw_complex *psfLeft[MAX_DISPARITY], *psfRight[MAX_DISPARITY];

  
  char buf[256];
  FILE* fp = fopen( argv[1], "w");
  
  if( fp == NULL ){
    printf("there is no file as %s\n", argv[1]);
    return 0;
  }

  // input images
  fgets( buf, 256, fp );
  inputLeft = readImage(buf);
  fgets( buf, 256, fp );
  inputRight = readImage(buf);

  // apetures
  fgets( buf, 256, fp);
  apertureLeft = readImage(buf);
  fgets( buf, 256, fp);
  apertureRight = readImage(buf);

  // parameters
  fgets( buf, 256, fp);
  fscanf( fp, "%lf, %lf", &paramLeft[0], &paramLeft[1]);
  fgets( buf, 256, fp);
  fscanf( fp, "%lf, %lf", &paramRight[0], &paramRight[1]);

  // convert to psf
  fgets( buf, 256, fp);
  if( strcmp( buf, "1" ) == 0 ){
    printf("disparity == YES\n");
    makeShiftBlurPSFFreq( inputLeft->height, inputLeft->width, LEFT_CAM,
			  psfLeft, apertureLeft, paramLeft);
    makeShiftBlurPSFFreq( inputRight->height, inputRight->width, RIGHT_CAM,
			  psfRight, apertureRight, paramRight);
  }else{
    printf("disparity == NO \n");
    makeBlurPSFFreq( apertureLeft, paramLeft, psfLeft,
		     Point( inputLeft->width, inputLeft->height), MAX_DISPARITY);
    makeBlurPSFFreq( apertureRight, paramRight, psfRight,
		     Point( inputRight->width, inputRight->height), MAX_DISPARITY);
  }


  // compute disparity map
  IMG* disparityMap;
  fgets( buf, 256, fp);
  if( strcmp( buf, "1" ) == 0 ){
    printf("frequency region depth estiamtion\n");
    disparityMap = latentBaseEstimationIMG( inputLeft, inputRight, psfLeft, psfRight);
  }else{
    printf("stereo base depth estimation\n");
    Mat fund = createHorizontalFundMat();
    IMG_COL *tmpLeft = convertIMG2IMG_COL( inputLeft );
    IMG_COL *tmpRight = convertIMG2IMG_COL( inputRight );
    disparityMap = stereoRecursive( tmpLeft, tmpRight, &fund, MAX_DISPARITY, 1);
  }
  
  fgets( buf, 256, fp);
  saveImage( disparityMap, buf );


  fclose(fp);

  return 0;


}

