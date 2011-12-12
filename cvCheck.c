#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char* argv[] ){

  if ( argc < 4 ){
    printf("input arguments\n");
    printf("arg[1] : input file name");
    printf("arg[2] : width to change \n");
    printf("arg[3] : height to change \n");
    printf("arg[4] : output file name\n");
  }

  IplImage *input = cvLoadImage( argv[1], CV_LOAD_IMAGE_COLOR );
  int width = atoi( argv[2] );
  int height = atoi( argv[3] );
  IplImage *output = cvCreateImage( cvSize( width, height), IPL_DEPTH_8U, 3);
  cvResize( input, output, CV_INTER_LINEAR );

  cvSaveImage( argv[4], output, NULL );



  return 0;

}
