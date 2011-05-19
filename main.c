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
  /*
  printf("start computing time\n");
  startClock();

  IMG* src = readImage("img/LENNA.bmp");
  IMG* hoge = readImage("img/Zhou0002.png");
  IMG* piyo = createImage( 16, 16);
  resizeImage( hoge, piyo);
  IMG* blu = blur(src, piyo);
  saveImage( blu, "img/blurred2.png");
  IMG* dbl = deblur2( blu, piyo, 16);
  saveImage( dbl, "img/deblurred3.png");


  return 0;
  */
  //load
  IMG_COL* left = readImageColor("img/DSC_0095.JPG");
  IMG_COL* right = readImageColor("img/DSC_0094.JPG");
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
  IMG* disparityMap;
  if( LOAD_DISPARITY_MAP == YES){
    disparityMap = readImage("img/disparityMap.png");
  }else{
    disparityMap = stereoRecursive(left, right, &epiMat, 32, 2);
    saveImage(disparityMap, "img/disparityMap.png");
  }

  printf("stereo correspondence done\n");

  int disp;
  param[0] = 1.0;
  param[1] = 0.0;
  for(disp = 1; disp < MAX_DISPARITY; ++disp){
    printf("disparity = %d\n",disp);
    convertScaleImage(disparityMap, disparityMap, 0.0, disp);
    IMG* dbl = deblur( left->channel[0],psf,disparityMap, param);
    char filename[256];
    sprintf( filename, "img/test/dblDisp%02d.png",disp);
    saveImage(dbl, filename);
    releaseImage(&dbl);
  }


  return 0;


  IMG* deblurredImage = deblur( left->channel[0], 
				psf,
				disparityMap,
				param);

  saveImage( deblurredImage, "img/deblurredImage.png");

  printPassedTime();
  

  return 0;

}
