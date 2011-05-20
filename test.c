#include <stdio.h>
#include <string.h>

#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur2.h"
#include "blur.h"

#define YES 1
#define NO 0


int main( int argc, char* argv[]){


  IMG* src = readImage("img/DSC_0095.JPG");
  IMG* psf = readImage("img/zhou005-110222.png");
  IMG* dispMap = createImage( src->height, src->width);
  convertScaleImage(dispMap, dispMap, 0.0, 0.0);

  char filename[256];

  int size;

  for( size = 1; size < 32; ++size ){
    printf("size = %d\n",size);
    double param[2] = { 0.0, 0.0 };
    param[1] = (double)size;
    IMG* dbl = deblurFFTWInvariant(src, psf, dispMap, param);
    sprintf(filename, "img/test/size%02d.png", size);
    saveImage(dbl, filename);
    releaseImage(&dbl);
  }

  return 0;

}
