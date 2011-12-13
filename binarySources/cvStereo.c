#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char* argv[] ){

  if( argc < 5 ){
    printf("input arguments :\n");
    printf("arg[1] : input left image\n");
    printf("arg[2] : input right image\n");
    printf("arg[3] : max disparity\n");
    printf("arg[4] : result disparity map name\n");
  }

  IplImage *imgLeft  = cvLoadImage( argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  IplImage *imgRight = cvLoadImage( argv[2], CV_LOAD_IMAGE_GRAYSCALE);


  
  int maxDisparity = atoi( argv[3] );
  printf("max disparity = %d\n", maxDisparity);

  IplImage *dispMap;
  IplImage *dispLeft = cvCreateImage(cvGetSize( imgLeft ), IPL_DEPTH_16S, 1 );
  IplImage *dispRight = cvCreateImage(cvGetSize( imgLeft), IPL_DEPTH_16S, 1 );
  CvStereoGCState *gcState = cvCreateStereoGCState( maxDisparity, 4 );
  cvFindStereoCorrespondenceGC( imgLeft, imgRight, dispLeft, dispRight, gcState, 0);

  dispMap = dispLeft;
  IplImage *dst = cvCreateImage( cvGetSize( dispMap ), IPL_DEPTH_8U, 1);
  for( int h = 0; h < dispLeft->height; h += 4){
    for( int w = 0; w < dispLeft->width; w +=4 ){
      int hoge =(int)CV_IMAGE_ELEM( dispLeft, short, h, w);
      int piyo =(int)CV_IMAGE_ELEM( dispRight, short, h, w);
      printf("%03d, %03d, left : %d, right : %d\n", h, w, hoge, piyo);
    }
  }
  cvConvertScale( dispRight, dst, 1.0 / 1.0, 0.0 );
  cvSaveImage( argv[4], dst , NULL);

  

  return 0;

}
