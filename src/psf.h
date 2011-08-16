#ifndef __PSF__
#define __PSF__

#include "include.h"

void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam);

void makeBlurPSF( IMG* psf[MAX_DISPARITY], 
		  IMG* aperture,
		  int maxDepth,
		  double param[2]);

void makeShiftBlurPSF( Mat psf[MAX_DISPARITY], int cam,
		       IMG* aperture, double par[2]);

#endif 
