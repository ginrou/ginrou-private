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
  
  IMG* dispCir = readImage("img/dispCir.png");
  IMG* dispZhou = readImage("img/dispZhou.png");
  IMG* depthMap = readImage("img/depth.png");

  IMG* errCir = createImage( dispCir->height, dispCir->width );
  IMG* errZhou = createImage( dispCir->height, dispCir->width );
  
  //parameters
  double W = 512;
  double b = 0.070434;
  double fov = 2.0 * atan( tan(40.0*M_PI/180.0)/2.24905 );
  double tfov = tan( fov/2.0 );

  for( int h = 0; h < depthMap->height; ++h){
    for( int w = 0 ; w < depth->width; ++w ){
      double depth = IMG_ELEM( depthMap, h, w) / 32.0 ;
      double disparity = (W*b)/(2.0*tfov*depth);
      double err;

      if( depth >= 255/32 - 1 ) {
	IMG_ELEM( errCir, h, w ) = 255;
	IMG_ELEM( errZhou, h, w ) = 255;
      }

      err = IMG_ELEM( dispCir, h, w ) - disparity ;
      IMG_ELEM( errCir, h, w ) = err;

      err = IMG_ELEM( dispZhou, h, w ) - disparity ;
      IMG_ELEM( errZhou, h, w ) = err;

    }
  }
  
  saveImage( errCir, "errCir.png" );
  saveImage( errZhou, "errZhou.png" );
  

  return 0;

}
