#include "psf.h"

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

void makeBlurPSF( Mat src[MAX_DISPARITY], 
		  Mat dst[MAX_DISPARITY], 
		  IMG* aperture, double par[2]) // parはDisp To PSF Sizeparam
{
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    
    double size = (double)disp * par[0] + par[1];
    if(fabs(size) < 1.0 ) size = 1.0;
    dst[disp] = matrixAlloc( abs(size) , 2.0*abs(size) + src[disp].clm );
    
    IMG* psf = createImage( abs(size), abs(size) );
    resizeImage( aperture, psf );

    matrixZero(dst[disp]);

    printf("d = %d size = %lf\n", disp, size);

    for(int h = 0; h < src[disp].row; ++h){
      for(int w = 0; w < src[disp].clm; ++w){
	
	for(int y = 0 ; y < abs(size); ++y ){
	  for(int x = 0; x < abs(size); ++x ){
	    ELEM0( dst[disp] , h+y, w+x ) +=
	      ELEM0( src[disp], h, w ) * (double)IMG_ELEM( psf, y, x);
	  }
	}

      }
    }
    
    releaseImage(&psf);

  }
  return;
}

