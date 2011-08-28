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

  IMG* dispMap;
  char filename[256];
  sprintf(filename, "img/MPro/debug/dispMap.png");

  /*
  dispMap = stereoRecursive( srcRight, srcLeft, &fund, 20, 0);
  saveImage( dispMap, "img/MPro/debug/dispMap.png");
  */

  dispMap = readImage(filename);
  double par[] = {0.5786, 3.034 };  

  IMG* dst = deblurFFTWInvariant( readImage("img/MPro/debug/DSC_0094.JPG"),
				  psfBase, dispMap, par );

  saveImage( dst, "img/MPro/debug/deblur.png");
  return 0;


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


int batch110808( int argc, char* argv[] ){ // stereo deblurの実験評価

  setbuf( stdout, NULL); // 改行をまたないように

  char dir[] = "img/MPro/exp/110808/";
  char srcPath[][256] = {
    "img/MPro/exp/110808/blurredCircleLeftBig.png",
    "img/MPro/exp/110808/blurredCircleRightBig.png",
    "img/MPro/exp/110808/blurredCircleLeftMin.png",
    "img/MPro/exp/110808/blurredCircleRightMin.png",
    "img/MPro/exp/110808/blurredZhouLeftBig.png",
    "img/MPro/exp/110808/blurredZhouRightBig.png",
    "img/MPro/exp/110808/blurredZhouLeftMin.png",
    "img/MPro/exp/110808/blurredZhouRightMin.png"
  };

  char psfPath[][256] = {
    "img/MPro/exp/110808/circle.png",
    "img/MPro/exp/110808/Zhou0002.png"
  };

  double param[2] = { 1.723563, -15.674991};
  IMG* disparityMap = readImage("img/MPro/exp/110808/disparityMap.png");
  convertScaleImage( disparityMap, disparityMap, 1.0/4.0, 0.0);

  char dstPath[][256] = {
    "img/MPro/exp/110808/dblCirBigKnown.png", 
    "img/MPro/exp/110808/dblCirBigUnknown.png", 
    "img/MPro/exp/110808/dblCirMinKnown.png", 
    "img/MPro/exp/110808/dblCirMinUnknown.png", 
    "img/MPro/exp/110808/dblZhouBigKnown.png", 
    "img/MPro/exp/110808/dblZhouBigUnknown.png", 
    "img/MPro/exp/110808/dblZhouMinKnown.png", 
    "img/MPro/exp/110808/dblZhouMinUnknown.png"
  };


  char filename[256];
  IMG_COL* src[8];
  for(int i = 0; i < 8; ++i) src[i] = readImageColor( srcPath[i] );

  IMG* psf[2];
  for(int i = 0; i < 2; ++i) psf[i] = readImage( psfPath[i] );

  // ステレオ法の比較
  Mat FundMat = createHorizontalFundMat();
  IMG* disp[4];
  for(int i = 0; i < 8; i += 2){
    sprintf( filename, "%sdisparityMap%02d.png", dir, i/2);
    disp[i/2] = stereoRecursive( src[i], src[i+1], &FundMat, 47, 0);
    //disp[i/2] = readImage(filename);
    saveImage( disp[i/2], filename);
  }
  
  // deblur
  for(int i = 0; i < 8; ++i){
    IMG* src = readImage( srcPath[2*(i/2)] );
    IMG* psfIn = psf[i/4];
    IMG* dispMap = ( i%2 == 0)? disparityMap : disp[i/2] ;
    IMG* dst = deblurFFTWInvariant( src, psfIn, dispMap, param );
    saveImage( dst, dstPath[i]);
  }

  return 0;
  

}


int batch110809( int argc, char* argv[] )
{
  IMG* left = readImage("img/MBP/110809/blurredLeft.png");
  IMG* right = readImage("img/MBP/110809/blurredRight.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  Mat psfLeft[MAX_DISPARITY];
  Mat psfRight[MAX_DISPARITY];
  
  double par[2];

  IMG* psfImg;
  char filename[256];


  par[0] = 1.253558;
  par[1] = -9.43181;
  makeShiftBlurPSF( psfLeft, LEFT_CAM, aperture, par);

  par[0] = 1.253558;
  par[1] = -7.050460;
  makeShiftBlurPSF( psfRight, RIGHT_CAM, aperture, par);


  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];


  for( int d = 0; d < MAX_DISPARITY; ++d){
     psfImg = createImage( psfLeft[d].row, psfLeft[d].clm);
    convertMat2IMG( &(psfLeft[d]), psfImg);
    dblLeft[d] = deblurFFTW( left, psfImg );

    psfImg = createImage( psfRight[d].row, psfRight[d].clm);    
    convertMat2IMG( &(psfRight[d]), psfImg);
    dblRight[d] = deblurFFTW( right, psfImg );
    
    sprintf( filename, "img/MBP/110809/%02ddblLeft.png", d);
    saveImage( dblLeft[d], filename);
    sprintf( filename, "img/MBP/110809/%02ddblRight.png", d);
    saveImage( dblRight[d], filename);
  }


  IMG* dst = createImage( left->height, left->width);
  for(int h = 0; h < dst->height; ++h){
    for(int w = 0; w < dst->width; ++w){

      int disp ;
      double min = DBL_MAX;
      
      for(int d = 0; d < MAX_DISPARITY; ++d){
	int blk = 4;
	double val = 0;
	for(int y = 0 ; y < blk; ++y){
	  for( int x = 0; x < blk; ++x){
	    
	    if( h+y < 0 || h+y >= dblLeft[d]->height ||
		w+x < 0 || w+x >= dblLeft[d]->width){
	      continue;
	    }
	    
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    val += a*a;

	  }
	}

	if( val < min ){
	  min = val;
	  disp = d;
	}
      }//d 

      IMG_ELEM( dst, h,w ) = disp;

    }
  }

  saveImage(dst, "img/MBP/110809/depthMap.png");


  return 0;


}

int batch110814( int argc, char* argv[] )
{
  
  IMG* left = readImage("img/MBP/110814-3/blurredLeft.png");
  IMG* right = readImage("img/MBP/110814-3/blurredRight.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  char filename[256];
  
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];
  double par[2];
  par[0] = 1.209449;
  par[1] = -9.166775;
  makeShiftBlurPSF( psfLeft, LEFT_CAM, aperture, par);

  par[1] = -7.144526;
  makeShiftBlurPSF( psfRight, RIGHT_CAM, aperture, par);
  
  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];
  IMG* psfImg;
  for(int d = 0; d < MAX_DISPARITY; ++d){
    psfImg = createImage( psfLeft[d].row, psfLeft[d].clm);
    convertMat2IMG( &(psfLeft[d]), psfImg);
    dblLeft[d] = deblurFFTW( left, psfImg);

    psfImg = createImage( psfRight[d].row, psfRight[d].clm);
    convertMat2IMG( &(psfRight[d]), psfImg);
    dblRight[d] = deblurFFTW( right, psfImg);

    sprintf( filename, "img/MBP/110814-3/test/dbl%02dLeft.png", d);
    saveImage( dblLeft[d], filename);
    sprintf( filename, "img/MBP/110814-3/test/dbl%02dRight.png", d);
    saveImage( dblRight[d], filename);

  }


  IMG* dst = createImage( left->height, left->width);
  for(int h = 0 ; h < dst->height; ++h){
    for( int w = 0; w < dst->width; ++w){
      
      int disp;
      double min = DBL_MAX;
      
      for( int d = 0; d < MAX_DISPARITY; ++d){
	int blk = 6;
	double sum = 0;

	for(int y = 0; y < blk; ++y){
	  for(int x = 0 ; x < blk; ++x){
	    
	    if( h+y < 0 || h+y >= dst->height ||
		w+x < 0 || w+x >= dst->width ) continue;
	    
	    double a;
	    a = IMG_ELEM( dblLeft[d], h+y, w+x) - IMG_ELEM( dblRight[d], h+y, w+x);
	    sum += a*a;
	    
	  }
	}

	if( sum < min ){
	  min = sum;
	  disp = d;
	}

      }

      IMG_ELEM( dst, h, w) = disp;

    }
  }
    
    convertScaleImage( dst, dst, 4.0, 0.0);
    saveImage( dst, "img/MBP/110814-3/dst.png");

    return 0;


}


int batch110815( int argc, char* argv[] )
{
    char filename[256];
  IMG* left[4];
  IMG* right[4];
  IMG* dst[4];

  for(int i = 0; i < 4; ++i){
    sprintf( filename, "img/MBP/110815-1/blurredLeft%d.png", i+1);
    left[i] = readImage( filename );
    sprintf( filename, "img/MBP/110815-1/blurredRight%d.png", i+1);
    right[i] = readImage( filename );
  }

  IMG* ca1  = readImage("img/MBP/aperture/CAPair1.png");
  IMG* ca2  = readImage("img/MBP/aperture/CAPair2.png");
  IMG* zhou = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* cir  = readImage("img/MBP/aperture/circle.png");

  double par[2][2];
  int blk = 4;

  /*
  // 自分のシステム
  par[0][0] = 1.777778;
  par[0][1] = -18.673244;
  par[1][0] = 1.777778;
  par[1][1] = -15.371831;
  printf("start current system\n");

  dst[0] = currentSystemDispmap( left[0], right[0],
				 zhou, zhou,
				 par, 30, blk);
  saveImage( dst[0], "img/MBP/110815-1/dispmap1.png");
  */
  // Coded Aperture Pair
  printf("coded aperture pair\n");
  par[0][0] = 1.0;
  par[0][1] = 0.0;
  dst[1] = CodedAperturePairDispmap( left[1], right[1],
				     ca1, ca2, par[0], 
				     30, blk);
  saveImage( dst[1], "img/MBP/110815-1/dispmap2.png");
  return 0;
  // ステレオ法
  IMG_COL *leftCol = readImageColor("img/MBP/110815-1/blurredLeft3.png");
  IMG_COL *rightCol = readImageColor("img/MBP/110815-1/blurredRight3.png");
  Mat fund = createHorizontalFundMat();
  dst[2] = stereoRecursive( leftCol, rightCol, &fund, 30, 0);
  saveImage( dst[2], "img/MBP/110815-1/dispmap3.png");

  // Depth from defocus
  par[0][0] = 1.777778;
  par[0][1] = -18.673244;
  par[1][0] = 1.777778;
  par[1][1] = -15.371831;
  dst[3] = DepthFromDeocus( left[3], right[3], cir, par, blk);
  saveImage( dst[3], "img/MBP/110815-1/dispmap4.png");

  return 0;

}


// 修論用システムの実験2 CAPairを改良
int batch110816( int argc, char* argv[] ){
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

  dst[0] = currentSystemDispmap2( left[0], right[0],
				 zhou, zhou,
				 par, 30, blk);
  saveImage( dst[0], "img/MBP/110816-1/dispmap1-1.png");

  return 0;


  // Coded Aperture Pair
  printf("coded aperture pair\n");
  par[0][0] = 1.0;
  par[0][1] = 0.0;
  dst[1] = CodedAperturePairDispmap2( left[1], right[1],
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

  return 0;

}

/*
  周波数領域におけるカーネルの線形補間を用いる事で
  中途半端なサイズのカーネルでもdeblurできるようになった
*/
int batch110827( int argc, char* argv[] )
{
  IMG* img = readImage("img/MBP/110826-1/blurredwin.png");
  IMG* apertureIn = readImage("img/MBP/aperture/Zhou0002.png");
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *psf = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);
  int h, w;

  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
      psf[idx][0] = 0.0;
      psf[idx][1] = 0.0;
    }
  }


  //copy psf
  IMG* aperture1 = createImage( 15, 15);
  IMG* aperture2 = createImage( 18, 18);
  flipImage( apertureIn, 1, 1);
  resizeImage( apertureIn, aperture1);
  resizeImage( apertureIn, aperture2);
  fftw_complex *psf1 = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *psf2 = (fftw_complex*)fftw_malloc(memSize);
  
  for(int i = 0; i < img->height * img->width; ++i){
    psf1[i][0] = 0.0;
    psf1[i][1] = 0.0;
    psf2[i][0] = 0.0;
    psf2[i][1] = 0.0;
  }

  // psf1
  double norm1 = imageNormL1(aperture1);
  for(h = 0; h < aperture1->height; ++h){
    for(w = 0 ; w < aperture1->width; ++w){
      int y = h - aperture1->height/2;
      int x = w - aperture1->width/2;
      y += (y<0) ? img->height : 0;
      x += (x<0) ? img->width : 0;
      int idx = y * img->width + x;
      psf1[idx][0] = DBL_ELEM( aperture1, h, w) / norm1;
    }
  }

  // psf2
  double norm2 = imageNormL1(aperture2);
  for(h = 0; h < aperture2->height; ++h){
    for(w = 0 ; w < aperture2->width; ++w){
      int y = h - aperture2->height/2;
      int x = w - aperture2->width/2;
      y += (y<0) ? img->height : 0;
      x += (x<0) ? img->width : 0;
      int idx = y * img->width + x;
      psf2[idx][0] = DBL_ELEM( aperture2, h, w) / norm2;
    }
  }  
  
  // makeplan and FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pSrc);

  fftw_plan pPsf1 = fftw_plan_dft_2d( img->height, img->width, psf1, psf1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan pPsf2 = fftw_plan_dft_2d( img->height, img->width, psf2, psf2, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( pPsf1 );
  fftw_execute( pPsf2 );


  //deblur

  for(h = 0; h < img->height; ++h){
    for(w = 0 ; w < img->width; ++w){
      int idx = h * img->width + w;
      double a = src[idx][0] ;
      double b = src[idx][1] ;
      double c = ( 2.0 * psf1[idx][0] + 1.0 * psf2[idx][0] ) / 3.0;
      double d = ( 2.0 * psf1[idx][1] + 1.0 * psf2[idx][1] ) / 3.0;
      double snr = 0.002;
      dbl[idx][0] = ( a*c + b*d ) / ( c*c + d*d + snr );
      dbl[idx][1] = ( b*c - a*d ) / ( c*c + d*d + snr );

    }
  }

  //IDFT
  fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute( pDbl );

  IMG* dst = createImage( img->height, img->width );
  double scale = img->height * img->width;
  for( h = 0; h < dst->height; ++h){
    for( w = 0; w < dst->width; ++w){
      int idx = h * img->width + w;
      double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
      IMG_ELEM( dst, h, w) = sqrt(val) / scale;
      //printf("dbl = %lf + %lf  i \n", dbl[idx][0]/scale, dbl[idx][1]/scale);      
    }
  }

  saveImage( dst, "img/MBP/110826-1/deblurredResize.png");

  return 0;

}



/*
  サブピクセルオーダーの視差マップを用いる事で、より高精度のぼけ除去を行えるように
 */
int batch110828_1( int argc, char* argv[] ){
  int h, w;
  IMG* img      = readImage("img/MBP/110827-1/blurredLeft5.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* dispMap  = readImage("img/MBP/110827-1/disparityMap5.png");
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);

  //showDispMap(dispMap);
  //return 0;

  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
    }
  }

  //copy psf
  int kernels = 50;
  fftw_complex *psf[50];
  flipImage( aperture, 0, 1); //0,1
  for( int i = 1; i < kernels; ++i ){

    IMG* apIn = createImage( i, i );
    resizeImage( aperture, apIn );

    psf[i] = (fftw_complex*)fftw_malloc(memSize);
    for(int n = 0; n < img->height * img->width; ++n){
      psf[i][n][0] = 0.0;
      psf[i][n][1] = 0.0;
    }
    
    double norm = imageNormL1(apIn);
    for( h = 0; h < i; ++h ){
      for( w = 0 ; w < i; ++w ){
	int y = h - i/2;
	int x = w - i/2;
	y += (y<0) ? img->height : 0;
	x += (x<0) ? img->width  : 0;
	int idx = y * img->width + x;
	psf[i][idx][0] = DBL_ELEM( apIn, h, w) / norm ;
      }
    }
    releaseImage( &apIn );
  }
	 


  // makeplan and FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pSrc);
  
  fftw_plan pPSF;
  for( int i = 1; i < kernels; ++i){
    pPSF = fftw_plan_dft_2d( img->height, img->width, psf[i], psf[i], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( pPSF );
    fftw_destroy_plan( pPSF );
  }

  //deblur
  IMG *dblImg[MAX_DISPARITY];
  fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);

  double param1[2] = { -1.2857, 13.3661};
  double param2[2] = { -1.6413, 17.838};
  double param51[2] = { -5.6075/4.0, 43.1935};
  double param52[2] = { 3.621871 / 4.0, -28.161897 };
  double paramCalib[2] = { 0.1, 3.0};
  double *param = param52;

  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    double size = fabs( (double)disp * param[0] + param[1] );
    double r = size - (int)size;

    if( size < 1.0 ) {size = 1.0; r = 0.0 ;}
    printf("dispartiy = %d, size = %lf  r = %lf\n", disp, size, r);

    if( size > 50 )continue;

    for(h = 0; h < img->height; ++h){
      for(w = 0 ; w < img->width; ++w){
	int idx = h * img->width + w;
	double a = src[idx][0] ;
	double b = src[idx][1] ;
	double c = (1-r)*psf[(int)size][idx][0] + r* psf[(int)size+1][idx][0];
	double d = (1-r)*psf[(int)size][idx][1] + r* psf[(int)size+1][idx][1];

	double snr = 0.002;
	dbl[idx][0] = ( a*c + b*d ) / ( c*c + d*d + snr);
	dbl[idx][1] = ( b*c - a*d ) / ( c*c + d*d + snr);
      }
    }

    //IDFT
    fftw_execute( pDbl );

    dblImg[disp] = createImage( img->height, img->width );
    double scale = img->height * img->width;
    for( h = 0; h < dblImg[disp]->height; ++h){
      for( w = 0; w < dblImg[disp]->width; ++w){
	int idx = h * img->width + w;
	double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
	IMG_ELEM( dblImg[disp], h, w) = sqrt(val) / scale;
      }
    }

    if( param == paramCalib){
      char filename[256];
      sprintf( filename, "img/MBP/110827-1/test/%02d.png", disp);
      saveImage( dblImg[disp], filename );
    }
  }

  IMG* dst = createImage( img->height, img->width );
  for( h = 0; h < dst->height; ++h){
    for( w = 0; w < dst->width; ++w){
      int disp = IMG_ELEM( dispMap, h, w);
      IMG_ELEM( dst, h, w) = IMG_ELEM( dblImg[disp], h, w);
    }
  }

  saveImage( dst, "img/MBP/110827-1/deblurredImage.png");

  return 0;

}


int parameterCalibration( int argc, char* argv[])
{
  int h, w;
  IMG* img = readImage("img/MBP/110828-1/blurredRight.png");
  IMG* aperture = readImage("img/MBP/aperture/Zhou0002.png");
  IMG* dispMap = readImage("img/MBP/110828-1/disparityMap.png");
  size_t memSize = sizeof(fftw_complex) * img->height * img->width ;
  fftw_complex *src = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex *dbl = (fftw_complex*)fftw_malloc(memSize);

  //copy src
  for( h = 0; h < img->height ; ++h ){
    for( w = 0; w < img->width; ++w){
      int idx = h * img->height + w;
      src[idx][0] = (double)IMG_ELEM( img, h, w);
      src[idx][1] = 0.0;
    }
  }

  //copy psf
  int kernels = MAX_DISPARITY;
  fftw_complex *psf[MAX_DISPARITY];
  flipImage( aperture, 1, 0); //0,1
  for( int i = 1; i < kernels; ++i ){

    IMG* apIn = createImage( i, i );
    resizeImage( aperture, apIn );

    psf[i] = (fftw_complex*)fftw_malloc(memSize);
    for(int n = 0; n < img->height * img->width; ++n){
      psf[i][n][0] = 0.0;
      psf[i][n][1] = 0.0;
    }
    
    double norm = imageNormL1(apIn);
    for( h = 0; h < i; ++h ){
      for( w = 0 ; w < i; ++w ){
	int y = h - i/2;
	int x = w - i/2;
	y += (y<0) ? img->height : 0;
	x += (x<0) ? img->width  : 0;
	int idx = y * img->width + x;
	psf[i][idx][0] = DBL_ELEM( apIn, h, w) / norm ;
      }
    }
    releaseImage( &apIn );
  }
  
  // makeplan and FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pSrc);
  
  fftw_plan pPSF;
  for( int i = 1; i < kernels; ++i){
    pPSF = fftw_plan_dft_2d( img->height, img->width, psf[i], psf[i], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( pPSF );
    fftw_destroy_plan( pPSF );
  }
  //deblur
  IMG *dblImg[MAX_DISPARITY];
  fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);

  double paramCalib[2] = { 0.1, 2.0};
  double paramLeft[2]  = { 1.6381, -25.5643};
  double paramRight[2] = { -2.2457, 28.5836};
  double *param = paramRight;

  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    double size = fabs( (double)disp/4.0 * param[0] + param[1] );
    double r = size - (int)size;

    if( size < 1.0 ) {size = 1.0; r = 0.0 ;}
    printf("dispartiy = %d, size = %lf  r = %lf\n", disp, size, r);

    if( size > 50 )continue;

    for(h = 0; h < img->height; ++h){
      for(w = 0 ; w < img->width; ++w){

	int idx = h * img->width + w;
	double a = src[idx][0] ;
	double b = src[idx][1] ;
	double c = (1-r)*psf[(int)size][idx][0] + r* psf[(int)size+1][idx][0];
	double d = (1-r)*psf[(int)size][idx][1] + r* psf[(int)size+1][idx][1];

	double snr = 0.002;
	dbl[idx][0] = ( a*c + b*d ) / ( c*c + d*d + snr);
	dbl[idx][1] = ( b*c - a*d ) / ( c*c + d*d + snr);
      }
    }

    //IDFT
    fftw_execute( pDbl );

    dblImg[disp] = createImage( img->height, img->width );
    double scale = img->height * img->width;
    for( h = 0; h < dblImg[disp]->height; ++h){
      for( w = 0; w < dblImg[disp]->width; ++w){
	int idx = h * img->width + w;
	double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
	IMG_ELEM( dblImg[disp], h, w) = sqrt(val) / scale;
      }
    }

    if( param == paramCalib){
      char filename[256];
      sprintf( filename, "img/MBP/110828-1/test/%02d.png", disp);
      saveImage( dblImg[disp], filename );
    }
  }

  IMG* dst = createImage( img->height, img->width );
  for( h = 0; h < dst->height; ++h){
    for( w = 0; w < dst->width; ++w){
      int disp = IMG_ELEM( dispMap, h, w) / 1.0;
      IMG_ELEM( dst, h, w) = IMG_ELEM( dblImg[disp], h, w);
    }
  }

  saveImage( dst, "img/MBP/110828-1/deblurredImage.png");

  


  return 0;


}
