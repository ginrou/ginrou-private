#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように

  IMG* left = readImage("img/MBP/110809/blurredLeft.png");
  IMG* right = readImage("img/MBP/110809/blurredRight.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  
  char filename[256];
  IMG* dst, *psf;

  for(int d = 1; d < MAX_PSF_SIZE; ++d){
    psf = createImage( d, d);
    resizeImage( aperture, psf);

    dst = deblurFFTW( left, psf);
    sprintf( filename, "img/MBP/110812/left%02d.png", d);
    saveImage( dst, filename);
    
    dst = deblurFFTW( right, psf);
    sprintf( filename, "img/MBP/110812/right%02d.png", d);
    saveImage( dst, filename);

  }
  

  return 0;

}
