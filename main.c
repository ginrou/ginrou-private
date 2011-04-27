#include <stdio.h>
#include "hiuraLib.h"
#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"

int main(void)
{

  printf("start computing time\n");
  startClock();


  IMG_COL* leftImage = readImageColor("img/meter-right-cut.png");
  IMG_COL* rightImage = readImageColor("img/meter-left-cut.png");

  showImage( leftImage->channel[0], 500);
  showImage( rightImage->channel[0], 500);

  Mat fundMat = matrixAlloc(3, 3);
  ELEM0( fundMat, 0, 0) = 0.0;
  ELEM0( fundMat, 0, 1) = 0.0;
  ELEM0( fundMat, 0, 2) = 0.0;
  ELEM0( fundMat, 1, 0) = 0.0;
  ELEM0( fundMat, 1, 1) = 0.0;
  ELEM0( fundMat, 1, 2) =  1.0;
  ELEM0( fundMat, 2, 0) = 0.0;
  ELEM0( fundMat, 2, 1) = -1.0;
  ELEM0( fundMat, 2, 2) = 0.01;

  //put some comment here!
  //It has changed!!
  //From Mac Book Pro!!!
  //Can I fixed it?



  printPassedTime();

  IMG* disparityMap = stereoInitialDisparityMap( leftImage, rightImage, &fundMat, 32);
  
  saveImage(disparityMap, "img/dispmap.png");

  printPassedTime();

  //  showImage( disparityMap, 0);
  

  return 0;

}
