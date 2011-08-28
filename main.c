#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  
  int h, w;
  IMG* imgLeft = readImage("img/MBP/110828-1/blurredLeft.png");
  IMG* imgRight = readImage("img/MBP/110828-1/blurredRight.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  size_t memSize = sizeof(fftw_complex) * imgLeft->height * imgLeft->width;
  fftw_complex *srcLeft  = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *srcRight = (fftw_complex*)fftw_malloc(memSize);

  double paramLeft[2]  = { 1.6381, -25.5643};
  double paramRight[2] = { -2.2457, 28.5836};
  
  // copy src
  for( h = 0; h < imgLeft->height; ++h){
    for( w = 0 ; w < imgLeft->width ; ++w){
      int idx = h * imgLeft->width + w;
      srcLeft[idx][0] = DBL_ELEM( imgLeft, h, w);
      srcLeft[idx][1] = 0.0;
      srcRight[idx][0] = DBL_ELEM( imgRight, h, w);
      srcRight[idx][1] = 0.0;
    }
  }

  //FFT
  fftw_plan planSrcLeft = fftw_plan_dft_2d( imgLeft->height, imgLeft->width, srcLeft, srcLeft, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan planSrcRight = fftw_plan_dft_2d( imgRight->height, imgRight->width, srcRight, srcRight, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( planSrcLeft );
  fftw_execute( planSrcRight );


  fftw_complex* psfLeft[MAX_DISPARITY];
  fftw_complex* psfRight[MAX_DISPARITY];
  fftw_complex* fftw_tmp1 = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex* fftw_tmp2 = (fftw_complex*)fftw_malloc(memSize);
  fftw_plan planTmp1 = fftw_plan_dft_2d( imgLeft->height, imgLeft->width, fftw_tmp1, fftw_tmp1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan planTmp2 = fftw_plan_dft_2d( imgLeft->height, imgLeft->width, fftw_tmp2, fftw_tmp2, FFTW_FORWARD, FFTW_ESTIMATE);

  makeShiftBlurPSFFreq( imgLeft->height, imgLeft->width, LEFT_CAM,
			psfLeft, aperture, paramLeft );

  makeShiftBlurPSFFreq( imgRight->height, imgRight->width, RIGHT_CAM, 
			psfRight, aperture, paramRight);


  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];

  // deblur
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){


    double snr = 0.002;
    dblLeft[disp] = deblurFFTW2( srcLeft, psfLeft[disp], snr, imgLeft->height, imgLeft->width);
    dblRight[disp] = deblurFFTW2( srcRight, psfRight[disp], snr, imgRight->height, imgRight->width);

    char filename[256];
    sprintf(filename, "img/MBP/110828-1/test/dbl%02dLeft.png", disp);
    saveImage( dblLeft[disp], filename );
    sprintf(filename, "img/MBP/110828-1/test/dbl%02dRihgt.png", disp);
    saveImage( dblRight[disp], filename );

  }

  IMG* dst = createImage( imgLeft->height, imgLeft->width );
  for( h = 0; h < dst->height; ++h){
    for( w= 0 ; w < dst->width ;++w ){

      int blk = 4;
      double min = DBL_MAX;
      int disp;

      for( int d = 0; d < MAX_DISPARITY; ++d){

	double sum = 0.0;
	for(int y = 0; y < blk; ++y){
	  for( int x = 0; x < blk; ++x){
	    sum += abs( IMG_ELEM( dblLeft[d], h+y, w+x) - IMG_ELEM( dblRight[d], h+y, w+x) );
	  }
	}

	if( sum < min ){
	  min = sum;
	  disp = d;
	}

      }//d
      
      IMG_ELEM( dst, h, w) = disp;;

    }
  }

  convertScaleImage( dst, dst, 4.0, 10.0 );

  saveImage( dst, "img/MBP/110828-1/dispmap.png");


  return 0;

}

