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
  IMG* img = readImage( "img/MBP/center.png" );
  IMG* gt  = readImage( "img/MBP/left.png" );
  Mat psf[MAX_DISPARITY];

  for( int d = 0; d < MAX_DISPARITY; ++d){
    psf[d] = matrixAlloc( 1, MAX_DISPARITY);
    
    for(int h = 0; h < psf[d].row; ++h){
      for(int w = 0; w < psf[d].clm; ++w){
	ELEM0( psf[d], h, w) = 0.0;
      }
    }
    ELEM0( psf[d], 0, psf[d].clm/2 + d/2) = 1.0;

    IMG* tmp = createImage( psf[d].row, psf[d].clm);
    for(int h = 0; h < psf[d].row; ++h){
      for(int w = 0; w < psf[d].clm; ++w){
	IMG_ELEM(tmp, h, w) = ELEM0( psf[d], h, w);
      }
    }
    char filename[256];
    sprintf(filename, "img/MBP/psf%02d.png", d);
    //    saveImage( tmp, filename);
    
  }

  IMG* map = readImage("img/MBP/disparityMapLeft.png");

  convertScaleImage(map, map, 1.0/4.0, 0.0);
  
  IMG* dst =  blurWithPSFMap( img, psf, map);
  /*  
  convertScaleImage( dst, dst, 0.0, 0.0);
  for( int h = 0 ; h < dst->height; ++h){
    for(int w = 0; w < dst->width; ++w){
      int disp = IMG_ELEM( map, h, w);
      IMG_ELEM( dst, h, w) = IMG_ELEM( img, h, w+disp/2);
    }
  }
  */
  IMG* dif = createImage( dst->height, dst->width );
  for( int h = 0 ; h < dst->height; ++h){
    for(int w = 0; w < dst->width; ++w){
      IMG_ELEM( dif, h, w) = 15.0*abs(IMG_ELEM( dst, h, w) - IMG_ELEM(gt, h, w));
    }
  }
  

  saveImage( dst, "img/MBP/test.png");
  saveImage( dif, "img/MBP/dif.png");
  return 0;

}
