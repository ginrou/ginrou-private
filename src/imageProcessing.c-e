/****************************************
imageProcessing.c
****************************************/

#include "imageProcessing.h"

void resizeImage(const  IMG* src, IMG* dst)
{

  // (H,W) -> (Y,X)
  int H = src->height;
  int W = src->width;

  int Y = dst->height;
  int X = dst->width;

  /*
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
  */

  IplImage* in = cvCreateImage( cvSize( W, H), IPL_DEPTH_8U, 1);
  convertIMG2Ipl( src, in );

  IplImage* out = cvCreateImage( cvSize( X, Y), IPL_DEPTH_8U, 1);

  cvResize( in, out, CV_INTER_AREA);

  convertIpl2IMG( out, dst);

  cvReleaseImage( &in );
  cvReleaseImage( &out );


  return;

}

double imageNormL1(const IMG* img)
{
  double ret = 0.0;
  for(int h = 0; h < img->height; ++h){
    for( int w = 0; w < img->width; ++w){
      ret += IMG_ELEM( img, h, w);
    }
  }
  return ret;
}

void normalizeMat(Mat src, Mat dst)
{
  //compute sum
  double sum = 0.0;
  for(int row = 0; row < src.row; ++row){
    for(int col = 0; col < src.clm; ++col){
      sum += ELEM0(src, row, col);
    }
  }

  //devide by sum
  for(int row = 0; row < dst.row; ++row){
    for(int col = 0; col < dst.clm ; ++col){
      ELEM0(dst, row, col) = ELEM0(src, row, col) / sum;
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

// 平均mean, 分散varのホワイトノイズを付加
void putnoise(const IMG* src, IMG* dst, double mean, double var)
{
  srand(time(NULL));
  
  for(int h = 0; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width ; ++w){

      double tmp = 0.0;
      for(int i = 0; i < 6; ++i){
	tmp += rand()/(double)RAND_MAX;
      }
      tmp = var*(tmp - 3.0 ) + mean;

      double val = (double)IMG_ELEM( src, h, w);
      if( val + tmp >= 255 )
	IMG_ELEM( dst, h, w) =  255;
      else
	IMG_ELEM( dst, h, w) =  val + tmp;

    }
  }

}

void flipImage( IMG* img, int horizontal, int vertcial)
{
  IplImage* ipl = cvCreateImage( cvSize(img->height, img->width), IPL_DEPTH_8U, 1);
  convertIMG2Ipl(img, ipl);

  int flip = 0;
  if( horizontal && vertcial ) flip = -1;
  else if( vertcial ) flip = 1;
  else flip = 0;
  
  cvFlip(ipl, ipl, flip);
  
  convertIpl2IMG( ipl, img);
  cvReleaseImage(&ipl);
  return ;
}
