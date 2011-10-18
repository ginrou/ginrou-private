#include <stdio.h>
#include "include.h"

#include <cv.h>

int main(int argc, char* argv[])
{

  printf("%d\n", fabs(10)&&1);
  return 0;

  setbuf( stdout, NULL); // 改行をまたないように

  IMG* input1 = readImage( argv[1] );
  IMG* input2 = readImage( argv[2] );

  if( input1 == NULL || input2 == NULL ) return 0;

  point stPoint = Point( 150, 50 );
  point edPoint = Point( 350, 450 );

  IMG* difMap = createImage( input1->height, input1->width );
  convertScaleImage( difMap, difMap, 0.0, 0.0);


  FILE *fp = fopen( argv[4], "w" );

  int error = 0;
  int count = 0;


  for( int h = stPoint.y ; h < edPoint.y; ++h ){
    for( int w = stPoint.x; w < edPoint.x; ++w ){

      int dif = IMG_ELEM( input1, h, w)/4 - IMG_ELEM( input2, h, w);
      IMG_ELEM( difMap, h, w) = dif*dif;

      fprintf( fp, "%d, %d\n", IMG_ELEM( input1, h, w)/4, IMG_ELEM( input2, h, w));

      count++;
      error += (int)(fabs(dif) && 1 );


    }
  }

  fclose(fp);
  saveImage( difMap, argv[3] );

  printf( "error / count = %d / %d = %lf %% \n", error, count, (double)error/(double)count );


  return 0;


}

