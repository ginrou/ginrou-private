#include <stdio.h>
#include "include.h"

#include <cv.h>

/*
  args
  argv[1] : ground truth image
  argv[2] : analyze objective
  argv[3] : output file name (png image )
  argv[4] : start height to analyze
  argv[5] : start width to analyze
  argv[6] : end height point to analyze
  argv[7] : end width point to analyze
 */

#define START_HEIGHT 50
#define END_HEIGHT 450
#define START_WIDTH 150
#define END_WIDTH 350

int main(int argc, char* argv[]){
  

  setbuf(stdout, NULL);
  IMG* input1 = readImage(argv[1]);
  _ClearLine();
  IMG* input2 = readImage(argv[2]);  
  _ClearLine();

  if( input1 == NULL || input2 == NULL ) return 1;
  
  IMG* errorMap = createImage( input1->height, input1->width);
  convertScaleImage( errorMap, errorMap, 0.0, 0.0 );

  int startHeight = START_HEIGHT;
  int startWidth  = START_WIDTH;
  int endHeight   = END_HEIGHT;
  int endWidth    = END_WIDTH;

  if( argc >= 4 ){
    startHeight = atoi( argv[4] );
    startWidth  = atoi( argv[5] );
    endHeight   = atoi( argv[6] );
    endWidth    = atoi( argv[7] );
  }
  
  int h, w;
  double scale = (endHeight-startHeight)*(endWidth-startWidth);
  int count = 0;
  // fill error map and calc mean
  double mean = 0.0;
  for( h = startHeight; h < endHeight; ++h){
    for( w = startWidth; w < endWidth; ++w){
      int val1 = IMG_ELEM( input1, h, w);
      int val2 = IMG_ELEM( input2, h, w);
      mean += SQUARE( val1 - val2 );
      IMG_ELEM( errorMap, h, w) = SQUARE( val1-val2 );
      if(fabs(val1-val2) >= 1) count++;
    }
  }
  mean /= scale;
  
  // calc variance
  double var = 0.0;
  for( h = startHeight; h < endHeight; ++h){
    for( w = startWidth; w < endWidth; ++w){
      int val1 = IMG_ELEM( input1, h, w);
      int val2 = IMG_ELEM( input2, h, w);
      var += SQUARE( mean - SQUARE(val1-val2) );
    }
  }
  var /= scale;

  saveImage( errorMap, argv[3] );
  printf("mean : %lf\n", mean);
  printf("variance : %lf\n", var);
  printf("count = %d, error rate = %lf\n", count, (double)count/scale);


  return 0;

}
