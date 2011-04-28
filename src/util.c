/****************************************
util.c
****************************************/
#include "util.h"

//途中の画像を表示
void showImage( const IMG *img, int keyWait)
{
  if( img == NULL ) return;
  
  IplImage* dispImage = cvCreateImage( cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
  convertIMG2Ipl( img, dispImage);
  
  cvNamedWindow("image", CV_WINDOW_AUTOSIZE);
  cvShowImage("image", dispImage);
  cvWaitKey( keyWait );
  cvDestroyWindow("image");
  cvReleaseImage(&dispImage);

  return;
}

//処理時間計測用
struct timeval tv;
double startTime = 0.0;

void startClock(void)
{
  gettimeofday(&tv, NULL);
  startTime = tv.tv_sec + (double)tv.tv_usec*1e-6;
}

double getPassedTime(void)
{
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6 - startTime;
}

void printPassedTime(void)
{
  double t = getPassedTime();

  int h = (int)t / 3600;
  t -= (double)h*3600.0;

  int m = (int)t/60;
  t -= (double)m * 60.0;

  printf("passed time = %d : %d : %lf \n", h, m, t);
}
