#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "blur.h"
int main(void)
{

  printf("start computing time\n");
  startClock();

  //load
  IMG_COL* left = readImageColor("img/DSC_0094.JPG");
  IMG_COL* right = readImageColor("img/DSC_0095.JPG");
  IMG* psf = readImage("img/zhou005-110222.png");

  double param[2] = {1.1803, 4.4626};

  Mat epiMat = matrixAlloc(3, 3);
  ELEM0(epiMat, 0, 0) = 0.0;
  ELEM0(epiMat, 0, 1) = 0.0;
  ELEM0(epiMat, 0, 2) = 0.0;
  ELEM0(epiMat, 1, 0) = 0.0;
  ELEM0(epiMat, 1, 1) = 0.0;
  ELEM0(epiMat, 1, 2) = 1.0;
  ELEM0(epiMat, 2, 0) = 0.0;
  ELEM0(epiMat, 2, 1) = -1.0;
  ELEM0(epiMat, 2, 2) = -1.0;

  //stereo
  IMG* disparityMap = stereoRecursive(left, right, &epiMat, 32, 2);

  printf("stereo correspondence done\n");

  saveImage(disparityMap, "img/disparityMap.png");

  return 0;

  IMG* deblurredImage = deblur( left->channel[0], 
				psf,
				disparityMap,
				param);

  saveImage( deblurredImage, "img/deblurredImage.png");

  printPassedTime();
  

  return 0;

}
