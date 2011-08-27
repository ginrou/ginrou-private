#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  int h, w;

  IMG* img      = readImage("img/MBP/110827-1/blurredLeft5.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* dispMap  = readImage("img/MBP/110827-1/disparityMap5.png");
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);

  //showDispMap(dispMap);
  //return 0;

  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
    }
  }

  //copy psf
  int kernels = 50;
  fftw_complex *psf[50];
  flipImage( aperture, 0, 1); //0,1
  for( int i = 1; i < kernels; ++i ){

    IMG* apIn = createImage( i, i );
    resizeImage( aperture, apIn );

    psf[i] = (fftw_complex*)fftw_malloc(memSize);
    for(int n = 0; n < img->height * img->width; ++n){
      psf[i][n][0] = 0.0;
      psf[i][n][1] = 0.0;
    }
    
    double norm = imageNormL1(apIn);
    for( h = 0; h < i; ++h ){
      for( w = 0 ; w < i; ++w ){
	int y = h - i/2;
	int x = w - i/2;
	y += (y<0) ? img->height : 0;
	x += (x<0) ? img->width  : 0;
	int idx = y * img->width + x;
	psf[i][idx][0] = DBL_ELEM( apIn, h, w) / norm ;
      }
    }
    releaseImage( &apIn );
  }
	 


  // makeplan and FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pSrc);
  
  fftw_plan pPSF;
  for( int i = 1; i < kernels; ++i){
    pPSF = fftw_plan_dft_2d( img->height, img->width, psf[i], psf[i], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( pPSF );
    fftw_destroy_plan( pPSF );
  }


  //deblur
  IMG *dblImg[MAX_DISPARITY];
  fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);

  double param1[2] = { -1.2857, 13.3661};
  double param2[2] = { -1.6413, 17.838};
  double param51[2] = { -5.6075/4.0, 43.1935};
  double param52[2] = { 3.621871 / 4.0, -28.161897 };
  double paramCalib[2] = { 0.1, 3.0};
  double *param = param52;

  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    double size = fabs( (double)disp * param[0] + param[1] );
    double r = size - (int)size;

    if( size < 1.0 ) {size = 1.0; r = 0.0 ;}
    printf("dispartiy = %d, size = %lf  r = %lf\n", disp, size, r);

    if( size > 50 )continue;

    for(h = 0; h < img->height; ++h){
      for(w = 0 ; w < img->width; ++w){
	int idx = h * img->width + w;
	double a = src[idx][0] ;
	double b = src[idx][1] ;
	double c = (1-r)*psf[(int)size][idx][0] + r* psf[(int)size+1][idx][0];
	double d = (1-r)*psf[(int)size][idx][1] + r* psf[(int)size+1][idx][1];

	double snr = 0.002;
	dbl[idx][0] = ( a*c + b*d ) / ( c*c + d*d + snr);
	dbl[idx][1] = ( b*c - a*d ) / ( c*c + d*d + snr);
      }
    }

    //IDFT
    fftw_execute( pDbl );

    dblImg[disp] = createImage( img->height, img->width );
    double scale = img->height * img->width;
    for( h = 0; h < dblImg[disp]->height; ++h){
      for( w = 0; w < dblImg[disp]->width; ++w){
	int idx = h * img->width + w;
	double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
	IMG_ELEM( dblImg[disp], h, w) = sqrt(val) / scale;
      }
    }

    if( param == paramCalib){
      char filename[256];
      sprintf( filename, "img/MBP/110827-1/test/%02d.png", disp);
      saveImage( dblImg[disp], filename );
    }
  }

  IMG* dst = createImage( img->height, img->width );
  for( h = 0; h < dst->height; ++h){
    for( w = 0; w < dst->width; ++w){
      int disp = IMG_ELEM( dispMap, h, w);
      IMG_ELEM( dst, h, w) = IMG_ELEM( dblImg[disp], h, w);
    }
  }

  saveImage( dst, "img/MBP/110827-1/deblurredImage.png");

  return 0;

}

