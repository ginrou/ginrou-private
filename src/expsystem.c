#include "expsystem.h"

// 現在の自分のシステムの奥行き推定システム
IMG* currentSystemDispmap
( IMG* srcLeft, 
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2][2],
  int maxDisparity,
  int blockSize
  )
{
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];

  makeShiftBlurPSF( psfLeft, LEFT_CAM, apertureLeft, param[0]);
  makeShiftBlurPSF( psfRight, RIGHT_CAM, apertureRight, param[1]);

  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];
  IMG* psfImg;

  // deblur part
  for( int d = 0; d < maxDisparity; ++d){
    psfImg = createImage( psfLeft[d].row, psfLeft[d].clm);
    convertMat2IMG( &(psfLeft[d]), psfImg);
    dblLeft[d] = deblurFFTW( srcLeft, psfImg);
    releaseImage(&psfImg);

    psfImg = createImage( psfRight[d].row, psfRight[d].clm);
    convertMat2IMG( &(psfRight[d]), psfImg);
    dblRight[d] = deblurFFTW( srcRight, psfImg);
    releaseImage(&psfImg);

    _ClearLine();
    printf("%02d / %02d done", d+1 , maxDisparity);
  }

  printf("\n");

  // depth estiamtion part
  IMG* dst = createImage( srcLeft->height, srcRight->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      
      int disp;
      double min = DBL_MAX;

      for( int d = 0; d <maxDisparity; ++d){
	double sum = 0.0;
	
	for( int y = 0; y < blockSize ; ++y){
	  for( int x = 0; x < blockSize; ++x){
	    
	    if( h+y < 0 || h+y >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;

	  }
	}

	if( sum < min ){
	  min = sum;
	  disp = d;
	}

      }// d
      IMG_ELEM( dst, h, w ) = disp;
    }// w
  }// h
  
  // クリーニング
  

  for( int d = 0; d < maxDisparity; ++d){
    matrixFree( psfLeft[d] );
    matrixFree( psfRight[d] );
    releaseImage( &(dblLeft[d]) );
    releaseImage( &(dblRight[d]) );
  }

  return dst;

}


IMG* CodedAperturePairDispmap
( IMG* srcLeft,
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2],
  int maxDepth,
  int blockSize
  )
{

  IMG* psfLeft[MAX_PSF_SIZE];
  IMG* psfRight[MAX_PSF_SIZE];
  makeBlurPSF( psfLeft, apertureLeft, MAX_PSF_SIZE, param);
  makeBlurPSF( psfRight, apertureRight, MAX_PSF_SIZE, param);

  IMG* dblLeft[MAX_PSF_SIZE];
  IMG* dblRight[MAX_PSF_SIZE];

  for( int d = 0; d < MAX_PSF_SIZE; ++d){
    flipImage( psfLeft[d], 1, 1);
    flipImage( psfRight[d], 1, 1);
    dblLeft[d] = deblurFFTW( srcLeft, psfLeft[d] );
    dblRight[d] = deblurFFTW( srcRight, psfRight[d] );
    char filename[256];
    sprintf(filename, "img/MBP/110815-1/test/dblLeft%02d.png", d);
    saveImage( dblLeft[d], filename);
    sprintf(filename, "img/MBP/110815-1/test/dblRight%02d.png", d);
    saveImage( dblRight[d], filename);
  }

  printf("deblurring done\n");

  IMG* dst = createImage( srcLeft->height, srcLeft->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      int disp;
      double min;
      for( int d = 0; d < MAX_PSF_SIZE; ++d){
	double sum = 0;
	
	for(int y = 0; y < blockSize; ++y){
	  for(int x = 0 ; x < blockSize; ++x){
	    if( y+h < 0 || y+h >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;
	  }//x
	}//y

	if( min > sum ){
	  sum = min;
	  disp = d;
	}

      }//d

      IMG_ELEM( dst, h, w) = disp;

    }//w
  }//x

  for(int i = 0 ; i < MAX_PSF_SIZE; ++i){
    releaseImage( &(psfLeft[i]) );
    releaseImage( &(psfRight[i]) );
    releaseImage( &(dblLeft[i]) );
    releaseImage( &(dblRight[i]) );
  }
  return dst;
}



// ピント位置を変更したDFD
IMG* DepthFromDeocus
( IMG* srcLeft,
  IMG* srcRight,
  IMG* aperture,
  double param[2][2],
  int blockSize
  )
{
  IMG* psfLeft[MAX_DISPARITY];
  IMG* psfRight[MAX_DISPARITY];
  makeBlurPSF( psfLeft, aperture, MAX_DISPARITY, param[0] );
  makeBlurPSF( psfRight, aperture, MAX_DISPARITY, param[0] );

  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];

  for( int d = 0 ; d < MAX_DISPARITY; ++d){
    dblLeft[d] = deblurFFTW( srcLeft, psfLeft[d] );
    dblRight[d] = deblurFFTW( srcRight, psfRight[d] );
  }

  IMG* dst = createImage( srcLeft->height, srcLeft->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      int disp;
      double min;
      for( int d = 0; d < MAX_DISPARITY; ++d){
	double sum = 0;
	
	for(int y = 0; y < blockSize; ++y){
	  for(int x = 0 ; x < blockSize; ++x){
	    if( y+h < 0 || y+h >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;
	  }//x
	}//y

	if( min > sum ){
	  sum = min;
	  disp = d;
	}

      }//d

      IMG_ELEM( dst, h, w) = disp;

    }//w
  }//x

  for(int i = 0 ; i < MAX_DISPARITY; ++i){
    releaseImage( &(psfLeft[i]) );
    releaseImage( &(psfRight[i]) );
    releaseImage( &(dblLeft[i]) );
    releaseImage( &(dblRight[i]) );
  }
  return dst;

}
