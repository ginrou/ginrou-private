#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{

  IplImage* disparityMap = cvLoadImage( "test/disparityMap.png", CV_LOAD_IMAGE_GRAYSCALE);
  IplImage* cir, *zhou;

  FILE* fp = fopen("test/error.txt", "w" );
  fprintf( fp, "#var\tcircle \tzhou\n" );

  for(int i = 0; i < 10; ++i){
    char filename[256];
    
    sprintf( filename, "test/dispCir%02d.png", i);
    cir = cvLoadImage( filename, CV_LOAD_IMAGE_GRAYSCALE );

    sprintf( filename, "test/dispZhou%02d.png", i);
    zhou = cvLoadImage( filename, CV_LOAD_IMAGE_GRAYSCALE );


    double errCir = 0.0;
    double errZhou = 0.0;
    int count = 0;
    CvRect rect = cvRect( 117, 206, 260, 117);
    for(int h = rect.y ; h < rect.y + rect.height; ++h){
      for( int w = rect.x ; w < rect.x + rect.width ; ++w){
	errCir += abs( CV_IMAGE_ELEM( cir, uchar, h, w) - CV_IMAGE_ELEM( disparityMap, uchar, h, w)/4.0 );
	errZhou += abs( CV_IMAGE_ELEM( zhou, uchar, h, w) - CV_IMAGE_ELEM( disparityMap, uchar, h, w)/4.0 );
	count++;
      }
    }

    errCir /= (double)count;
    errZhou /= (double)count;

    printf("var = %lf, cir = %lf, zhou = %lf\n", (double)i/2.0, errCir, errZhou);
    fprintf( fp, "%lf\t%lf\t%lf\n", (double)i/2.0, errCir, errZhou);

  }

  fclose(fp);
  
  return 0;

}
