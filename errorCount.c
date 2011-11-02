#include <stdio.h>
#include "include.h"

#include <cv.h>

/*
  一つ目の引数の画像と二つ目の引数の画像の二乗誤差を取る
  結果は三つ目の引数のファイル名で出力される
  誤差を計測する範囲(ROI)はこの中で定義する。→別に引数としてもいいけど　
  平均エラーとその分散はプリントされる
 */

#define startHeight 50
#define endHeight 450
#define startWidth 150
#define endWidth 350



int main(int argc, char* argv[]){
  

  setbuf(stdout, NULL);
  IMG* input1 = readImage(argv[1]);
  _ClearLine();
  IMG* input2 = readImage(argv[2]);  
  _ClearLine();

  if( input1 == NULL || input2 == NULL ) return 1;
  
  IMG* errorMap = createImage( input1->height, input1->width);
  convertScaleImage( errorMap, errorMap, 0.0, 0.0 );
  
  int h, w;
  double scale = (endHeight-startHeight)*(endWidth-startWidth);
  // fill error map and calc mean
  double mean = 0.0;
  for( h = startHeight; h < endHeight; ++h){
    for( w = startWidth; w < endWidth; ++w){
      int val1 = IMG_ELEM( input1, h, w);
      int val2 = IMG_ELEM( input2, h, w);
      mean += SQUARE( val1 - val2 );
      IMG_ELEM( errorMap, h, w) = SQUARE( val1-val2 );
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

  return 0;

}
