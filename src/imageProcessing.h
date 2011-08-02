#ifndef __IMAGE_PROCESSING__
#define __IMAGE_PROCESSING__

#include <matrix.h>
#include "imageData.h"
#include "util.h"

void resizeImage(const  IMG* src, IMG* dst);
void normalizeMat(Mat src, Mat dst);// compute L1 norm (sum of all elements)
void convertScaleImage( const IMG* src,  IMG* dst, double scale, double shift);
void putnoise(const IMG* src, IMG* dst, double mean, double var); // 平均mean, 分散varのホワイトノイズを付加

#endif
