#include "expsystem.h"

// 現在の自分のシステムの奥行き推定システム
IMG* currentSystemDispmap
( IMG* srcLeft, 
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2][2],
  int maxDisparity,
  int blockSize
  )
{
  Mat psfLeft[MAX_DISPARITY], psfRight[MAX_DISPARITY];

  makeShiftBlurPSF( psfLeft, LEFT_CAM, apertureLeft, param[0]);
  makeShiftBlurPSF( psfRight, RIGHT_CAM, apertureRight, param[1]);

  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];
  IMG* psfImg;

  // deblur part
  for( int d = 0; d < maxDisparity; ++d){
    psfImg = createImage( psfLeft[d].row, psfLeft[d].clm);
    convertMat2IMG( &(psfLeft[d]), psfImg);
    dblLeft[d] = deblurFFTW( srcLeft, psfImg);
    releaseImage(&psfImg);

    psfImg = createImage( psfRight[d].row, psfRight[d].clm);
    convertMat2IMG( &(psfRight[d]), psfImg);
    dblRight[d] = deblurFFTW( srcRight, psfImg);
    releaseImage(&psfImg);

    _ClearLine();
    printf("%02d / %02d done", d+1 , maxDisparity);
  }

  printf("\n");

  // depth estiamtion part
  IMG* dst = createImage( srcLeft->height, srcRight->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      
      int disp;
      double min = DBL_MAX;

      for( int d = 0; d <maxDisparity; ++d){
	double sum = 0.0;
	
	for( int y = 0; y < blockSize ; ++y){
	  for( int x = 0; x < blockSize; ++x){
	    
	    if( h+y < 0 || h+y >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;

	  }
	}

	if( sum < min ){
	  min = sum;
	  disp = d;
	}

      }// d
      IMG_ELEM( dst, h, w ) = disp;
    }// w
  }// h
  
  // クリーニング
  

  for( int d = 0; d < maxDisparity; ++d){
    matrixFree( psfLeft[d] );
    matrixFree( psfRight[d] );
    releaseImage( &(dblLeft[d]) );
    releaseImage( &(dblRight[d]) );
  }

  return dst;

}


IMG* CodedAperturePairDispmap
( IMG* srcLeft,
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2],
  int maxDepth,
  int blockSize
  )
{

  IMG* psfLeft[MAX_PSF_SIZE];
  IMG* psfRight[MAX_PSF_SIZE];
  makeBlurPSF( psfLeft, apertureLeft, MAX_PSF_SIZE, param);
  makeBlurPSF( psfRight, apertureRight, MAX_PSF_SIZE, param);

  IMG* dblLeft[MAX_PSF_SIZE];
  IMG* dblRight[MAX_PSF_SIZE];

  for( int d = 0; d < MAX_PSF_SIZE; ++d){
    dblLeft[d] = deblurFFTW( srcLeft, psfLeft[d] );
    dblRight[d] = deblurFFTW( srcRight, psfRight[d] );
    char filename[256];
    sprintf(filename, "img/MBP/110815-1/test/dblLeft%02d.png", d);
    saveImage( dblLeft[d], filename);
    sprintf(filename, "img/MBP/110815-1/test/dblRight%02d.png", d);
    saveImage( dblRight[d], filename);
  }

  printf("deblurring done\n");

  IMG* dst = createImage( srcLeft->height, srcLeft->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      int disp;
      double min;
      for( int d = 0; d < MAX_PSF_SIZE; ++d){
	double sum = 0;
	
	for(int y = 0; y < blockSize; ++y){
	  for(int x = 0 ; x < blockSize; ++x){
	    if( y+h < 0 || y+h >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;
	  }//x
	}//y

	if( min > sum ){
	  sum = min;
	  disp = d;
	}

      }//d

      IMG_ELEM( dst, h, w) = disp;

    }//w
  }//x

  for(int i = 0 ; i < MAX_PSF_SIZE; ++i){
    releaseImage( &(psfLeft[i]) );
    releaseImage( &(psfRight[i]) );
    releaseImage( &(dblLeft[i]) );
    releaseImage( &(dblRight[i]) );
  }
  return dst;
}



// ピント位置を変更したDFD
IMG* DepthFromDeocus
( IMG* srcLeft,
  IMG* srcRight,
  IMG* aperture,
  double param[2][2],
  int blockSize
  )
{
  IMG* psfLeft[MAX_DISPARITY];
  IMG* psfRight[MAX_DISPARITY];
  makeBlurPSF( psfLeft, aperture, MAX_DISPARITY, param[0] );
  makeBlurPSF( psfRight, aperture, MAX_DISPARITY, param[0] );

  IMG* dblLeft[MAX_DISPARITY];
  IMG* dblRight[MAX_DISPARITY];

  for( int d = 0 ; d < MAX_DISPARITY; ++d){
    dblLeft[d] = deblurFFTW( srcLeft, psfLeft[d] );
    dblRight[d] = deblurFFTW( srcRight, psfRight[d] );
  }

  IMG* dst = createImage( srcLeft->height, srcLeft->width);
  for( int h = 0 ; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      int disp;
      double min;
      for( int d = 0; d < MAX_DISPARITY; ++d){
	double sum = 0;
	
	for(int y = 0; y < blockSize; ++y){
	  for(int x = 0 ; x < blockSize; ++x){
	    if( y+h < 0 || y+h >= dst->height ||
		w+x < 0 || w+x >= dst->width) continue;
	    double a;
	    a = IMG_ELEM(dblLeft[d], h+y, w+x) - IMG_ELEM(dblRight[d], h+y, w+x);
	    sum += a*a;
	  }//x
	}//y

	if( min > sum ){
	  sum = min;
	  disp = d;
	}

      }//d

      IMG_ELEM( dst, h, w) = disp;

    }//w
  }//x

  for(int i = 0 ; i < MAX_DISPARITY; ++i){
    releaseImage( &(psfLeft[i]) );
    releaseImage( &(psfRight[i]) );
    releaseImage( &(dblLeft[i]) );
    releaseImage( &(dblRight[i]) );
  }
  return dst;

}


IMG* CodedAperturePairDispmap2
( IMG* srcLeft,
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2],
  int maxDepth,
  int blockSize
  )
{
  /****************************************
    大文字始まりの変数は周波数領域における
    変数を表す
  *****************************************/
  int h, w;
  int height = srcLeft->height;
  int width  = srcLeft->width;
  size_t memSize = sizeof(fftw_complex) * height * width;

  int maxSize = abs(maxDepth * param[0] + param[1]);
  if( maxSize < abs(param[1]) ) maxSize = abs(param[1]);

  // src画像のFFT
  fftw_complex *Left = (fftw_complex*)fftw_malloc( memSize );
  fftw_complex *Right = (fftw_complex*)fftw_malloc( memSize );
  
  // copy and windowfunction
  for(h=0;h<height;++h){
    for( w = 0 ; w < width ; ++w ){
      int idx = h * width + w;
      double wh, ww;
      
      if( h < maxSize/2 ) 
	wh = 0.5 - 0.5*cos( (double)h * M_PI * 2.0 / (double)maxSize);
      else if( h >= height - maxSize/2)
	wh = 0.5 - 0.5*cos( (double)(h-height+maxSize) * M_PI * 2.0 / (double)maxSize);
      else 
	wh = 1.0;

      if( w < maxSize/2 ) 
	ww = 0.5 - 0.5*cos( (double)w * M_PI * 2.0 / (double)maxSize);
      else if( w >= width - maxSize/2)
	ww = 0.5 - 0.5*cos( (double)(w-width + maxSize) * M_PI * 2.0 / (double)maxSize);
      else 
	ww = 1.0;

      Left[idx][0] = (double)IMG_ELEM( srcLeft, h, w) * ww * wh;
      Left[idx][1] = 0.0;
      Right[idx][0] = (double)IMG_ELEM( srcRight, h, w) * ww * wh;
      Right[idx][1] = 0.0;

    }
  }

  // makeplan and FFT
  fftw_plan planLeft = fftw_plan_dft_2d( height, width, Left, Left, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan planRight = fftw_plan_dft_2d( height, width, Right, Right, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(planLeft);
  fftw_execute(planRight);


  // PSFのFFT
  fftw_complex* LeftPSF[MAX_DISPARITY];
  fftw_complex* RightPSF[MAX_DISPARITY];
  fftw_plan planLeftPSF[MAX_DISPARITY];
  fftw_plan planRightPSF[MAX_DISPARITY];

  IMG* leftPSFImg[MAX_DISPARITY];
  IMG* rightPSFImg[MAX_DISPARITY];
  makeBlurPSF( leftPSFImg, apertureLeft, maxDepth, param );
  makeBlurPSF( rightPSFImg, apertureRight, maxDepth, param );

  for( int d = 0 ; d < maxDepth; ++d){
    LeftPSF[d] = (fftw_complex*)fftw_malloc(memSize);
    RightPSF[d] = (fftw_complex*)fftw_malloc(memSize);

    double leftSum = 0.0;
    double rightSum = 0.0;
    
    // copy left
    for( h = 0; h < leftPSFImg[d]->height ; ++h){
      for( w = 0 ; w < leftPSFImg[d]->width; ++w){
	int y = h - leftPSFImg[d]->height / 2;
	int x = w - leftPSFImg[d]->width / 2;
	y += (y<0) ? height : 0;
	x += (x<0) ? width  : 0;
	int idx = y * width + w;
	LeftPSF[d][idx][0] = (double)IMG_ELEM( leftPSFImg[d], h, w);
	leftSum += (double)IMG_ELEM( leftPSFImg[d], h, w);
      }
    }

    // copy right
    for( h = 0; h < rightPSFImg[d]->height ; ++h){
      for( w = 0 ; w < rightPSFImg[d]->width; ++w){
	int y = h - rightPSFImg[d]->height / 2;
	int x = w - rightPSFImg[d]->width / 2;
	y += (y<0) ? height : 0;
	x += (x<0) ? width  : 0;
	int idx = y * width + w;
	RightPSF[d][idx][0] = (double)IMG_ELEM( rightPSFImg[d], h, w);
	rightSum += (double)IMG_ELEM( rightPSFImg[d], h, w);
      }
    }

    // normalize
    for( h = 0 ; h < height ; ++h){
      for( w = 0 ; w < width ; ++w ){
	int idx = h * width + w;
	LeftPSF[d][idx][0] /= leftSum;
	RightPSF[d][idx][0] /= rightSum;
      }
    }

    // FFT
    planLeftPSF[d] = fftw_plan_dft_2d( height, width, LeftPSF[d] , LeftPSF[d], FFTW_FORWARD, FFTW_ESTIMATE);
    planRightPSF[d] = fftw_plan_dft_2d( height, width, RightPSF[d] , RightPSF[d], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( planLeftPSF[d] );
    fftw_execute( planRightPSF[d] );

  }// d

  printf("fft done\n");

  // ぼけ除去画像の作成
  fftw_complex* DblLeft[MAX_DISPARITY];
  fftw_complex* DblRight[MAX_DISPARITY];
  for( int d = 0; d < maxDepth; ++d){
    DblLeft[d] = (fftw_complex*)fftw_malloc(memSize);
    DblRight[d] = (fftw_complex*)fftw_malloc(memSize);

    for( h = 0 ; h < height; ++h){
      for( w = 0; w < width; ++w){
	int idx = h * width + w ;
	double a, b, c, e;
	double snr = 0.002;

	a = Left[idx][0];
	b = Left[idx][1];
	c = LeftPSF[d][idx][0];
	e = LeftPSF[d][idx][1];
	DblLeft[d][idx][0] = ( a*c + b*e ) / ( c*c + e*e + snr );
	DblLeft[d][idx][1] = ( b*c - a*e ) / ( c*c + e*e + snr );

	a = Right[idx][0];
	b = Right[idx][1];
	c = RightPSF[d][idx][0];
	e = RightPSF[d][idx][1];
	DblRight[d][idx][0] = ( a*c + b*e ) / ( c*c + e*e + snr );
	DblRight[d][idx][1] = ( b*c - a*e ) / ( c*c + e*e + snr );

      }//w
    }//h
  }//d

  printf("deblurring done\n");
  
  Mat residualMap[MAX_DISPARITY];
  fftw_complex* rMapLeft = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex* rMapRight = (fftw_complex*)fftw_malloc(memSize);
  fftw_plan planRmapLeft = fftw_plan_dft_2d( height, width, rMapLeft, rMapLeft, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_plan planRmapRight = fftw_plan_dft_2d( height, width, rMapRight, rMapRight, FFTW_BACKWARD, FFTW_ESTIMATE);
 
  IMG* tmp1 = createImage( height, width);
  IMG* tmp2 = createImage( height, width);
  for( int d = 0; d < maxDepth; ++d){
    residualMap[d] = matrixAlloc( width, height);
    
    for( h = 0 ; h < height; ++h ){
      for( w = 0 ; w < width; ++w ){
	int idx = h * width + w;
	double a,b,c,e;
	
	a = DblLeft[d][idx][0];
	b = DblLeft[d][idx][1];
	c = LeftPSF[d][idx][0];
	e = LeftPSF[d][idx][1];
	rMapLeft[idx][0] = a*c - b*e - Left[idx][0];
	rMapLeft[idx][1] = a*e + b*c - Left[idx][1];

	a = DblRight[d][idx][0];
	b = DblRight[d][idx][1];
	c = RightPSF[d][idx][0];
	e = RightPSF[d][idx][1];
	rMapRight[idx][0] = a*c - b*e - Right[idx][0];
	rMapRight[idx][1] = a*e + b*c - Right[idx][1];

      }
    }

    fftw_execute( planRmapLeft );
    fftw_execute( planRmapRight );

    for( h = 0 ; h < height; ++h ){
      for( w = 0 ; w < width; ++w ){
	int idx = h * width + w;
	double l = rMapLeft[idx][0] * rMapLeft[idx][0] + rMapLeft[idx][1] * rMapLeft[idx][1];
	double r = rMapRight[idx][0]*rMapRight[idx][0] + rMapRight[idx][1]*rMapRight[idx][1];
	ELEM0( residualMap[d], h, w) = sqrt(l) + sqrt(r);

      }
    }

  }//d 
  printf("compute residual map done\n");

  IMG* img = createImage( height, width);
  for(int d = 0; d < maxDepth; ++d){
    convertMat2IMG( &(residualMap[d]), img);
    char filename[256];
    sprintf( filename, "img/MBP/110816-1/test/resmap%02d.png", d);
    saveImage( img, filename);
  }


  IMG* dst = createImage( height, width);
  for( h = 0 ; h < height; ++h ){
    for( w = 0; w < width; ++w ){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < maxDepth; ++d){
	double val = ELEM0( residualMap[d], h, w);
	if(val < min ){
	  min = val;
	  disp = d;
	}
      }

      IMG_ELEM( dst, h , w) = disp;

    }
  }

  return dst;

}



IMG* currentSystemDispmap2
( IMG* srcLeft, 
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2][2],
  int maxDisparity,
  int blockSize
  )
{
  int h, w;
  int height = srcLeft->height;
  int width  = srcLeft->width;
  size_t memSize = sizeof(fftw_complex) * height * width;

  int maxSize = abs(maxDisparity * param[0][0] + param[0][1]);
  if( maxSize < abs(param[0][1]) ) maxSize = abs(param[0][1]);

  // src画像のFFT
  fftw_complex *Left = (fftw_complex*)fftw_malloc( memSize );
  fftw_complex *Right = (fftw_complex*)fftw_malloc( memSize );
  
  // copy and windowfunction
  for(h=0;h<height;++h){
    for( w = 0 ; w < width ; ++w ){
      int idx = h * width + w;
      double wh, ww;
      
      if( h < maxSize/2 ) 
	wh = 0.5 - 0.5*cos( (double)h * M_PI * 2.0 / (double)maxSize);
      else if( h >= height - maxSize/2)
	wh = 0.5 - 0.5*cos( (double)(h-height+maxSize) * M_PI * 2.0 / (double)maxSize);
      else 
	wh = 1.0;

      if( w < maxSize/2 ) 
	ww = 0.5 - 0.5*cos( (double)w * M_PI * 2.0 / (double)maxSize);
      else if( w >= width - maxSize/2)
	ww = 0.5 - 0.5*cos( (double)(w-width + maxSize) * M_PI * 2.0 / (double)maxSize);
      else 
	ww = 1.0;

      Left[idx][0] = (double)IMG_ELEM( srcLeft, h, w) * ww * wh;
      Left[idx][1] = 0.0;
      Right[idx][0] = (double)IMG_ELEM( srcRight, h, w) * ww * wh;
      Right[idx][1] = 0.0;

    }
  }

  // makeplan and FFT
  fftw_plan planLeft = fftw_plan_dft_2d( height, width, Left, Left, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan planRight = fftw_plan_dft_2d( height, width, Right, Right, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(planLeft);
  fftw_execute(planRight);


  // PSFのFFT
  fftw_complex* LeftPSF[MAX_DISPARITY];
  fftw_complex* RightPSF[MAX_DISPARITY];
  fftw_plan planLeftPSF[MAX_DISPARITY];
  fftw_plan planRightPSF[MAX_DISPARITY];

  Mat leftPSFMat[MAX_DISPARITY];
  Mat rightPSFMat[MAX_DISPARITY];
  makeShiftBlurPSF( leftPSFMat, LEFT_CAM, apertureLeft, param[0]);
  makeShiftBlurPSF( rightPSFMat, RIGHT_CAM, apertureRight, param[1]);

  IMG* leftPSFImg[MAX_DISPARITY];
  IMG* rightPSFImg[MAX_DISPARITY];
  for( int d = 0 ; d < maxDisparity; ++d){

    leftPSFImg[d] = createImage( leftPSFMat[d].clm, leftPSFMat[d].row );
    convertMat2IMG( &(leftPSFMat[d]), leftPSFImg[d] );
    rightPSFImg[d] = createImage( rightPSFMat[d].clm, rightPSFMat[d].row );
    convertMat2IMG( &(rightPSFMat[d]), rightPSFImg[d] );


    LeftPSF[d] = (fftw_complex*)fftw_malloc(memSize);
    RightPSF[d] = (fftw_complex*)fftw_malloc(memSize);

    double leftSum = 0.0;
    double rightSum = 0.0;
    
    // copy left
    for( h = 0; h < leftPSFImg[d]->height ; ++h){
      for( w = 0 ; w < leftPSFImg[d]->width; ++w){
	int y = h - leftPSFImg[d]->height / 2;
	int x = w - leftPSFImg[d]->width / 2;
	y += (y<0) ? height : 0;
	x += (x<0) ? width  : 0;
	int idx = y * width + w;
	LeftPSF[d][idx][0] = (double)IMG_ELEM( leftPSFImg[d], h, w);
	leftSum += (double)IMG_ELEM( leftPSFImg[d], h, w);
      }
    }

    // copy right
    for( h = 0; h < rightPSFImg[d]->height ; ++h){
      for( w = 0 ; w < rightPSFImg[d]->width; ++w){
	int y = h - rightPSFImg[d]->height / 2;
	int x = w - rightPSFImg[d]->width / 2;
	y += (y<0) ? height : 0;
	x += (x<0) ? width  : 0;
	int idx = y * width + w;
	RightPSF[d][idx][0] = (double)IMG_ELEM( rightPSFImg[d], h, w);
	rightSum += (double)IMG_ELEM( rightPSFImg[d], h, w);
      }
    }

    // normalize
    for( h = 0 ; h < height ; ++h){
      for( w = 0 ; w < width ; ++w ){
	int idx = h * width + w;
	LeftPSF[d][idx][0] /= leftSum;
	RightPSF[d][idx][0] /= rightSum;
      }
    }

    // FFT
    planLeftPSF[d] = fftw_plan_dft_2d( height, width, LeftPSF[d] , LeftPSF[d], FFTW_FORWARD, FFTW_ESTIMATE);
    planRightPSF[d] = fftw_plan_dft_2d( height, width, RightPSF[d] , RightPSF[d], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( planLeftPSF[d] );
    fftw_execute( planRightPSF[d] );

  }// d

  printf("fft done\n");

  // ぼけ除去画像の作成
  fftw_complex* DblLeft[MAX_DISPARITY];
  fftw_complex* DblRight[MAX_DISPARITY];
  for( int d = 0; d < maxDisparity; ++d){
    DblLeft[d] = (fftw_complex*)fftw_malloc(memSize);
    DblRight[d] = (fftw_complex*)fftw_malloc(memSize);

    for( h = 0 ; h < height; ++h){
      for( w = 0; w < width; ++w){
	int idx = h * width + w ;
	double a, b, c, e;
	double snr = 0.002;

	a = Left[idx][0];
	b = Left[idx][1];
	c = LeftPSF[d][idx][0];
	e = LeftPSF[d][idx][1];
	DblLeft[d][idx][0] = ( a*c + b*e ) / ( c*c + e*e + snr );
	DblLeft[d][idx][1] = ( b*c - a*e ) / ( c*c + e*e + snr );

	a = Right[idx][0];
	b = Right[idx][1];
	c = RightPSF[d][idx][0];
	e = RightPSF[d][idx][1];
	DblRight[d][idx][0] = ( a*c + b*e ) / ( c*c + e*e + snr );
	DblRight[d][idx][1] = ( b*c - a*e ) / ( c*c + e*e + snr );

      }//w
    }//h
  }//d

  printf("deblurring done\n");
  
  Mat residualMap[MAX_DISPARITY];
  fftw_complex* rMapLeft = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex* rMapRight = (fftw_complex*)fftw_malloc(memSize);
  fftw_plan planRmapLeft = fftw_plan_dft_2d( height, width, rMapLeft, rMapLeft, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_plan planRmapRight = fftw_plan_dft_2d( height, width, rMapRight, rMapRight, FFTW_BACKWARD, FFTW_ESTIMATE);
 
  IMG* tmp1 = createImage( height, width);
  IMG* tmp2 = createImage( height, width);
  for( int d = 0; d < maxDisparity; ++d){
    residualMap[d] = matrixAlloc( width, height);
    
    for( h = 0 ; h < height; ++h ){
      for( w = 0 ; w < width; ++w ){
	int idx = h * width + w;
	double a,b,c,e;
	
	a = DblLeft[d][idx][0];
	b = DblLeft[d][idx][1];
	c = LeftPSF[d][idx][0];
	e = LeftPSF[d][idx][1];
	rMapLeft[idx][0] = a*c - b*e - Left[idx][0];
	rMapLeft[idx][1] = a*e + b*c - Left[idx][1];

	a = DblRight[d][idx][0];
	b = DblRight[d][idx][1];
	c = RightPSF[d][idx][0];
	e = RightPSF[d][idx][1];
	rMapRight[idx][0] = a*c - b*e - Right[idx][0];
	rMapRight[idx][1] = a*e + b*c - Right[idx][1];

      }
    }

    fftw_execute( planRmapLeft );
    fftw_execute( planRmapRight );

    for( h = 0 ; h < height; ++h ){
      for( w = 0 ; w < width; ++w ){
	int idx = h * width + w;
	double l = rMapLeft[idx][0] * rMapLeft[idx][0] + rMapLeft[idx][1] * rMapLeft[idx][1];
	double r = rMapRight[idx][0]*rMapRight[idx][0] + rMapRight[idx][1]*rMapRight[idx][1];
	ELEM0( residualMap[d], h, w) = sqrt(l) + sqrt(r);

      }
    }

  }//d 
  printf("compute residual map done\n");

  IMG* img = createImage( height, width);
  for(int d = 0; d < maxDisparity; ++d){
    convertMat2IMG( &(residualMap[d]), img);
    char filename[256];
    sprintf( filename, "img/MBP/110816-1/test/resmap%02d.png", d);
    saveImage( img, filename);
  }


  IMG* dst = createImage( height, width);
  for( h = 0 ; h < height; ++h ){
    for( w = 0; w < width; ++w ){
      
      double min = DBL_MAX;
      int disp;

      for(int d = 0; d < maxDisparity; ++d){
	double val = ELEM0( residualMap[d], h, w);
	if(val < min ){
	  min = val;
	  disp = d;
	}
      }

      IMG_ELEM( dst, h , w) = disp;

    }
  }

  return dst;



}
