#include <batch.h>

int batch110801( int argc, char* argv[] )
{
  // stereo + deblurのシステムの実装

  
  // 実験材料とパラメータ
  IMG_COL* srcLeft  = readImageColor("img/MPro/debug/DSC_0094.JPG");
  IMG_COL* srcRight  = readImageColor("img/MPro/debug/DSC_0095.JPG");
  IMG* psfBase  = readImage("img/MPro/debug/zhou005-110222.png");

  Mat fund = createHorizontalFundMat();

  IMG* dispMap = stereoRecursive( srcLeft, srcRight, &fund, 20, 0);

  double par[] = {1.528633, -26.839933 };  

  char filename[256];

  par[0] = 1.0;par[1] = 0.0;
  for(int disp = 0; disp <MAX_DISPARITY; ++disp){
    convertScaleImage( dispMap, dispMap, 0.0, disp);
    IMG *dbl = deblurFFTWInvariant( srcLeft->channel[0], psfBase, dispMap, par);
    sprintf( filename, "img/MPro/exp/test/dbl%02d.png", disp);
    saveImage( dbl, filename );
    printf("deblur finish! %d\n", disp);
    return 0;
  }
  return 0;
}
