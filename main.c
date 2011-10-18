#include <stdio.h>
#include "include.h"

#include <cv.h>

int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように
  int h, w;  
  IMG* aperture = readImage("img/MBP/111014/circle.png");
  fftw_complex *psfLeft[MAX_DISPARITY], *psfRight[MAX_DISPARITY];

  /*
  IMG* leftInput = readImage("img/MBP/110828-3/blurredLeft.png");
  IMG* rightInput = readImage("img/MBP/110828-3/blurredLeft.png");
  double paramLeft[] = {0.833333, -13.086012};
  double paramRight[] = {0.833333, -10.772417};
  makeShiftBlurPSFFreq( leftInput->height, leftInput->height, LEFT_CAM,
			psfLeft, aperture, paramLeft);

  makeShiftBlurPSFFreq( rightInput->height, rightInput->height, RIGHT_CAM,
			psfRight, aperture, paramRight);
  */


  IMG* leftInput = readImage("img/MBP/111014/blurredLeft.png");
  IMG* rightInput = readImage("img/MBP/111014/blurredLeft.png");
  double paramLeft[] = {1.209449, -9.404082  };
  double paramRight[] = {1.209449, -6.313580  };
  point imgSize = Point( leftInput->width, leftInput->height);
  makeBlurPSFFreq( aperture, paramLeft, psfLeft, imgSize, MAX_DISPARITY);
  makeBlurPSFFreq( aperture, paramLeft, psfRight, imgSize, MAX_DISPARITY);





  IMG* depthMap = latentBaseEstimationIMG( leftInput, rightInput, psfLeft, psfRight );
  
  saveImage(depthMap, "img/MBP/111014/depthmap.png");


  return 0;

}

