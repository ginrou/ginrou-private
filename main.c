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
  
  char filename[256];
  IMG_COL* blurredCirLeft = readImageColor("img/blurredCirLeft.png");
  IMG_COL* blurredCirRight = readImageColor("img/blurredCirRight.png");
  IMG_COL* blurredZhouLeft = readImageColor("img/blurredZhouLeft.png");
  IMG_COL* blurredZhouRight = readImageColor("img/blurredZhouRight.png");

  FILE* fp = fopen( "errorData.txt", "w");
  fprintf(fp, "##var,  cirError, zhouError\n");
  for(int i = 0; i < 20; ++i){
    double var = (double)i / 2.0;
    for(int c = 0; c < 3;++c){
      putnoise( blurredCirLeft->channel[c], blurredCirLeft->channel[c], 0.0, var);
      putnoise( blurredCirRight->channel[c], blurredCirRight->channel[c], 0.0, var);
      putnoise( blurredZhouLeft->channel[c], blurredZhouLeft->channel[c], 0.0, var);
      putnoise( blurredZhouRight->channel[c], blurredZhouRight->channel[c], 0.0, var);
    }

    sprintf(filename, "img/noisedimage%02d.png", i);
    saveImageColor( blurredCirLeft, filename);

    Mat FundMat = createHorizontalFundMat();

    IMG* dst = stereoRecursive( blurredCirLeft, blurredCirRight, &FundMat, 32, 1);
    sprintf(filename, "img/dispCir%02d.png", i);
    saveImage( dst, filename );

    dst = stereoRecursive( blurredZhouLeft, blurredZhouRight, &FundMat, 32, 1);
    sprintf(filename, "img/dispZhou%02d.png", i);
    saveImage( dst, filename );
  

    sprintf(filename, "img/dispCir%02d.png", i);
    IMG* dispCir = readImage(filename);
    sprintf(filename, "img/dispZhou%02d.png", i);
    IMG* dispZhou = readImage(filename);
    IMG* dispMap = readImage("img/disparityMap.png");

    IMG* errCir = createImage( dispCir->height, dispCir->width );
    IMG* errZhou = createImage( dispCir->height, dispCir->width );
  
    double zhouTotal = 0.0;
    double cirTotal = 0.0;
    int count = 0;

    for( int h = 0; h < dispMap->height; ++h){
      for( int w = 0 ; w < dispMap->width; ++w ){
	double disparity = IMG_ELEM( dispMap, h, w) / 4.0; 
	double err;
	if( disparity < 0.5 ) {
	  IMG_ELEM( errCir, h, w ) = 255;
	  IMG_ELEM( errZhou, h, w ) = 255;
	  continue;
	}

	count++;

	err = IMG_ELEM( dispCir, h, w ) - disparity ;
	IMG_ELEM( errCir, h, w ) = fabs(err);
	cirTotal += (int)fabs(err);

	err = IMG_ELEM( dispZhou, h, w ) - disparity ;
	IMG_ELEM( errZhou, h, w ) = fabs(err);
	zhouTotal += (int)fabs(err);

      }
    }
  
    cirTotal /= (double)count;
    zhouTotal /= (double)count;
    printf("total error Zhou : %lf, Cir %lf\n", zhouTotal, cirTotal);
    fprintf(fp, "%lf, %lf, %lf\n", var, cirTotal, zhouTotal);

    sprintf(filename, "img/errCir%02d.png", i);
    saveImage( errCir, filename );
    sprintf(filename, "img/errZhou%02d.png", i);
    saveImage( errCir, filename );

  }
  fclose(fp);
  return 0;

}
