#ifndef __IMAGE_PROCESSING__
#define __IMAGE_PROCESSING__

#include "hiuraLib.h"
#include "imageData.h"
#include "util.h"

void resizeImage( IMG* src, IMG* dst);
void normalizeMat(Mat *src, Mat *dst);// compute L1 norm (sum of all elements)
void convetScaleImage( const IMG* src,  IMG* dst, double scale, double shift);

#endif
