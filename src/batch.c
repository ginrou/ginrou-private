#include <batch.h>

int batch110731( int argc, char* argv[] )
{
  
  IMG* img = readImage( "img/MBP/center.png" );
  IMG* gt  = readImage( "img/MBP/left.png" );
  Mat psf[MAX_DISPARITY];

  for( int d = 0; d < MAX_DISPARITY; ++d){
    psf[d] = matrixAlloc( 1, MAX_DISPARITY);
    
    for(int h = 0; h < psf[d].row; ++h){
      for(int w = 0; w < psf[d].clm; ++w){
	ELEM0( psf[d], h, w) = 0.0;
      }
    }
    ELEM0( psf[d], 0, psf[d].clm/2 + d/2) = 1.0;

    IMG* tmp = createImage( psf[d].row, psf[d].clm);
    for(int h = 0; h < psf[d].row; ++h){
      for(int w = 0; w < psf[d].clm; ++w){
	IMG_ELEM(tmp, h, w) = ELEM0( psf[d], h, w);
      }
    }
    
  }

  IMG* map = readImage("img/MBP/disparityMapLeft.png");

  convertScaleImage(map, map, 1.0/4.0, 0.0);
  
  IMG* dst =  blurWithPSFMap( img, psf, map);

  IMG* dif = createImage( dst->height, dst->width );
  for( int h = 0 ; h < dst->height; ++h){
    for(int w = 0; w < dst->width; ++w){
      IMG_ELEM( dif, h, w) = 15.0*abs(IMG_ELEM( dst, h, w) - IMG_ELEM(gt, h, w));
    }
  }
  

  saveImage( dst, "img/MBP/test.png");
  saveImage( dif, "img/MBP/dif.png");
  return 0;


}

int batch110801( int argc, char* argv[] )
{
  // stereo + deblurのシステムの実装

  
  // 実験材料とパラメータ
  IMG_COL* srcLeft  = readImageColor("img/MPro/debug/DSC_0094.JPG");
  IMG_COL* srcRight  = readImageColor("img/MPro/debug/DSC_0095.JPG");
  IMG* psfBase  = readImage("img/MPro/debug/zhou005-110222.png");

  Mat fund = createHorizontalFundMat();

  IMG* dispMap = stereoRecursive( srcLeft, srcRight, &fund, 20, 0);
  saveImage( dispMap, "img/MPro/debug/dispMap.png");

  double par[] = {1.528633, -26.839933 };  

  char filename[256];

  par[0] = 1.0;par[1] = 0.0;
  for(int disp = 0; disp <MAX_DISPARITY; ++disp){
    convertScaleImage( dispMap, dispMap, 0.0, disp);
    IMG *dbl = deblurFFTWInvariant( srcLeft->channel[0], psfBase, dispMap, par);
    sprintf( filename, "img/MPro/exp/test/dbl%02d.png", disp);
    saveImage( dbl, filename );
    printf("deblur finish! %d\n", disp);

  }
  return 0;
}




