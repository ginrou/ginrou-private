#include "depthEstimation.h"

// utility functions
void copySrc( const IMG* src, freq* dst)
{
  for( int h = 0; h < src->height; ++h){
    for( int w = 0 ;w < src->width; ++w){
      int idx = h * src->width + w;
      dst[idx][0] = (double)IMG_ELEM(src, h, w);
      dst[idx][1] = 0.0;
    }
  }
}

void copyDbl(freq* src, Mat dst)
{
  double scale = dst.row * dst.clm;
  for( int h = 0; h < dst.row; ++h){
    for( int w = 0; w < dst.clm; ++w){
      int idx = h * dst.clm + w;
      double val = src[idx][0]*src[idx][0] + src[idx][1]*src[idx][1];
      ELEM0( dst, h, w ) = sqrt( val ) / scale;
    }
  }
}

void wienerCalc(freq*  src,  freq* psf, freq* dbl, int size)
{
  for( int i = 0; i < size; ++i){
    double a = src[i][0];
    double b = src[i][1];
    double c = psf[i][0];
    double d = psf[i][1];
    
    dbl[i][0] = ( a*c + b*d ) / ( c*c + d*d + SNR );
    dbl[i][1] = ( b*c - a*d ) / ( c*c + d*d + SNR );
  }
}



// depth estimation functions

IMG* blurBaseEstimationIMG(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[])
{
  Mat result = blurBaseEstimationMat( left, right, psfLeft, psfRight );
  IMG* dst = createImage( result.row, result.clm);
  convertMat2IMG( &result, dst);
  matrixFree(result);
  return dst;
}


IMG* deblurBaseEstimationIMG(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[])
{
  Mat result = deblurBaseEstimationMat( left, right, psfLeft, psfRight );
  IMG* dst = createImage( result.row, result.clm);
  convertMat2IMG( &result, dst);
  matrixFree(result);
  return dst;  
}

IMG* deblurBaseEstimationIMGFreq(IMG* left, IMG* right, freq* psfLeft[], freq* psfRight[]){
  Mat result = deblurBaseEstimationMatFreq( left, right, psfLeft, psfRight );
  IMG *dst = createImage( result.row, result.clm );
  convertMat2IMG( &result, dst );
  matrixFree(result);
  return dst;
}

IMG* latentBaseEstimationIMG( IMG* left, IMG* right, freq* psfLeft[], freq* psfRight[])
{
  Mat result = latentBaseEstimationMat( left, right, psfLeft, psfRight );
  IMG* dst = createImage( result.row, result.clm);
  convertMat2IMG( &result, dst);
  matrixFree(result);
  return dst;
}


// Mat形式で渡されるPSFは全部0-1なので多分IMG形式に渡したら全部ゼロになる
// 一回255を全部の画素にかけないとねー
Mat blurBaseEstimationMat(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[])
{
  
  IMG *bluLeft[MAX_DISPARITY], *bluRight[MAX_DISPARITY];

  //blurring pat
  for( int d = 0; d < MAX_DISPARITY; ++d){

    IMG* psfLeftIn = createImage( psfLeft[d].row, psfLeft[d].clm );
    IMG* psfRightIn = createImage( psfRight[d].row, psfRight[d].clm );

    convertMat2IMG( &(psfLeft[d]), psfLeftIn );
    convertMat2IMG( &(psfRight[d]), psfRightIn );

    bluLeft[d] = blur( left, psfLeftIn );
    bluRight[d] = blur( right, psfRightIn );
    
    releaseImage( &psfLeftIn );
    releaseImage( &psfRightIn );

  }
  
  // depth Estimation Part
  Mat dst = matrixAlloc( left->height, left->width );
  for(int h = 0 ; h < dst.row; ++h){
    for(int w = 0 ; w < dst.clm; ++w){
      
      double min = DBL_MAX;
      double minDisp;
      
      for( int d = 0; d < MAX_DISPARITY; ++d){
	double resid = 0.f;
	
	for( int y = 0; y < WINDOW_SIZE; ++y){
	  for( int x = 0; x < WINDOW_SIZE; ++x){
	    if( y+h >= dst.row || x+w >= dst.clm) 
	      continue;
	    else {
	      double val = IMG_ELEM( bluLeft[d], h+y, w+x) - IMG_ELEM( bluRight[d], h+y, w+x );
	      resid += val * val;
	    }
	  }
	}

	if( resid < min ){
	  min = resid;
	  minDisp = d ;
	}

      }//d
      
      ELEM0( dst, h, w) = minDisp;

    }//w
  }//h

  for(int d = 0; d < MAX_DISPARITY; ++d){
    releaseImage( &(bluLeft[d]) );
    releaseImage( &(bluRight[d]) );
  }

  return dst;

}


Mat deblurBaseEstimationMat(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[])
{
  int h, w;
  int height = left->height;
  int width = left->width;
  Mat dblLeft[MAX_DISPARITY], dblRight[MAX_DISPARITY];
  
  // set up of FFTW
  size_t memSize = sizeof(freq) * ( left->height * left->width );
  freq *img = (freq*) fftw_malloc ( memSize );
  freq *filter = (freq*) fftw_malloc ( memSize );
  freq *dbl = (freq*)fftw_malloc ( memSize );
  fftw_plan srcFFTPlan = fftw_plan_dft_2d( height, width, img, img , 
					   FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan psfFFTPlan = fftw_plan_dft_2d( height, width,filter, filter, 
					   FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan dblFFTPlan = fftw_plan_dft_2d( height, width, dbl, dbl, 
					   FFTW_BACKWARD, FFTW_ESTIMATE);

  
  // deblur part
  for(int d = MIN_DISPARITY ; d < MAX_DISPARITY; ++d){
    
    // left image
    // copy
    copySrc( left, img); 
    PSFCopyForFFTW( psfLeft[d], filter, Point(width, height) ); 
    // FFT
    fftw_execute( srcFFTPlan );
    fftw_execute( psfFFTPlan );
    // deconvolution
    wienerCalc( img, filter, dbl, height * width );
    // IDFT
    fftw_execute( dblFFTPlan );
    // copy deblurred image
    dblLeft[d] = matrixAlloc( height, width);
    copyDbl( dbl, dblLeft[d] );


    // right image
    // copy src and psf : Mat or IMG -> fftw
    copySrc( right, img);
    PSFCopyForFFTW( psfRight[d], filter, Point(width, height) );
    // FFT of src and psf
    fftw_execute( srcFFTPlan );
    fftw_execute( psfFFTPlan );
    // deconvolution
    wienerCalc( img, filter, dbl, height*width);
    // IDFT of dbl
    fftw_execute( dblFFTPlan );
    // copy deblurred image : fftw -> Mat
    dblRight[d] = matrixAlloc( height, width );
    copyDbl( dbl, dblRight[d] );

  }


  // depth estimation part
  Mat dst = matrixAlloc( height, width );
  for( h = 0; h < height; ++h){
    for( w = 0 ; w < width; ++w){
      
      double min = DBL_MAX;
      double minDisp, val;

      for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){
	double resid = 0.f;
	
	for( int y = 0 ; y < WINDOW_SIZE; ++y){
	  for( int x = 0 ; x < WINDOW_SIZE; ++x){
	    if( y+h >= height || x+w >= width )
	      continue;
	    else{
	      val = ELEM0( dblLeft[d], h+y, w+x) - ELEM0( dblRight[d], h+y, w+x );
	      resid += val * val;
	    }
	  }// x
	}// y
	if( resid < min ){
	  min = resid;
	  minDisp = d;
	}
      }// d
      ELEM0( dst, h, w) = minDisp;
    }// w
  }// h

  // clean up
  for( int d = 0; d < MAX_DISPARITY; ++d){
    matrixFree( dblLeft[d] );
    matrixFree( dblRight[d] );
  }

  fftw_free( img );
  fftw_free( filter );
  fftw_free( dbl );
  fftw_destroy_plan( srcFFTPlan );
  fftw_destroy_plan( psfFFTPlan );
  fftw_destroy_plan( dblFFTPlan );
  return dst;
}

Mat deblurBaseEstimationMatFreq(IMG* left, IMG* right, freq* psfLeft[], freq* psfRight[])
{
  int h, w;
  int height = left->height;
  int width = left->width;
  size_t memSize = sizeof( freq ) * height * width ;
  freq *capLeft  = (freq*)fftw_malloc( memSize );
  freq *capRight = (freq*)fftw_malloc( memSize ); 
  freq* tmpDbl   = (freq*)fftw_malloc( memSize ); 
  Mat dblLeft[MAX_DISPARITY], dblRight[MAX_DISPARITY];


  // FFT of captured images
  copySrc( left, capLeft );
  copySrc( right, capRight );
  fftw_plan capLeftPlan = fftw_plan_dft_2d( height, width, capLeft, capLeft,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan capRightPlan = fftw_plan_dft_2d( height, width, capRight, capRight,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( capLeftPlan );
  fftw_execute( capRightPlan );
  fftw_destroy_plan( capLeftPlan );
  fftw_destroy_plan( capRightPlan );

  // compute deblurred image
  fftw_plan dblPlan = fftw_plan_dft_2d( height, width, tmpDbl, tmpDbl, 
					FFTW_BACKWARD, FFTW_ESTIMATE);
  for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){

    wienerCalc( capLeft, psfLeft[d], tmpDbl, height*width );
    fftw_execute( dblPlan);
    dblLeft[d] = matrixAlloc( height, width );
    copyDbl( tmpDbl, dblLeft[d] );

    wienerCalc( capRight, psfRight[d], tmpDbl, height*width );
    fftw_execute( dblPlan );
    dblRight[d] = matrixAlloc( height, width );
    copyDbl( tmpDbl, dblRight[d] );
  }

  // debug : save image
  if( saveDebugImages == YES ){
    char filename[256];
    IMG* img = createImage( height, width);
    for(int d = 0; d < MAX_DISPARITY; ++d){
      convertMat2IMG( &dblLeft[d], img );
      sprintf( filename, "%s/deblur-left%02d.png", tmpImagesDir, d );
      saveImage( img, filename );

      convertMat2IMG( &dblRight[d], img );
      sprintf( filename, "%s/deblur-right%02d.png", tmpImagesDir, d );
      saveImage( img, filename );
    }
    releaseImage( &img );
  }

  // depth estimation 
  Mat dst = matrixAlloc( height, width );
  for( h = 0; h < height; ++h){
    for( w = 0; w < width; ++w){
      double min = DBL_MAX;
      double minDisp, val;

      for(int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){
	double resid = 0.0;

	for( int y = 0; y < WINDOW_SIZE; ++y){
	  for( int x = 0; x < WINDOW_SIZE; ++x){
	    if( y+h >= height || w+x >= width ){ 
	      continue;
	    }else{
	      val = ELEM0( dblLeft[d], h+y, w+x ) - ELEM0( dblRight[d], h+y, w+x );
	      resid += val*val;
	    }// else
	  }// x
	}//y
	if( resid < min ){
	  min = resid; minDisp = d;
	}
      }//d

      ELEM0( dst, h, w ) = minDisp;

    }// w
  }//h

  // clean up
  for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){
    matrixFree( dblLeft[d] );
    matrixFree( dblRight[d] );
  }

  fftw_free( capLeft );
  fftw_free( capRight );
  fftw_free( tmpDbl );
  fftw_destroy_plan( dblPlan );
  return dst;
}



Mat latentBaseEstimationMat( IMG* left, IMG* right, freq* psfLeft[], freq* psfRight[])
{
  int h, w;
  int height = left->height;
  int width  = left->width;

  size_t memSize = sizeof( freq ) * height * width ;
  freq *capLeft = (freq*)fftw_malloc( memSize );
  freq *capRight = (freq*)fftw_malloc( memSize ); 
  freq *latent[MAX_DISPARITY];

  // FFT of captured images
  copySrc( left, capLeft );
  copySrc( right, capRight );
  fftw_plan capLeftPlan = fftw_plan_dft_2d( height, width, capLeft, capLeft,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan capRightPlan = fftw_plan_dft_2d( height, width, capRight, capRight,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( capLeftPlan );
  fftw_execute( capRightPlan );
  fftw_destroy_plan( capLeftPlan );
  fftw_destroy_plan( capRightPlan );

  // compute latent image
  for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){

    // compute latent image
    latent[d] = (freq*)fftw_malloc( memSize );
    for( int i = 0; i < height * width; ++i){

      double fLr = psfLeft[d][i][0];
      double fLi = psfLeft[d][i][1];
      double fRr = psfRight[d][i][0];
      double fRi = psfRight[d][i][1];
      double yLr = capLeft[i][0];
      double yLi = capLeft[i][1];
      double yRr = capRight[i][0];
      double yRi = capRight[i][1];

      double denom = fLr*fLr + fLi*fLi + fRr*fRr + fRi*fRi + SNR ;
      latent[d][i][0] = ( fLr*yLr + fLi*yLi + fRr*yRr + fRi*yRi ) / denom ;
      latent[d][i][1] = ( fLr*yLi - fLi*yLr + fRr*yRi - fRi*yRr ) / denom ;
    }

    // debug : save latent image
    if(saveDebugImages == YES){
      char filename[256];
      freq *debugRegion = (freq*)fftw_malloc( memSize );
      fftw_plan planDebug = fftw_plan_dft_2d( height, width, latent[d], debugRegion, FFTW_BACKWARD, FFTW_ESTIMATE);
      fftw_execute( planDebug );
      IMG* debugImage = createImage( height, width );
      double scale = height*width;
      for( h = 0; h < height; ++h){
	for( w = 0 ; w < width; ++w){
	  int i = h*width+w;
	  double hoge = debugRegion[i][0]*debugRegion[i][0] + debugRegion[i][1]*debugRegion[i][1];
	  hoge = sqrt(hoge)/scale;
	  if( hoge < 0 ) hoge = 0.0;
	  else if( hoge > 255 ) hoge = 255.0;
	  IMG_ELEM( debugImage, h, w) = hoge;
	}
      }
      sprintf(filename, "%s/latent%02d.png", tmpImagesDir, d);
      saveImage( debugImage, filename );
    }
    // debug zone end

  }

  // compute residual and estiamte depth
  Mat residMap = matrixAlloc( height, width);
  freq *residMapLeft = (freq*)fftw_malloc( memSize );
  freq *residMapRight = (freq*)fftw_malloc( memSize );
  fftw_plan residLeftPlan = fftw_plan_dft_2d( height, width, 
					      residMapLeft, residMapLeft,
					      FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_plan residRightPlan = fftw_plan_dft_2d( height, width, 
					      residMapRight, residMapRight,
					      FFTW_BACKWARD, FFTW_ESTIMATE);
  
  // allocate dst
  Mat dst = matrixAlloc( height, width);
  Mat minMap = matrixAlloc( height, width);
  for( h = 0 ; h < height ; ++h){
    for( w = 0 ; w < width; ++w){
      ELEM0( minMap, h, w) = DBL_MAX;
    }
  }


  // loop of disparity
  for( int d = MIN_DISPARITY ; d < MAX_DISPARITY; ++d){
    
    // calc resid
    for( int i = 0; i < height*width ;++i){
      double xr = latent[d][i][0];
      double xi = latent[d][i][1];
      double fLr = psfLeft[d][i][0];
      double fLi = psfLeft[d][i][1];
      double fRr = psfRight[d][i][0];
      double fRi = psfRight[d][i][1];
      double yLr = capLeft[i][0];
      double yLi = capLeft[i][1];
      double yRr = capRight[i][0];
      double yRi = capRight[i][1];

      residMapLeft[i][0] = xr*fLr - xi*fLi - yLr;
      residMapLeft[i][1] = xi*fLr + xr*fLi - yLi;
      residMapRight[i][0] = xr*fRr - xi*fRi - yRr;
      residMapRight[i][1] = xi*fRr + xr*fRi - yRi;

    }

    // IFFT of resid map
    fftw_execute( residLeftPlan );
    fftw_execute( residRightPlan );
    
    // resid map
    for( h = 0; h < height; ++h){
      for( w = 0; w < width; ++w){
	int i = h * width + w;
	double scale = height * width ;
	double val;

	val = residMapLeft[i][0]*residMapLeft[i][0] + residMapLeft[i][1]*residMapLeft[i][1];
	double residLeft = sqrt(val*val) / scale;

	val = residMapRight[i][0]*residMapRight[i][0] + residMapRight[i][1]*residMapRight[i][1];
	double residRight = sqrt( val*val ) / scale;

	ELEM0( residMap, h, w ) = residLeft + residRight;
      }//w
    }//h

    for( h = 0; h < height; ++h){
      for( w = 0; w < width; ++w){
	double sum = 0.0, val;
	for( int y = -WINDOW_SIZE/2 ; y < WINDOW_SIZE/2; ++y){
	  for( int x = -WINDOW_SIZE/2; x < WINDOW_SIZE/2; ++x){
	    if(y+h < 0 || y+h >= height || 
	       x+w < 0 || x+w >= width ) continue;
	    else{
	      sum += ELEM0( residMap, h+y, w+x);
	    }
	  }// x
	}// y 

	if( sum < ELEM0( minMap, h, w) ){
	  ELEM0( minMap, h, w) = sum;
	  ELEM0( dst, h , w) = d;
	}

      }// w
    }// h


    //estimated latent image
    fftw_free( latent[d] );

  }//d

  // finish calculating
  
  // clean up
  fftw_destroy_plan( residLeftPlan );
  fftw_destroy_plan( residRightPlan );  
  fftw_free( residMapLeft );
  fftw_free( residMapRight );


  return dst;
  
}




void deblurBaseEstimationFreqDebugOnly( IMG*left, IMG* right, freq* psfLeft[], freq* psfRight[] )
{
  int h, w;
  int height = left->height;
  int width = left->width;
  size_t memSize = sizeof( freq ) * height * width ;
  freq *capLeft  = (freq*)fftw_malloc( memSize );
  freq *capRight = (freq*)fftw_malloc( memSize ); 
  freq* tmpDbl   = (freq*)fftw_malloc( memSize ); 
  Mat dblLeft[MAX_DISPARITY], dblRight[MAX_DISPARITY];


  // FFT of captured images
  copySrc( left, capLeft );
  copySrc( right, capRight );
  fftw_plan capLeftPlan = fftw_plan_dft_2d( height, width, capLeft, capLeft,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan capRightPlan = fftw_plan_dft_2d( height, width, capRight, capRight,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( capLeftPlan );
  fftw_execute( capRightPlan );
  fftw_destroy_plan( capLeftPlan );
  fftw_destroy_plan( capRightPlan );

  // compute deblurred image
  fftw_plan dblPlan = fftw_plan_dft_2d( height, width, tmpDbl, tmpDbl, 
					FFTW_BACKWARD, FFTW_ESTIMATE);
  for( int d = 0; d < MAX_DISPARITY; ++d){

    wienerCalc( capLeft, psfLeft[d], tmpDbl, height*width );
    fftw_execute( dblPlan);
    dblLeft[d] = matrixAlloc( height, width );
    copyDbl( tmpDbl, dblLeft[d] );

    wienerCalc( capRight, psfRight[d], tmpDbl, height*width );
    fftw_execute( dblPlan );
    dblRight[d] = matrixAlloc( height, width );
    copyDbl( tmpDbl, dblRight[d] );
  }

  // debug : save image
  if( saveDebugImages == YES ){
    char filename[256];
    IMG* img = createImage( height, width);
    for(int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){
      convertMat2IMG( &dblLeft[d], img );
      sprintf( filename, "%s/deblur-left%02d.png", tmpImagesDir, d );
      saveImage( img, filename );

      convertMat2IMG( &dblRight[d], img );
      sprintf( filename, "%s/deblur-right%02d.png", tmpImagesDir, d );
      saveImage( img, filename );
    }
    releaseImage( &img );
  }

}
