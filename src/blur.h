#ifndef __BLUR__
#define __BLUR__

#include <stdio.h>
#include <math.h>

#include "fourier.h"

#include <imageData.h>
#include <imageProcessing.h>
#include <util.h>

IMG* blur(IMG *img, IMG* psf);
IMG* blurFilter( IMG *img, IMG *psf);
IMG* blurWithPSFMap( IMG* img, Mat psf[], IMG* psfMap);


void normalize( Complex arr[FFT_SIZE][FFT_SIZE] );


#endif
