#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"

IMG* blurWithPSFMap2( IMG* img, IMG* psf[MAX_DISPARITY], IMG *dispMap);

int main( int argc, char* argv[]){

  int h, w, d;

  // read image
  IMG* center = readImage("test/center.png");
  IMG* dispMap = readImage("test/disparityMap.png");
  IMG* aperture = readImage("test/PSF2m.png");

  if( center && dispMap && aperture == NULL ){
    printf("cannot open file\n");
    return 0;
  }

  // params
  double focalDisaprity[2] = { 61, 75 };
  double Diameter = 0.75;
  double paramLeft[2],  paramRight[2];
  paramLeft[0] = Diameter;
  paramLeft[1] = -focalDisaprity[0] * Diameter;

  paramRight[0] = -Diameter;
  paramRight[1] = focalDisaprity[1] * Diameter;
  


  // create images
  IMG* imgLeft = createImage( center->width, center->height);
  IMG* imgRight = createImage( center->width, center->height);

  // shift image
  convertScaleImage( imgLeft, imgLeft, 0.0, 0.0 );
  convertScaleImage( imgRight, imgRight, 0.0, 0.0 );
  for( h = 0; h < center->height; ++h){
    for( w = 0; w < center->width ;++w){
      d = IMG_ELEM( dispMap, h, w);
      if( w-d/2 < 0 || w + d/2 >= imgRight->width || d < 0) continue;
      IMG_ELEM( imgLeft, h, w ) = IMG_ELEM( center, h, w  + d/2);
      IMG_ELEM( imgRight, h, w ) = IMG_ELEM( center, h, w-d/2 );
    }
  }
  saveImage( imgLeft, "test/shiftLeft.png");
  saveImage( imgRight, "test/shiftRight.png");


  // blur image
  // create PSF
  IMG *psfLeft[MAX_DISPARITY], *psfRight[MAX_DISPARITY];
  makeBlurPSF( psfLeft, aperture, MAX_DISPARITY, paramLeft );
  makeBlurPSF( psfRight, aperture, MAX_DISPARITY, paramRight );

  printf("make psf done\n");
  IMG *bluLeft = blurWithPSFMap2( imgLeft, psfLeft, dispMap );
  IMG *bluRight = blurWithPSFMap2( imgRight, psfRight, dispMap );
  
  saveImage(bluLeft, "test/bluredLeft.png");
  saveImage(bluRight, "test/bluredRight.png");

  // deblur image
  
  // set debugging directry
  strcpy( tmpImagesDir, "test/imgs");
  saveDebugImages = YES;
  printf("save images to %s\n", tmpImagesDir);

  // calcuate disparity
  freq *psfLeftFreq[MAX_DISPARITY], *psfRightFreq[MAX_DISPARITY];
  
  flipImage( aperture, 1, 1);
  makeShiftBlurPSFFreq( imgLeft->height, imgLeft->width, LEFT_CAM,
			psfLeftFreq, aperture, paramLeft);
  flipImage( aperture, 1, 1);
  makeShiftBlurPSFFreq( imgRight->height, imgRight->width, RIGHT_CAM,
			psfRightFreq, aperture, paramRight);
  IMG*disparityMapEstimated;
  //disparityMapEstimated = latentBaseEstimationIMG( bluLeft, bluRight, psfLeftFreq, psfRightFreq);
  disparityMapEstimated = deblurBaseEstimationIMGFreq( bluLeft, bluRight, psfLeftFreq, psfRightFreq);

  saveImage( disparityMapEstimated, "test/disparityMapEstimated.png");

  IMG* deblurred = deblurFromTwoImages( bluLeft, bluRight, 
					psfLeftFreq, psfRightFreq,
					disparityMapEstimated);
  saveImage( deblurred, "test/deblurred.png");

  return 0;
}



IMG* blurWithPSFMap2( IMG* img, IMG* psf[MAX_DISPARITY], IMG *dispMap)
{
  IMG* dst = createImage( img->width, img->height );

  double norm[MAX_DISPARITY];
  for( int d = 0; d < MAX_DISPARITY; ++d){
    norm[d] = 0.0;
    for( int h = 0; h < psf[d]->height; ++h){
      for( int w = 0; w < psf[d]->width; ++w){
	norm[d] += IMG_ELEM( psf[d], h, w );
      }
    }
  }

  convertScaleImage( dst, dst, 0.0, 0.0 );
  for( int h = 0; h < dst->height; ++h){
    for( int w = 0; w < dst->width; ++w){
      int d = IMG_ELEM( dispMap, h, w);
      if( d < MIN_DISPARITY || d >= MAX_DISPARITY ) continue;
      double sum = 0.0;
      for( int y = 0; y < psf[d]->height; ++y){
	for( int x = 0; x < psf[d]->width; ++x){
	  int py = h + y - psf[d]->height/2;
	  int px = w + x - psf[d]->width/2;
	  if( py < 0 || py >= img->height ||
	      px < 0 || px >= img->width) continue;
	  sum += IMG_ELEM( img, py, px) * IMG_ELEM( psf[d], y, x);
	}
      }
      IMG_ELEM( dst, h, w ) = sum / norm[d];
      //printf("h = %d, w = %d, d = %d,  %lf\n", h, w, d, sum/norm[d]);
    }
  }

  return dst;

}
