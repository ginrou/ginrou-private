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

IMG* deblurFFTW( IMG* img, IMG* psf);
IMG* deblurFFTWInvariant( IMG* src,
			  IMG* psfBase,
			  IMG* disparityMap,
			  double param[2]);

IMG* deblurFFTWResize( IMG* img, IMG* psf, double size);


#endif

  
