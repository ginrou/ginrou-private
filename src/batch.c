#include <batch.h>

int batch110801( int argc, char* argv[] )
{
  // stereo + deblurのシステムの実装

  
  // 実験材料とパラメータ
  IMG* srcLeft  = readImage("img/MBP/exp/blurredLeft.png");
  IMG* srcRight = readImage("img/MBP/exp/blurredRight.png");
  IMG* dispMap  = readImage("img/MBP/exp/disparityMap.png");
  IMG* psfBase  = readPixel("img/MBP/exp/Zhou0002.png");
  double DTPparam[2] = {1.599641, -27.508864 };  

  convertScaleImage( dispMap, dispMap, 1.0/4.0, 0.0);
  
  IMG_COL* imgCol = createImageColor( srcLeft->height, srcLeft->width);
  for(int c = 0; c < 3; ++C){
    imgCol->channel[c] = deblurFFTWInvariant( srcLeft, psfBase, dispMap, DTPparam);
  }

  saveImageColor( imgCol, "img/MBP/exp/deblur.png" );

  return 0;
}
