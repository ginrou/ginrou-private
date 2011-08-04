#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  IMG* left = readImage("img/MBP/110803/blurredLeft.png");
  IMG* right = readImage("img/MBP/110803/blurredRight.png");
  IMG* apeture = readImage("img/MBP/110803/circle.png");
  
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  double par[2];

  IMG* leftDbl[MAX_DISPARITY];
  IMG* rightDbl[MAX_DISPARITY];

  // PSF を作る
  par[0] = 3.579218;
  par[1] = -26.265516;
  makeShiftBlurPSF( psfLeft, LEFT_CAM, apeture, par);

  par[0] = 1.500331;
  par[1] = -25.745794;
  makeShiftBlurPSF( psfRight, RIGHT_CAM, apeture, par);


  // deblurを行う
  IMG* psf;
  for( int d = 0; d < MAX_DISPARITY; ++d){

    char filename[256];

    psf = createImage( psfLeft[d].row, psfLeft[d].clm );
    convertMat2IMG( &(psfLeft[d]), psf );
    leftDbl[d] = deblurFFTW( left, psf );
    sprintf( filename, "img/MBP/110803/test/dbl%02dLeft.png", d);
    saveImage( leftDbl[d], filename);

    psf = createImage( psfRight[d].row, psfRight[d].clm );
    convertMat2IMG( &( psfRight[d] ), psf );
    rightDbl[d] = deblurFFTW( right, psf );
    sprintf( filename, "img/MBP/110803/test/dbl%02dRight.png", d);
    saveImage( rightDbl[d], filename);
    
  }


  // deblur結果から視差を計算
  IMG* dispMap = createImage( left->height, left->width);
  for( int h = 0 ;h < dispMap->height; ++h){
    for( int w = 0; w < dispMap->width; ++w){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < MAX_DISPARITY; ++d){
	int blk = 4;
	double err = 0.0;
	
	for(int y = 0; y < blk; ++y){
	  for(int x = 0;  x < blk; ++x ){
	    if( h+y >= dispMap->height || w+x >= dispMap->width)
	      continue;

	    double val =
	      IMG_ELEM( leftDbl[d], h + y, w + x )
	      - IMG_ELEM( rightDbl[d], h+y, w+x );
	      
	    err += val * val;

	  }
	}

	if( err < min ){
	  min = err;
	  disp = d;
	}

      }//d 

      IMG_ELEM( dispMap, h, w) = disp * 4.0;

    }//w 
  }//h

  saveImage( dispMap, "img/MBP/110803/disparityMap.png");

  return 0;
}
