#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  int h, w;

  IMG* img      = readImage("img/MBP/110827-1/blurredLeft4.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* dispMap  = readImage("img/MBP/110827-1/disparityMap4.png");
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);
  double param[2] = { 2.653007, -20.628481};

  showDispMap(dispMap);
  return 0;
  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
    }
  }

  //copy psf
  int kernels = 45;
  fftw_complex *psf[45];
  flipImage( aperture, 1, 1);
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
  double depth = 110.0 / 32.0 ;
  for(int i = 0; i < 100; ++i ){
    depth = ( 80.0 + 1.0 * (double)i ) / 32.0;
    double disp = 512.0*0.025349 / ( 2.0 * depth * tan( 35.952969*M_PI/360.0 ));
    double size = fabs( disp * param[0] + param[1] );
    size = (double)i / 3.0;
    double r = size - (int)size;
    if( size < 1.0 ) {size = 1.0; r = 0.0 ;}
    printf("depth = %lf dispartiy = %lf, size = %lf  r = %lf\n",depth, disp, size, r);

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
    fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute( pDbl );
    
    IMG* dst = createImage( img->height, img->width );
    double scale = img->height * img->width;
    for( h = 0; h < dst->height; ++h){
      for( w = 0; w < dst->width; ++w){
	int idx = h * img->width + w;
	double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
	IMG_ELEM( dst, h, w) = sqrt(val) / scale;
      }
    }
    char filename[256];
    sprintf( filename, "img/MBP/110827-1/test/%02d.png", i);
    saveImage( dst, filename);
  }
  return 0;

}

