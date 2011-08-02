#ifndef __PSF__
#define __PSF__

#include "include.h"

void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam);
void makeBlurPSF( Mat src[MAX_DISPARITY], 
		  Mat dst[MAX_DISPARITY], 
		  IMG* aperture, double par[2]); // parはDisp To PSF Sizeparam


#endif 
