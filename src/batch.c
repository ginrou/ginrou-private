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

int batch110802( int argc, char* argv[] )
{
  IMG* left = readImage("img/MBP/110802-01/blurredLeft.png");
  IMG* right = readImage("img/MBP/110802-01/blurredRight.png");  
  IMG* cir = readImage("img/MBP/110802/circle.png");
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  Mat tmpPSF[MAX_DISPARITY];
  char filename[256];
  double par[2];


  par[0] = 3.579218;
  par[1] = -26.265516;
  makeShiftBlurPSF( psfLeft, LEFT_CAM, cir, par);

  par[0] = 1.500331;
  par[1] = -25.745794;
  makeShiftBlurPSF( psfRight, RIGHT_CAM, cir, par);

  IMG* hoge = createImage( 64, 128 );
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){

    IMG* img = createImage( psfLeft[disp].row, psfLeft[disp].clm );
    convertMat2IMG( &(psfLeft[disp]), img);
    sprintf(filename, "img/MBP/110802-01/test/psfLeft%02d.png", disp);
    resizeImage( img, hoge);
    saveImage( hoge, filename);
    releaseImage(&img);
    
    img = createImage( psfRight[disp].row, psfRight[disp].clm );
    convertMat2IMG( &(psfRight[disp]), img);
    sprintf(filename, "img/MBP/110802-01/test/psfRight%02d.png", disp);
    resizeImage( img, hoge);
    saveImage( hoge, filename);
    releaseImage(&img);

    normalizeMat( psfLeft[disp], psfLeft[disp]);
    normalizeMat( psfRight[disp], psfRight[disp]);
  }
  printf("make psf done\n");

  
  IMG* leftConv[MAX_DISPARITY];
  IMG* rightConv[MAX_DISPARITY];
  IMG* map = createImage( left->height, left->width );

  for(int d = 0; d < MAX_DISPARITY; ++d ){
    convertScaleImage( map, map, 0.0, d );

    //leftConv[d] = blurWithPSFMap( left, psfLeft, map );
    sprintf(filename, "img/MBP/110802-01/test/%02dbluLeft.png", d);
    //saveImage( leftConv[d], filename );
    leftConv[d] = readImage(filename);
    
    //rightConv[d] = blurWithPSFMap( right, psfRight, map );
    sprintf(filename, "img/MBP/110802-01/test/%02dbluRight.png", d);
    //saveImage( rightConv[d], filename );
    rightConv[d] = readImage(filename);
  }

  IMG* dispMap = createImage( left->height, left->width );
  for( int h = 0; h < dispMap->height; ++h){
    for( int w = 0; w < dispMap->width; ++w){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < MAX_DISPARITY/2; ++d){
	int blk = 16;
	double err = 0.0;
	double hoge;

	// ブロックマッチング
	for(int y = 0; y < blk; ++y){
	  for(int x= 0 ; x < blk; ++x){
	    
	    if( h+y >= dispMap->height || w+x >= dispMap->width) 
	      continue;

	    hoge = IMG_ELEM( leftConv[d], h+y, w+x ) - IMG_ELEM( rightConv[d], h+y, w+x);
	    
	    err += hoge*hoge;

	  }
	}
	
	if( err < min ){
	  min = err;
	  disp = d;
	}
      }

      IMG_ELEM( dispMap, h, w) = disp * 4.0;
      

    }
  }
  
  saveImage( dispMap, "img/MBP/110802-01/dispMap.png" );
}

int batch110804( int argc, char* argv[] ){
  IMG* left = readImage("img/MBP/110804/2//blurredZhouLeft.png");
  IMG* right = readImage("img/MBP/110804/2/blurredZhouRight.png");
  IMG* apeture = readImage("img/MBP/aperture/Zhou0002.png");
  
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  double par[2];

  IMG* leftDbl[MAX_DISPARITY];
  IMG* rightDbl[MAX_DISPARITY];

  // PSF を作る
  par[0] = 2.229461;
  par[1] = -42.780720;
  makeShiftBlurPSF( psfLeft, LEFT_CAM, apeture, par);

  par[0] = 3.396809;
  par[1] = -43.088126;
  makeShiftBlurPSF( psfRight, RIGHT_CAM, apeture, par);

  // deblurを行う
  IMG* psf;
  for( int d = 0; d < MAX_DISPARITY; ++d){

    char filename[256];

    psf = createImage( psfLeft[d].row, psfLeft[d].clm );
    convertMat2IMG( &(psfLeft[d]), psf );
    leftDbl[d] = deblurFFTW( left, psf );
    sprintf( filename, "img/MBP/110804/3/dbl%02dLeft.png", d);
    saveImage( leftDbl[d], filename);

    psf = createImage( psfRight[d].row, psfRight[d].clm );
    convertMat2IMG( &( psfRight[d] ), psf );
    rightDbl[d] = deblurFFTW( right, psf );
    sprintf( filename, "img/MBP/110804/3/dbl%02dRight.png", d);
    saveImage( rightDbl[d], filename);
    
  }


  // deblur結果から視差を計算
  IMG* dispMap = createImage( left->height, left->width);
  for( int h = 0 ;h < dispMap->height; ++h){
    for( int w = 0; w < dispMap->width; ++w){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < MAX_DISPARITY; ++d){
	int blk = 4;
	double err = 0.0;
	
	for(int y = 0; y < blk; ++y){
	  for(int x = 0;  x < blk; ++x ){
	    if( h+y >= dispMap->height || w+x >= dispMap->width)
	      continue;

	    double val =
	      IMG_ELEM( leftDbl[d], h + y, w + x )
	      - IMG_ELEM( rightDbl[d], h+y, w+x );
	      
	    err += val * val;

	  }
	}

	if( err < min ){
	  min = err;
	  disp = d;
	}

      }//d 

      IMG_ELEM( dispMap, h, w) = disp * 4.0;

    }//w 
  }//h

  saveImage( dispMap, "img/MBP/110804/3/disparityMap.png");

  return 0;

}


int batch_deblurTestCode( int argc, char* argv[])
{
    int h, w;
  IMG* src = readImage("img/test/LENNA.bmp");
  IMG* apeture = readImage("img/test/circle.png");
  IMG* blu = createImage( src->height, src->width);
  IMG* psf = createImage( 8, 8);
  resizeImage( apeture, psf);

  double norm = 0.0;


  //blurring
  for(h = 0; h < psf->height; ++h){
    for(w = 0; w < psf->width; ++w){
      norm += IMG_ELEM( psf, h, w);
    }
  }
  printf("norm of PSF = %lf\n", norm);
  
  for(h = 0; h < src->height; ++h){
    for(w = 0; w < src->width; ++w){
      double sum = 0.0;
      for(int y = 0; y < psf->height; ++y){
	for(int x = 0 ; x < psf->width; ++x){
	  int py = h + y - psf->height/2;
	  int px = w + x - psf->width/2;
	  if( py < 0 || py >= blu->height || px < 0 || px >= blu->width) continue;
	  sum += IMG_ELEM( src, py, px) * IMG_ELEM( psf, y, x);
	}
      }
      IMG_ELEM( blu, h, w) = sum / norm;
    }
  }

  IMG* dbl = deblurFFTW(blu, psf);

  IMG* sve = createImage( 64, 64);

  for( h = 0 ; h < sve->height; ++h){
    for( w = 0 ; w < sve->width ; ++w){
      IMG_ELEM( sve, h, w) = IMG_ELEM( blu, (blu->height-sve->height)/2 + h, (blu->width-sve->width)/2 + w);
    }
  }
  saveImage( sve, "img/test/blurred.png");


  for( h = 0 ; h < sve->height; ++h){
    for( w = 0 ; w < sve->width ; ++w){
      IMG_ELEM( sve, h, w) = IMG_ELEM( dbl, (blu->height-sve->height)/2 + h, (blu->width-sve->width)/2 + w);
    }
  }
  saveImage( sve, "img/test/deblurred.png");

  
  return 0;
}
