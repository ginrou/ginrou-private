#include <stdio.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur2.h"
#include "blur.h"

#define YES 1
#define NO 0


int main( int argc, char* argv[]){
  IMG* src = readImage("img/blurred.png");
  IMG* psfTmp = readImage("img/Zhou0002.png");
  IMG* psf = createImage( 16, 16);
  resizeImage(psfTmp, psf);

  double p[2] = {0.0, 16.0};
  IMG* disparityMap = createImage(src->height, src->width);
  convertScaleImage( disparityMap, disparityMap, 0.0, 0.0);


  IMG* dbl = deblurFFTWInvariant(src, psfTmp, disparityMap, p);
  //IMG* dbl = deblurFFTW(src, psf);

  saveImage( dbl, "img/deblurredFFTW.png");

  return 0;

}
