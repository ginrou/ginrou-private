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

  IplImage *dispMap;
/*   CvStereoBMState* state = cvCreateStereoBMState( CV_STEREO_BM_BASIC, 128 ); */
/*   dispMap = cvCreateImage( cvGetSize( imgLeft ), IPL_DEPTH_16S, 1); */
/*   cvFindStereoCorrespondenceBM( imgLeft, imgRight, dispMap, state ); */

  IplImage *dispLeft  = cvCreateImage( cvGetSize( imgLeft ), IPL_DEPTH_16S, 1 );
  IplImage *dispRight = cvCreateImage( cvGetSize( imgLeft ), IPL_DEPTH_16S, 1 );
  CvStereoGCState *gcState = cvCreateStereoGCState( maxDisparity, 16 );
  cvFindStereoCorrespondenceGC( imgLeft, imgRight, dispLeft, dispRight, gcState, 0);
  
  dispMap = dispLeft;

  IplImage *dst = cvCreateImage( cvGetSize( dispMap ), IPL_DEPTH_8U, 1);
  cvConvertScale( dispMap, dst, 1.0 / 16.0, 0.0 );
  cvSaveImage( argv[4], dst , NULL);

  

  return 0;

}
