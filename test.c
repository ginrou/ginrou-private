#include <stdio.h>
#include <string.h>

#include "include.h"

int numOfNonZero( Mat mat ){
  int count = 0;
  for( int r = 0; r < mat.row; ++r){
    for( int c = 0; c < mat.clm; ++c){
      if( ELEM0(mat, r, c) > 0.00001 ) count++;
    }
  }
  return count;
}

Mat PSFCutoffZeroRegion( Mat src){
  if( src.row == 0 || src.clm == 0) return src;
  int horizontalEdge = 0;
  int vertcialEdge = 0;
  int h, w;

  for( h = 0; h < src.row; ++h){
    for( w = 0 ;w < src.clm; ++w){
      if( ELEM0( src, h, w) > DBL_MIN ){
	if( horizontalEdge < w ) horizontalEdge = w;
	if( vertcialEdge < h )	vertcialEdge = h;
      }
    }
  }

  if( vertcialEdge == 0 ) vertcialEdge = 1;
  if( horizontalEdge == 0 ) horizontalEdge = 1;

  Mat dst = matrixAlloc( vertcialEdge, horizontalEdge );
  for( h = 0; h < dst.row; ++h){
    for( w = 0; w < dst.clm; ++w){
      ELEM0( dst, h, w) = ELEM0( src, h, w);
    }
  }

  PSFNormalize(dst);
  return dst;
}


int main( int argc, char* argv[]){


  IMG* lenna = readImage("img/test/LENNA.bmp");
  IMG* aperture = readImage("img/test/circle.png");
  if(!lenna || !aperture) return 1;

  // test of psf
  Mat psfLeft[MAX_DISPARITY];
  double paramLeft[2] = { 1.5, -10.0 };
  makeBlurPSFMat( aperture, paramLeft, psfLeft, Point( lenna->height, lenna->width), MAX_DISPARITY);

  Mat psfRight[MAX_DISPARITY];
  double paramRight[2] = { -0.75, 20.0};
  makeBlurPSFMat( aperture, paramRight, psfRight, Point( lenna->height, lenna->width), MAX_DISPARITY);



  for( int d = 0 ; d < MAX_DISPARITY; ++d){
    psfLeft[d] = PSFCutoffZeroRegion( psfLeft[d] );
    psfRight[d] = PSFCutoffZeroRegion( psfRight[d] );
  }

  Mat  blurred;
  for( int d = 0 ; d < MAX_DISPARITY; ++d){

    blurred = blurMat2Mat( lenna, psfLeft[d] );
    IMG *imgLeft = createImage( blurred.row, blurred.clm );
    convertMat2IMG( &blurred, imgLeft);

    blurred = blurMat2Mat( lenna, psfRight[d] );
    IMG* imgRight = createImage( blurred.row, blurred.clm );
    convertMat2IMG( &blurred, imgRight );
    
    IMG* depthmap =  latentBaseEstimationIMG( imgLeft, imgRight, psfLeft, psfRight );
    convertScaleImage( depthmap, depthmap,  4.0, 0.0 );

    char filename[256];
    sprintf( filename, "test/depth%02d.png", d);
    saveImage( depthmap, filename );

    /*
    sprintf( filename, "test/blurredLeft%02d.png", d);
    saveImage( imgLeft, filename );
    sprintf( filename, "test/blurredRight%02d.png", d);
    saveImage( imgRight, filename );
    */
    

  }

  return 0;


}
