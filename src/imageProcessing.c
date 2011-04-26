/****************************************
imageProcessing.c
****************************************/

#include "imageProcessing.h"

void resizeImage( IMG* src, IMG* dst)
{


  // (H,W) -> (Y,X)
  int H = src->height;
  int W = src->width;

  int Y = dst->height;
  int X = dst->width;

  for( int y = 0; y < Y ; ++y ) {
      for( int x = 0; x < X; ++x) {
	
	double intensity = 0.0;
	double weight = 0.0;

	for( int h = y*H/Y ; h < (y+1)*H/Y+1 ; ++h) {
	  for( int w = x*W/X ; w < (x+1)*W/X+1 ; ++w) {
	    intensity += (double)IMG_ELEM(src, h, w);
	    weight += 1.0;
	  }
	}
	
	IMG_ELEM(dst, y, x) = intensity / weight;

      }
  }

  return;

}


void normalizeMat(Mat *src, Mat *dst)
{
  //compute sum
  double sum = 0.0;
  for(int row = 0; row < src->row; ++row){
    for(int col = 0; col < src->clm; ++col){
      sum += ELEM0(*src, row, col);
    }
  }

  //devide by sum
  for(int row = 0; row < dst->row; ++row){
    for(int col = 0; col < dst->clm ; ++col){
      ELEM0(*dst, row, col) = ELEM0(*src, row, col) / sum;
    }
  }

  return;

}

void convertScaleImage( const IMG* src,  IMG* dst, double scale, double shift)
{
  for(int h=0 ; h < dst->height; ++h)
    {
      for( int w=0 ; w < dst->width; ++w)
	{
	  IMG_ELEM(dst, h, w) = scale * IMG_ELEM(src, h, w) + shift;
	}
    }

  return;

}
