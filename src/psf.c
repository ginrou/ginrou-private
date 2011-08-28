#include "psf.h"

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
			   fftw_complex* dst[MAX_DISPARITY],
			   IMG* aperture, double param[2] )
{
  int h, w;
  size_t memSize = sizeof( fftw_complex ) * height * width ;
  fftw_complex* tmp1 = (fftw_complex*)fftw_malloc(memSize);
  fftw_complex* tmp2 = (fftw_complex*)fftw_malloc(memSize);
  fftw_plan plan1 = fftw_plan_dft_2d( height, width, tmp1, tmp1, FFTW_FORWARD, FFTW_ESTIMATE );
  fftw_plan plan2 = fftw_plan_dft_2d( height, width, tmp2, tmp2, FFTW_FORWARD, FFTW_ESTIMATE );

  for( int disp = 0; disp < MAX_DISPARITY; ++disp){
    
    // size of psf
    double size = fabs( (double)disp * param[0] + param[1] );
    if( size < 1.0 ) size = 1.0;

    printf("disp = %d, size = %lf ", disp, size);

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
    printf("r = %lf\n", r);
    dst[disp] = (fftw_complex*)fftw_malloc( memSize );
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
