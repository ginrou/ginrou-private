#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "blur.h"

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES


int main(void)
{
  IMG* img = readImage( "img/MBP/screan.png" );
  Mat psf[MAX_DISPARITY];

  for( int d = 0; d < MAX_DISPARITY; ++d){
    psf[d] = matrixAlloc( 1, MAX_DISPARITY);
    
    for(int h = 0; h < psf[d].row; ++h){
      for(int w = 0; w < psf[d].clm; ++w){
	ELEM0( psf[d], h, w) = 0.0;
      }
    }
    ELEM0( psf[d], 0, psf[d].clm/2 - d/2) = 1.0;

  }

  IMG* map = readImage("img/MBP/disparityMap.png");

  convertScaleImage(map, map, 1.0/4.0, 0.0);
  
  IMG* mapShift = createImage( map->width, map->height);
  convertScaleImage(mapShift, mapShift, 0.0, MAX_DISPARITY);
  for(int h = 0; h < map->height;++h){
    for(int w = 0 ; w < map->width; ++w){
      int disp = IMG_ELEM( map, h, w) ;
      IMG_ELEM(mapShift, h, w + disp/2) = disp;
    }
  }

  IMG* dst =  blurWithPSFMap( img, psf, map);
  

  saveImage( dst, "img/MBP/test.png");

  return 0;

}
