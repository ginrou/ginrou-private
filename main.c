#include <stdio.h>
#include "include.h"

#include <cv.h>



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



int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  int h, w;  

  IMG* leftInput = readImage("img/MBP/111014/blurredLeft.png");
  IMG* rightInput = readImage("img/MBP/111014/blurredRight.png");
  IMG* aperture = readImage("img/MBP/111014/circle.png");

  if( !leftInput  || !rightInput || !aperture )return 1;

  double paramLeft[] = {1.209449, -9.404082};
  double paramRight[] = {1.209449, -6.313580};
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  makeBlurPSFMat( aperture, paramLeft, psfLeft, Point( leftInput->height, leftInput->width), MAX_DISPARITY);
  makeBlurPSFMat( aperture, paramRight, psfRight, Point( rightInput->height, rightInput->width), MAX_DISPARITY);



  for( int d = 0 ; d < MAX_DISPARITY; ++d){
    psfLeft[d] = PSFCutoffZeroRegion( psfLeft[d] );
    psfRight[d] = PSFCutoffZeroRegion( psfRight[d] );
  }

  IMG* depthMap = latentBaseEstimationIMG( leftInput, rightInput, psfLeft, psfRight );
  
  saveImage(depthMap, "img/MBP/111014/depthmap.png");


  return 0;

}

