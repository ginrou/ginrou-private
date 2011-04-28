#include <stdio.h>
#include "hiuraLib.h"
#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"

int main(void)
{

  printf("start computing time\n");
  startClock();

  IMG* img = readImage( "img/LENNA.bmp" );
  IMG* psf = readImage( "img/Zhou0002.png" );
  IMG* psfMin = createImage(16, 16);
  resizeImage(psf, psfMin);

  IMG* blurred = blur( img, psfMin);

  saveImage( blurred, "img/blurred.png" );

  resizeImage(psf, psfMin);
  IMG* deblurred = deblur(blurred, psfMin);

  saveImage( deblurred, "img/deblurred.png" );

  return 0;

}
