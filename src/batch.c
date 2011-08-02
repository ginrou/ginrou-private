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


int batch110801_2( int argc, char* argv[] ){

  IMG* left = readImage("img/MBP/left.png");
  IMG* right = readImage("img/MBP/right.png");
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  makeShiftPSF( psfLeft, 0); // LEFT_CAM = 0
  makeShiftPSF( psfRight, 2);// RIGHT_CAM = 0

  IMG* leftCom[MAX_DISPARITY];
  IMG* rightCom[MAX_DISPARITY];

  IMG* map = createImage( left->height, left->width );

  char filename[256];

  // それぞれのPSFで畳み込み
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    convertScaleImage( map, map, 0.0, disp );
    leftCom[disp] = blurWithPSFMap( left, psfLeft, map);
    rightCom[disp] = blurWithPSFMap( right, psfRight, map );

    sprintf( filename, "img/test/left-%02d.png", disp);
    saveImage( leftCom[disp], filename);
  }
  // 畳み込んだ結果は leftCom, rightComへ


  IMG* dispMap = createImage( left->height, left->width);


  // 誤差が最も少ない画素が視差となる
  for( int h = 0; h < dispMap->height; ++h ){
    for( int w = 0 ; w < dispMap->width; ++w){

      double min = DBL_MAX;
      int disp;

      for(int d = 0 ; d < MAX_DISPARITY; ++d){

	int blk = 9;
	double err = 0.0;
	double hoge;

	for(int y = 0; y < blk; ++y){
	  for(int x = 0; x < blk; ++x){
	    if(h+y >= dispMap->height || w+x >= dispMap->width) continue;
	    hoge = IMG_ELEM( leftCom[d], h+y, w+x) - IMG_ELEM( rightCom[d], h+y, w+x);
	    err += hoge * hoge;

	  }
	}
	if( err < min ){
	  min = err;
	  disp = d;
	}
      }

      IMG_ELEM( dispMap, h, w) = disp * 4.0;
      if( h % 10 == 0 && w % 10 == 0)
	printf("disprity at %d, %d = %d\n", h, w, disp);
    }

  }

  saveImage( dispMap, "img/MBP/disp.png");


  // エラー計測
  IMG* gt = readImage("img/MBP/disparityMapLeft.png");
  IMG* sum = createImage( gt->height, gt->width );
  for(int h = 0 ; h < gt->height; ++h){
    for( int w = 0 ; w < gt->width ; ++w){
      int e = IMG_ELEM( gt, h, w) - IMG_ELEM(dispMap, h, w);
      IMG_ELEM(sum, h, w) = abs(e);
    }
  }

  saveImage( sum, "img/MBP/error.png");

  return 0;
}
