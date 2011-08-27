#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように

  IMG* img = readImage("img/MBP/110826-1/blurredwin.png");
  IMG* apertureIn = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* aperture = createImage( 17, 17);
  resizeImage(apertureIn, aperture);
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *psf = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);
  int h, w;
  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
      psf[idx][0] = 0.0;
      psf[idx][1] = 0.0;
    }
  }


  //copy psf
  double norm = imageNormL1(aperture);
  flipImage(aperture, 1, 1);
  for( h = 0; h < aperture->height; ++h){
    for( w = 0 ; w < aperture->width; ++w){
      int y = h - aperture->height/2;
      int x = w - aperture->width/2;
      y += (y<0) ? img->height : 0 ;
      x += (x<0) ? img->width  : 0 ;
      psf[ y * img->width+x ][0] = (double)IMG_ELEM( aperture, h, w) / norm;
    }
  }
  
  // makeplan and FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan pPSF = fftw_plan_dft_2d( img->height, img->width, psf, psf, FFTW_FORWARD, FFTW_ESTIMATE);

  fftw_execute(pSrc);
  fftw_execute(pPSF);

  //deblur
  double r = (double)aperture->height / 16.0 ;
  printf("r = %lf\n", r);
  for(h = 0; h < img->height; ++h){
    for(w = 0 ; w < img->width; ++w){
      int idx = h * img->width + w;
      double a = src[idx][0] ;
      double b = src[idx][1] ;
      double s = (double)h/r - (int)(h/r);
      double t = (double)w/r - (int)(w/r);
      int Wid = img->width;
      double c 
	= (1-s) * (1-t) * psf[(int)(h/r   ) * Wid + (int)(w/r   )][0]
	+ (1-s) * ( t ) * psf[(int)(h/r   ) * Wid + (int)(w/r +1)][0]
	+ ( s ) * (1-t) * psf[(int)(h/r +1) * Wid + (int)(w/r   )][0]
	+ ( s ) * ( t ) * psf[(int)(h/r +1) * Wid + (int)(w/r +1)][0];

      double d
	= (1-s) * (1-t) * psf[(int)(h/r   ) * Wid + (int)(w/r   )][1]
	+ (1-s) * ( t ) * psf[(int)(h/r   ) * Wid + (int)(w/r +1)][1]
	+ ( s ) * (1-t) * psf[(int)(h/r +1) * Wid + (int)(w/r   )][1]
	+ ( s ) * ( t ) * psf[(int)(h/r +1) * Wid + (int)(w/r +1)][1];
	
      c /= r*r;
      d /= r*r;
      double snr = 0.002;
      dbl[idx][0] = ( a*c + b*d ) / ( c*c + d*d + snr );
      dbl[idx][1] = ( b*c - a*d ) / ( c*c + d*d + snr );

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
      //printf("dbl = %lf + %lf  i \n", dbl[idx][0]/scale, dbl[idx][1]/scale);      
    }
  }

  saveImage( dst, "img/MBP/110826-1/deblurredResize.png");

  return 0;

}

