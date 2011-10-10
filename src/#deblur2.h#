#ifndef __DEBLUR2__
#define __DEBLUR2__

#include <stdio.h>
#include <math.h>

#include <matrix.h>
#include <imageData.h>
#include <stereo.h>
#include <fftw3.h>

#ifndef __DEBLUR__
#define CUT_OFF_SIZE 64
#define BLOCK_SIZE 8
#endif

#ifndef MAX_DISPARITY
#define MAX_DISPARITY 32
#endif

/*
  winner deconvolutionに基づきdeblurを行う関数
  畳み込みはpsfを反転させなければならないので、
  予めpsfを反転させておく必要がある
  psfの正規化は行わなくてもよい(uchar型の配列なので実質は無理)
 */
IMG* deblurFFTW( IMG* img, IMG* psf);

IMG* deblurFFTW2( fftw_complex* src, fftw_complex* psf, double snr, int height, int width);

IMG* deblurFFTWInvariant( IMG* src,
			  IMG* psfBase,
			  IMG* disparityMap,
			  double param[2]);

IMG* deblurFFTWResize( IMG* img, IMG* psf, double size);

Mat hummingWindow( int imgHeight, int imgWidth, int psfHeight, int psfWidth);


#endif

  
