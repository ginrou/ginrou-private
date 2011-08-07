#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{

  return batch110801( argc, argv);

  IMG* center = readImage("img/MBP/110805/center.png");
  IMG* aperture = readImage("img/MBP/aperture/circle.png");
  IMG* disparityMap = readImage("img/MBP/110805/disparityMap.png");
  convertScaleImage( disparityMap, disparityMap, 1.0/4.0, 0.0);

  double par[] = { 3.907555, -43.410005};
  Mat psf[MAX_DISPARITY];
  makeShiftBlurPSF( psf, RIGHT_CAM, aperture, par);
  for(int d = 0; d < MAX_DISPARITY; ++d)
    normalizeMat( psf[d], psf[d] );

  IMG* blurred = blurWithPSFMap( center, psf, disparityMap);

  saveImage(blurred, "img/MBP/110805/blurredRight.png");

  return 0;
}
