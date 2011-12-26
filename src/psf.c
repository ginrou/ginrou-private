#include "psf.h"

// private functions
void makeBlurPSFMatFreq( IMG* aperture, double param[2], Mat dst[MAX_DISPARITY],
			 point imgSize, int maxDepth);

void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam){
  
  for(int disp = 0; disp < MAX_DISPARITY; ++disp){
    psf[disp] = matrixAlloc( 1, MAX_DISPARITY);

    for( int y = 0 ; y < psf[disp].row; ++y){
      for( int x = 0 ; x < psf[disp].clm; ++x){
	ELEM0( psf[disp] , y, x ) = 0.0;
      }
    }

    if( cam == LEFT_CAM ){
      ELEM0( psf[disp], 0, MAX_DISPARITY/2 - disp/2) = 1.0;
    }else if( cam == RIGHT_CAM ){
      ELEM0( psf[disp], 0, MAX_DISPARITY/2 + disp/2) = 1.0;
    }

  }
  return;
}

void makeBlurPSF( IMG* psf[MAX_DISPARITY], 
		  IMG* aperture,
		  int maxDepth,
		  double param[2])
{

  for( int i = 0; i < maxDepth; ++i){
    int size = (double)i * param[0] + param[1];
    if( size == 0 ) size = 1;

    psf[i] = createImage( abs(size), abs(size) );
    resizeImage( aperture, psf[i]);
    if( size < 0 ) flipImage( psf[i], 1, 1);
    
  }

  return;
}

void makeBlurPSFMatFreq(IMG* aperture, double param[2], Mat dst[MAX_DISPARITY],
			point imgSize, int maxDepth)
{
  int h, w;
  int height = imgSize.y;
  int width =  imgSize.x;
  for(int disp = 0; disp < maxDepth; ++disp){

    // size
    double size = fabs( (double)disp * param[0] + param[1] );
    if(size < 1.0) size = 1.0;

    // copy apeture -> tmp1, tmp2 
    IMG* tmp1 = createImage( (int)size, (int)size );
    IMG* tmp2 = createImage( (int)size+1, (int)size+1 );
    resizeImage( aperture, tmp1 );
    resizeImage( aperture, tmp2 );
    if( (double)disp * param[0] + param[1]  < 0.0 ){
      flipImage( tmp1, 1, 1);
      flipImage( tmp2, 1, 1);
    }

    // copy to dst
    dst[disp] = matrixAlloc( height, width);
    double r = size - (int)size;
    for( h = 0 ; h < height; ++h){
      for( w = 0; w < width; ++w){
	ELEM0( dst[disp], h , w) = 0.0;
      }
    }

    for( h = 0 ; h < tmp1->height; ++h){
      for( w = 0; w < tmp1->width; ++w){
	ELEM0( dst[disp], h , w) += (1-r)*(double)IMG_ELEM( tmp1, h, w);
      }
    }

    for( h = 0 ; h < tmp2->height; ++h){
      for( w = 0; w < tmp2->width; ++w){
	ELEM0( dst[disp], h , w) += r*(double)IMG_ELEM( tmp2, h, w);
      }
    }

    // normalize
    PSFNormalize( dst[disp] );
    releaseImage( &tmp1 );
    releaseImage( &tmp2 );
  }

  return;

}


void makeBlurPSFFreq( IMG* aperture, double param[2], freq* dst[MAX_DISPARITY],
		     point imgSize, int maxDepth)
{
  int h, w;
  int height = imgSize.y;
  int width = imgSize.x;
  size_t memSize = sizeof( freq ) * height * width;
  
  freq* img1 = (freq*)fftw_malloc( memSize);
  freq* img2 = (freq*)fftw_malloc( memSize);
  fftw_plan plan1 = fftw_plan_dft_2d( height, width, img1, img1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan plan2 = fftw_plan_dft_2d( height, width, img2, img2, FFTW_FORWARD, FFTW_ESTIMATE);  
  for( int d = 0; d < maxDepth; ++d){
    //size
    double size = fabs( (double)d * param[0] + param[1] );
    if(size < 1.0 ) size = 1.0;

    // copy aperture to IMG1, IMG2 and resize
    IMG *tmp1 = createImage( (int)size, (int)size );
    IMG *tmp2 = createImage( (int)size + 1, (int)size + 1 );
    resizeImage( aperture, tmp1);
    resizeImage( aperture, tmp2);
    if( (double)d * param[0] + param[1] < 0.0 ){
      flipImage( tmp1, 1, 1);
      flipImage( tmp2, 1, 1);
    }

    printf("disp = %d, psf size = %lf\n", d, (double)d * param[0] + param[1]);
    
    // copy to FFTW
    Mat mat1 = cloneMatFromImage(tmp1);
    Mat mat2 = cloneMatFromImage(tmp2);    
    PSFNormalize(mat1);
    PSFNormalize(mat2);
    PSFCopyForFFTW( mat1, img1, imgSize ); 
    PSFCopyForFFTW( mat2, img2, imgSize ); 
    
    // FFT
    fftw_execute( plan1 );
    fftw_execute( plan2 );
    
    // merge
    double r = size - (int)size;
    dst[d] = (freq*)fftw_malloc( memSize );
    for( int i = 0 ; i < height* width ; ++i){
      dst[d][i][0] = (1-r) * img1[i][0] + r * img2[i][0];
      dst[d][i][1] = (1-r) * img1[i][1] + r * img2[i][1];
    }

    //clean up
    matrixFree(mat1);
    matrixFree(mat2);
    releaseImage(&tmp1);
    releaseImage(&tmp2);

  }

  fftw_destroy_plan( plan1 );
  fftw_destroy_plan( plan2 );
  fftw_free( img1 );
  fftw_free( img2 );
  return;
}


void makeBlurPSFMat( IMG* aperture, double param[2], Mat dst[MAX_DISPARITY],
			 point imgSize, int maxDepth)
{
  int h ,w;
  int height = imgSize.y;
  int width  = imgSize.x;
  size_t memSize = sizeof( freq ) * height * width ;

  freq* img1 = (freq*)fftw_malloc( memSize );
  freq* img2 = (freq*)fftw_malloc( memSize );
  freq* result  = (freq*)fftw_malloc( memSize );
  fftw_plan plan1 = fftw_plan_dft_2d( height, width, img1, img1, FFTW_FORWARD, FFTW_ESTIMATE );
  fftw_plan plan2 = fftw_plan_dft_2d( height, width, img2, img2, FFTW_FORWARD, FFTW_ESTIMATE );
  fftw_plan planResult = fftw_plan_dft_2d( height, width, result, result, FFTW_BACKWARD, FFTW_ESTIMATE );
  
  for( int disp = 0; disp < MAX_DISPARITY; ++disp){
    
    // size 
    double size = fabs( (double)disp * param[0] + param[1] );
    if( size < 1.0 ) size = 1.0;

    
    // copy aperture to IMG
    IMG *tmp1 = createImage( (int)size, (int)size );
    IMG *tmp2 = createImage( (int)size + 1, (int)size + 1 );
    resizeImage( aperture, tmp1);
    resizeImage( aperture, tmp2);
    if( (double)disp * param[0] + param[1] < 0.0 ){
      flipImage( tmp1, 1, 1);
      flipImage( tmp2, 1, 1);
    }

    printf("copy\n");

    // copy to mat and pass to img1, img2
    Mat mat1 = matrixAlloc( tmp1->height, tmp1->width);
    Mat mat2 = matrixAlloc( tmp2->height, tmp2->width);
    
    convertIMG2Mat( tmp1, &mat1 ); //convert
    convertIMG2Mat( tmp2, &mat2 );

    PSFNormalize( mat1 ); // normalize
    PSFNormalize( mat2 );

    PSFCopyForFFTW( mat1, img1, imgSize ); // copy to fftw
    PSFCopyForFFTW( mat2, img2, imgSize );

    matrixFree( mat1 ); // cleaning
    matrixFree( mat2 );
    releaseImage( &tmp1 );
    releaseImage( &tmp2 );

    // FFT
    fftw_execute( plan1 );
    fftw_execute( plan2 );

    printf("FFT\n");

    // merge img1 and img2 as linear interpolation
    double r = size - (int)size;
    for( int i = 0 ; i < height * width ; ++i){
      result[i][0] = (1-r) * img1[i][0] + r * img2[i][0];
      result[i][1] = (1-r) * img1[i][1] + r * img2[i][1];
    }

    printf("merge\n");

    // IDFT
    fftw_execute( planResult );

    printf("IDFT\n");

    // copy result to dst
    dst[disp] = matrixAlloc( height, width );

    for( h = 0 ; h < height; ++h){
      for( w = 0 ; w < width; ++w ){
	int idx = h*width+w;
	double scale = height*width;
	double val = result[idx][0] *result[idx][0] + result[idx][1] *result[idx][1];
	ELEM0( dst[disp], h, w) = sqrt(val)/scale;
      }
    }


    printf("result\n");

  }// d
  
  fftw_free( img1 );
  fftw_free( img2 );
  fftw_free( result );
  fftw_destroy_plan( plan1 );
  fftw_destroy_plan( plan2 );
  fftw_destroy_plan( planResult );


  return;
}




void makeShiftBlurPSF( Mat psf[MAX_DISPARITY], int cam,
		       IMG* aperture, double par[2])
{

  for( int disp = 0; disp < MAX_DISPARITY; ++disp){
    double size = (double)disp * par[0] + par[1];
    int sz = abs(size);
    if(sz == 0 )sz = 1;

    IMG* img = createImage( sz, sz );
    resizeImage( aperture, img);
    if(size > 0) flipImage( img, 1, 1);

    psf[disp] = matrixAlloc( sz, MAX_DISPARITY + sz );

    // PSFの中央を決める
    int center;
    if( cam == LEFT_CAM ){
      center = ( MAX_DISPARITY + sz - disp ) / 2 ;
    }else if( cam == RIGHT_CAM ){
      center = ( MAX_DISPARITY + sz + disp ) / 2 ;
    }

    // PSFを埋めて行く
    matrixZero( psf[disp] );
    for(int y = 0; y < sz ; ++y){
      for( int x = 0; x < sz ; ++x){
	int p = center - x + sz/2;
	if( p < 0 || p >= psf[disp].clm ) continue;

	ELEM0( psf[disp], y, p) = IMG_ELEM( img, y, x);

      }
    }
      
    releaseImage(&img);
  }
}

void makeShiftBlurPSFFreq( int height, int width, int cam,
			   freq* dst[MAX_DISPARITY],
			   IMG* aperture, double param[2] )
{
  int h, w;
  size_t memSize = sizeof( freq ) * height * width ;
  freq* tmp1 = (freq*)fftw_malloc(memSize);
  freq* tmp2 = (freq*)fftw_malloc(memSize);
  fftw_plan plan1 = fftw_plan_dft_2d( height, width, tmp1, tmp1, FFTW_FORWARD, FFTW_ESTIMATE );
  fftw_plan plan2 = fftw_plan_dft_2d( height, width, tmp2, tmp2, FFTW_FORWARD, FFTW_ESTIMATE );

  for( int disp = 0; disp < MAX_DISPARITY; ++disp){
    
    // size of psf
    double size = fabs( (double)disp * param[0] + param[1] );
    if( size < 1.0 ){ size = 1.0;}

    // copy image & resize
    IMG* img1 = createImage( (int)size, (int)size );
    IMG* img2 = createImage( (int)size+1, (int)size+1 );
    resizeImage( aperture, img1 );
    resizeImage( aperture, img2 );
    
    // flip
    if( cam == LEFT_CAM ){
      flipImage( img1, 0, 1);
      flipImage( img2, 0, 1);
    }else{
      flipImage( img1, 1, 0);
      flipImage( img2, 1, 0);
    }
    if( (double)disp * param[0] + param[1] < 0.0 ){
      flipImage( img1, 1, 1);
      flipImage( img2, 1, 1);
    }

    printf("disp = %d, imgsize = %d, psf size = %lf\n", disp, (int)size, (double)disp * param[0] + param[1]);
    
    // fill to tmp
    for(int i = 0; i < height * width ; ++ i){
      tmp1[i][0] = 0.0;
      tmp1[i][1] = 0.0;
      tmp2[i][0] = 0.0;
      tmp2[i][1] = 0.0;
    }

    // 小さい方
    double norm = imageNormL1( img1 );
    int center;
    if( cam == LEFT_CAM ) 
      center = ( MAX_DISPARITY + img1->width - disp ) / 2;
    else 
      center = ( MAX_DISPARITY + img1->width + disp ) / 2;
    for( h = 0; h < img1->height; ++h){
      for( w = 0 ; w < img1->width; ++w){
	int y = h - img1->height/2;
	int x = center - w ;
	y += (y<0) ? height : 0 ;
	x += (x<0) ? width  : 0 ; 
	int idx = y * width + x;
	tmp1[idx][0] = DBL_ELEM( img1, h ,w) / norm ;
      }
    }

    // 大きい方
    norm = imageNormL1( img2 );
    if( cam == LEFT_CAM ) 
      center = ( MAX_DISPARITY + img2->width - disp ) / 2;
    else 
      center = ( MAX_DISPARITY + img2->width + disp ) / 2;
    for( h = 0; h < img2->height; ++h){
      for( w = 0 ; w < img2->width; ++w){
	int y = h - img2->height/2;
	int x = center - w ;
	y += (y<0) ? height : 0 ;
	x += (x<0) ? width  : 0 ; 
	int idx = y * width + x;
	tmp2[idx][0] = DBL_ELEM( img2, h ,w) / norm ;
      }
    }

    // FFT
    fftw_execute( plan1 );
    fftw_execute( plan2 );

    //merge
    double r = size - (int)size;

    dst[disp] = (freq*)fftw_malloc( memSize );
    for( int i = 0; i < height * width ; ++i){
      dst[disp][i][0] = (1-r) * tmp1[i][0] + r * tmp2[i][0];
      dst[disp][i][1] = (1-r) * tmp1[i][1] + r * tmp2[i][1];
    }

    releaseImage( &img1);
    releaseImage( &img2);

  }

  fftw_destroy_plan(plan1);
  fftw_destroy_plan(plan2);
  fftw_free(tmp1);
  fftw_free(tmp2);

  return;

}




// 1/2ずつの視差をうまく表現できないから
// 画像を二倍して扱うやつ
void makeShiftBlurPSFFreq2x( int height, int width, int cam,
			   freq* dst[MAX_DISPARITY],
			   IMG* aperture, double param[2] )
{

  int h, w;
  height *= 2;
  width *= 2;

  size_t memSize = sizeof(freq) * height * width;
  freq* tmp1 = (freq*)fftw_malloc( memSize );
  freq* tmp2 = (freq*)fftw_malloc( memSize );
  freq* tmpDst = (freq*)fftw_malloc( memSize );
  fftw_plan plan1 = fftw_plan_dft_2d( height, width, tmp1, tmp1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan plan2 = fftw_plan_dft_2d( height, width, tmp2, tmp2, FFTW_FORWARD, FFTW_ESTIMATE);

  for( int disp = MIN_DISPARITY-1; disp < MAX_DISPARITY; ++disp){

    // size of psf at disparity disp
    double size = 2.0 * fabs( (double)disp * param[0] + param[1] );
    if( size < 1.0 ) size = 1.0;

    IMG* img1 = createImage( (int)size,   (int)size  );
    IMG* img2 = createImage( (int)size+1, (int)size+1);
    resizeImage( aperture, img1 );
    resizeImage( aperture, img2 );

    // flip
    if( cam == LEFT_CAM ){
      flipImage( img1, 0, 1);
      flipImage( img2, 0, 1);
    }else{
      flipImage( img1, 1, 0);
      flipImage( img2, 1, 0);
    }
    
    if( (double)disp * param[0] + param[1] < 0.0 ){
      flipImage( img1, 1, 1);
      flipImage( img2, 1, 1);
    }
    
    printf("disparity = %d, size = %lf\n", disp, size/2.0);

    // fill to tmp
    // set value to zero
    for( int i = 0 ; i < height * width ; ++i){
      tmp1[i][0] = 0.0;
      tmp1[i][1] = 0.0;
      tmp2[i][0] = 0.0;
      tmp2[i][1] = 0.0;
    }

    // tmp1
    double norm = imageNormL1( img1 );
    int center;
    if( cam == LEFT_CAM)
      center = -MAX_DISPARITY/2 + disp + img1->width/2;
    else
      center =  MAX_DISPARITY/2 - disp + img1->width/2;
    for( h = 0; h < img1->height; ++h){
      for( w = 0; w < img1->width; ++w){
	int y = h - img1->height/2;
	int x = w - center;
	y += ( y<0 ) ? height : 0;
	x += ( x<0 ) ? width  : 0;
	int idx = y *width + x;
	tmp1[idx][0] = DBL_ELEM( img1, h, w ) / norm;
      }
    }

    // tmp2
    norm = imageNormL1( img2 );
    center;
    if( cam == LEFT_CAM)
      center = -MAX_DISPARITY/2 + disp + img2->width/2;
    else
      center =  MAX_DISPARITY/2 - disp + img2->width/2;
    for( h = 0; h < img2->height; ++h){
      for( w = 0; w < img2->width; ++w){
	int y = h - img2->height/2;
	int x = w - center;
	y += ( y<0 ) ? height : 0;
	x += ( x<0 ) ? width  : 0;
	int idx = y *width + x;
	tmp2[idx][0] = DBL_ELEM( img2, h, w ) / norm;
      }
    }
    
    // FFT
    fftw_execute( plan1 );
    fftw_execute( plan2 );

    // merge 
    double r = size -(int)size;
    for( int i = 0; i < height*width; ++i ){
      tmpDst[i][0] = (1.0-r) * tmp1[i][0] + r * tmp2[i][0];
      tmpDst[i][1] = (1.0-r) * tmp1[i][1] + r * tmp2[i][1];
    }

    // resize to dst
    dst[disp] = (freq*)fftw_malloc( memSize / 4.0 );
    for( h = 0; h < height/2; ++h ){
      for( w = 0; w < width/2; ++w){
	int idx[4] = { (2*h  ) * width + 2*w,
		       (2*h  ) * width + 2*w+1,
		       (2*h+1) * width + 2*w,
		       (2*h+1) * width + 2*w+1};
	double re = 0.0, im = 0.0;
	for(int i = 0; i < 4; ++i){
	  re += tmpDst[idx[i]][0];
	  im += tmpDst[idx[i]][1];
	}

	dst[disp][ h*width/2 + w][0] = re / 4.0;
	dst[disp][ h*width/2 + w][1] = im / 4.0;

      }
    }

    // clean up
    releaseImage( &img1 );
    releaseImage( &img2 );
  }

  fftw_destroy_plan(plan1);
  fftw_destroy_plan(plan2);
  fftw_free( tmp1 );
  fftw_free( tmp2 );

  return;
}


void PSFCopyForFFTW( const Mat src, freq *dst , point size)
{
  // set all element to zero
  for( int i = 0; i < size.y * size.x; ++i) {
    dst[i][0] = 0.0;
    dst[i][1] = 0.0;
  }
  // copy
  for( int h = 0; h < src.row; ++h){
    for( int w = 0 ; w < src.clm; ++w){
      int y = h - src.row / 2;
      int x = w - src.clm / 2;
      y += ( y < 0 ) ? size.y : 0 ;
      x += ( x < 0 ) ? size.x : 0 ;
      int idx = y * size.x + x;
      dst[idx][0]  = ELEM0( src, h, w);
    }
  }

}



void PSFNormalize( Mat psf )
{
  normalizeMat( psf, psf);
}

Mat PSFCutoffZeroRegion( Mat src){
  if( src.row == 0 || src.clm == 0) return src;
  int horizontalEdge = 0;
  int vertcialEdge = 0;
  int h, w;

  for( h = 0; h < src.row; ++h){
    for( w = 0 ;w < src.clm; ++w){
      if( ELEM0( src, h, w) > DBL_MIN ){
	if( horizontalEdge < w ) horizontalEdge = w;
	if( vertcialEdge < h )	vertcialEdge = h;
      }
    }
  }

  if( vertcialEdge == 0 ) vertcialEdge = 1;
  if( horizontalEdge == 0 ) horizontalEdge = 1;

  Mat dst = matrixAlloc( vertcialEdge, horizontalEdge );
  for( h = 0; h < dst.row; ++h){
    for( w = 0; w < dst.clm; ++w){
      ELEM0( dst, h, w) = ELEM0( src, h, w);
    }
  }

  PSFNormalize(dst);
  return dst;
}



int numOfNonZero( Mat mat ){
  int count = 0;
  for( int r = 0; r < mat.row; ++r){
    for( int c = 0; c < mat.clm; ++c){
      if( ELEM0(mat, r, c) > 0.00001 ) count++;
    }
  }
  return count;
}


void PSFSaveForDebug( freq* psf[], int height, int width,
		      int minDisparity, int maxDisparity, char fileappend[])
{
  int memSize = sizeof(freq)*height*width;
  int h, w;

  freq* tmp = (freq*)fftw_malloc( memSize);
  IMG* img = createImage( height, width );

  for(int d = minDisparity; d < maxDisparity; ++d ){
    fftw_plan plan = fftw_plan_dft_2d( height, width, psf[d], tmp, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute( plan );
    double scale = height*width;
    for(h=0;h<height;++h){
      for(w=0;w<width;++w){
	int idx = h*width+w;
	double val = tmp[idx][0]*tmp[idx][0] + tmp[idx][1]*tmp[idx][1];
	val = 1000000.0 * sqrt(val) / scale;
	if( val < 0 ) val = 0.0;
	else if ( val > 255 ) val = 255;
	IMG_ELEM( img, h, w) = val;
      }
    }

    char filename[256];
    sprintf(filename, "%s/%s%02d.png", tmpImagesDir, fileappend, d);
    saveImage( img, filename);

    fftw_destroy_plan( plan );
  }
  releaseImage( &img );
  fftw_free( tmp );
}





