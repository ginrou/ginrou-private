#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "blur.h"

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES

int main(void)
{
  
  IMG_COL *leftCir = readImageColor( "img/blurredCircleLeft.png" );
  IMG_COL *rightCir = readImageColor( "img/blurredCircleRight.png" );
  IMG_COL *leftZhou = readImageColor( "img/blurredZhouLeft.png" );
  IMG_COL *rightZhou = readImageColor( "img/blurredZhouRight.png" );

  Mat fundMat = createHorizontalFundMat();

  for(int r = 0; r < 3; ++r){
    for(int c = 0; c < 3; ++c){
      printf("%lf   ", ELEM0( fundMat, r, c));
    }
    printf("\n");
  }

  if( leftCir == NULL ||
      rightCir == NULL ||
      leftZhou == NULL ||
      rightZhou == NULL ) 
    printf("null!\n");

  printf("stereo circle start\n");

  IMG* dispMap = stereoRecursive( leftCir, rightCir, &fundMat, 32, 1);
  saveImage( dispMap, "img/dispCir.png" );

  printf("stereo Zhou start\n");
  dispMap = stereoRecursive( leftZhou, rightZhou, &fundMat, 32, 1);
  saveImage( dispMap, "img/dispZhou.png" );

  return 0;

}
