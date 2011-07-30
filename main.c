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
  IMG* img = readImage( "img/LENNA.bmp" );
  IMG* tmp = readImage( "img/psf.png" );
  
  Mat psf[10];
  psf[0] = matrixAlloc( tmp->height, tmp->width);
  for(int h = 0; h < tmp->height; ++h){
    for( int w = 0; w < tmp->width; ++w){
      ELEM0( psf[0], h, w) = IMG_ELEM( tmp, h, w);
    }
  }

  normalizeMat( &(psf[0]), &(psf[0]) );
  
  IMG* map = createImage( img->height, img->width);
  convertScaleImage(map, map, 0.0, 0.0);

  IMG* dst =  blurWithPSFMap( img, psf, map);
  
  saveImage( dst, "img/test/test.png");
}
