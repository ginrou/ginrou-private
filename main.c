#include <stdio.h>
#include "include.h"

int main(int argc, char* argv[])
{

  IMG* left = readImage("img/MBP/110802-01/blurredLeft.png");
  IMG* right = readImage("img/MBP/110802-01/blurredRight.png");  
  IMG* cir = readImage("img/MBP/110802/circle.png");
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  Mat tmpPSF[MAX_DISPARITY];
  char filename[256];
  double par[2];


  par[0] = 1.500331;
  par[1] = -25.745794;
  makeShiftPSF(tmpPSF, LEFT_CAM);
  makeBlurPSF( tmpPSF, psfLeft, cir, par );

  par[0] = 3.579218;
  par[1] = -26.265516;
  makeShiftPSF(tmpPSF, RIGHT_CAM);
  makeBlurPSF( tmpPSF, psfRight, cir, par );  

  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    
    IMG* img = createImage( psfLeft[disp].row, psfLeft[disp].clm );
    convertMat2IMG( &(psfLeft[disp]), img);

    sprintf(filename, "img/MBP/110802-01/test/psfLeft%02d.png", disp);
    saveImage( img, filename);
    releaseImage(&img);
    
    img = createImage( psfRight[disp].row, psfRight[disp].clm );
    convertMat2IMG( &(psfRight[disp]), img);
    sprintf(filename, "img/MBP/110802-01/test/psfRight%02d.png", disp);
    saveImage( img, filename);
    releaseImage(&img);

    normalizeMat( psfLeft[disp], psfLeft[disp]);
    normalizeMat( psfRight[disp], psfRight[disp]);
  }
  printf("make psf done\n");

  
  IMG* leftConv[MAX_DISPARITY];
  IMG* rightConv[MAX_DISPARITY];
  IMG* map = createImage( left->height, left->width );

  for(int d = 0; d < MAX_DISPARITY; ++d ){
    convertScaleImage( map, map, 0.0, d );

    leftConv[d] = blurWithPSFMap( left, psfLeft, map );
    sprintf(filename, "img/MBP/110802-01/test/%02dbluLeft.png", d);
    saveImage( leftConv[d], filename );

    rightConv[d] = blurWithPSFMap( right, psfRight, map );
    sprintf(filename, "img/MBP/110802-01/test/%02dbluRight.png", d);
    saveImage( rightConv[d], filename );

  }

  IMG* dispMap = createImage( left->height, left->width );
  for( int h = 0; h < dispMap->height; ++h){
    for( int w = 0; w < dispMap->width; ++w){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < MAX_DISPARITY; ++d){
	int blk = 9;
	double err = 0.0;
	double hoge;

	// ブロックマッチング
	for(int y = 0; y < blk; ++y){
	  for(int x= 0 ; x < blk; ++x){
	    
	    if( h+y >= dispMap->height || w+x >= dispMap->width) 
	      continue;

	    hoge = IMG_ELEM( leftConv[d], h+y, w+x ) - IMG_ELEM( rightConv[d], h+y, w+x);
	    
	    err += hoge*hoge;

	  }
	}
	
	if( err < min ){
	  min = err;
	  disp = d;
	}
      }

      IMG_ELEM( dispMap, h, w) = disp * 4.0;
      

    }
  }
  
  saveImage( dispMap, "img/MBP/110802/dispMap.png" );


  return 0;

}
