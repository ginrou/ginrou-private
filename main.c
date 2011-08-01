#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "blur.h"
#include "batch.h"

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES

#define LEFT_CAM 0
#define CENTER_CAM 1
#define RIGHT_CAM 2

void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam){
  
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    psf[disp] = matrixAlloc( 1, MAX_DISPARITY);

    for( int y = 0 ; y < psf[disp].row; ++y){
      for( int x = 0 ; x < psf[disp].clm; ++x){
	ELEM0( psf[disp] , y, x ) = 0.0;
      }
    }

    if( cam == LEFT_CAM ){
      ELEM0( psf[disp], 0, MAX_DISPARITY/2 - disp/2) = 1.0;
    }else if( cam == RIGHT_CAM ){
      ELEM0( psf[disp], 0, MAX_DISPARITY/2 + disp/2) = 1.0;
    }

  }
  return;
}


int main(int argc, char* argv[])
{
  IMG* left = readImage("img/MBP/left.png");
  IMG* right = readImage("img/MBP/right.png");
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  makeShiftPSF( psfLeft, LEFT_CAM);
  makeShiftPSF( psfRight, RIGHT_CAM);

  IMG* leftCom[MAX_DISPARITY];
  IMG* rightCom[MAX_DISPARITY];

  IMG* map = createImage( left->height, left->width );

  char filename[256];

  // それぞれのPSFで畳み込み
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    convertScaleImage( map, map, 0.0, disp );
    leftCom[disp] = blurWithPSFMap( left, psfLeft, map);
    rightCom[disp] = blurWithPSFMap( right, psfRight, map );

    sprintf( filename, "img/test/left-%02d.png", disp);
    saveImage( leftCom[disp], filename);
  }
  // 畳み込んだ結果は leftCom, rightComへ


  IMG* dispMap = createImage( left->height, left->width);


  // 誤差が最も少ない画素が視差となる
  for( int h = 0; h < dispMap->height; ++h ){
    for( int w = 0 ; w < dispMap->width; ++w){

      double min = DBL_MAX;
      int disp;

      for(int d = 0 ; d < MAX_DISPARITY; ++d){

	int blk = 9;
	double err = 0.0;
	double hoge;

	for(int y = 0; y < blk; ++y){
	  for(int x = 0; x < blk; ++x){
	    if(h+y >= dispMap->height || w+x >= dispMap->width) continue;
	    hoge = IMG_ELEM( leftCom[d], h+y, w+x) - IMG_ELEM( rightCom[d], h+y, w+x);
	    err += hoge * hoge;

	  }
	}
	if( err < min ){
	  min = err;
	  disp = d;
	}
      }

      IMG_ELEM( dispMap, h, w) = disp * 4.0;
      if( h % 10 == 0 && w % 10 == 0)
	printf("disprity at %d, %d = %d\n", h, w, disp);
    }
  }

  saveImage( dispMap, "img/MBP/disp.png");


  // エラー計測
  IMG* gt = readImage("img/MBP/disparityMapLeft.png");
  IMG* sum = createImage( gt->height, gt->width );
  for(int h = 0 ; h < gt->height; ++h){
    for( int w = 0 ; w < gt->width ; ++w){
      int e = IMG_ELEM( gt, h, w) - IMG_ELEM(dispMap, h, w);
      IMG_ELEM(sum, h, w) = abs(e);
    }
  }

  saveImage( sum, "img/MBP/error.png");

  return 0;

}
