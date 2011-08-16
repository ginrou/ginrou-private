#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように

  char filename[256];
  IMG* left[4];
  IMG* right[4];
  IMG* dst[4];

  for(int i = 0; i < 4; ++i){
    sprintf( filename, "img/MBP/110816-1/blurredLeft%d.png", i+1);
    left[i] = readImage( filename );
    sprintf( filename, "img/MBP/110816-1/blurredRight%d.png", i+1);
    right[i] = readImage( filename );
  }

  IMG* ca1  = readImage("img/MBP/aperture/CAPair1.png");
  IMG* ca2  = readImage("img/MBP/aperture/CAPair2.png");
  IMG* zhou = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* cir  = readImage("img/MBP/aperture/circle.png");

  double par[2][2];
  int blk = 4;

  // 自分のシステム
  par[0][0] = 1.209449;
  par[0][1] = -8.324842;
  par[1][0] = 1.209449;
  par[1][1] = -6.853018;
  printf("start current system\n");

  dst[0] = currentSystemDispmap( left[0], right[0],
				 zhou, zhou,
				 par, 30, blk);
  saveImage( dst[0], "img/MBP/110816-1/dispmap1.png");

  // Coded Aperture Pair
  printf("coded aperture pair\n");
  par[0][0] = 1.0;
  par[0][1] = 0.0;
  dst[1] = CodedAperturePairDispmap( left[1], right[1],
				     ca1, ca2, par[0], 
				     30, blk);
  saveImage( dst[1], "img/MBP/110816-1/dispmap2.png");

  // ステレオ法
  IMG_COL *leftCol = readImageColor("img/MBP/110816-1/blurredLeft3.png");
  IMG_COL *rightCol = readImageColor("img/MBP/110816-1/blurredRight3.png");
  Mat fund = createHorizontalFundMat();
  dst[2] = stereoRecursive( leftCol, rightCol, &fund, 30, 0);
  saveImage( dst[2], "img/MBP/110816-1/dispmap3.png");

  // Depth from defocus
  par[0][0] = 1.777778;
  par[0][1] = -18.673244;
  par[1][0] = 1.777778;
  par[1][1] = -15.371831;
  dst[3] = DepthFromDeocus( left[3], right[3], cir, par, blk);
  saveImage( dst[3], "img/MBP/110816-1/dispmap4.png");

}

