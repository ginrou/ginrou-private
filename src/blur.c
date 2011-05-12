/****************************************
 blur.c
 ****************************************/

#include <blur.h>

IMG* blur(IMG *img, IMG* psf)
{
  Complex psfIn[FFT_SIZE][FFT_SIZE] = { 0.0 };
  IMG* dst = createImage( img->height, img->width);

  for(int h = 0; h < psf->height; ++h){
    for( int w = 0 ; w < psf->width; ++w){
      psfIn[h][w].Re = (double)IMG_ELEM(psf, h, w);
    }
  }
  
  normalize( psfIn );
  
  for(int h = 0; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      
      double val = 0.0;

      for(int y = 0; y < psf->height; ++y){
	for(int x = 0; x < psf->width; ++x){
	  
	  if( h+y >= img->height || w+x >= img->width) continue;

	  val += (double)IMG_ELEM(img, h+y, w+x) * psfIn[y][x].Re;
	}
      }
      IMG_ELEM(dst, h, w) = (uchar)val;
    }

  }

  return dst;

}


IMG* blurFilter( IMG *img, IMG *psf)
{
  double imgIn[FFT_SIZE][FFT_SIZE] = {0.0};
  double psfIn[FFT_SIZE][FFT_SIZE] = {0.0};
  double dstIn[FFT_SIZE][FFT_SIZE];

  Complex imgFreq[FFT_SIZE][FFT_SIZE];
  Complex psfFreq[FFT_SIZE][FFT_SIZE];
  Complex dstFreq[FFT_SIZE][FFT_SIZE];

  //copy
  for(int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0 ; w < FFT_SIZE; ++w){
      imgIn[h][w] = (double)IMG_ELEM(img , h, w);
    }
  }

  //copy psf
  for( int h = 0 ; h < psf->height; ++h){
    for( int w = 0; w < psf->width; ++w){
      int y = psf->height - h;
      int x = psf->width - w;
      psfIn[h][w] = (double)IMG_ELEM(psf, y, x);
    }
  }

  fourier( imgFreq, imgIn);
  fourier( psfFreq, psfIn);

  for(int h = 0; h < FFT_SIZE; ++h){
    for( int w = 0 ; w < FFT_SIZE; ++w){
      double a = imgFreq[h][w].Re;
      double b = imgFreq[h][w].Im;
      double c = psfFreq[h][w].Re;
      double d = psfFreq[h][w].Im;

      dstFreq[h][w].Re = a*c - b*d;
      dstFreq[h][w].Im = b*c + d*a;

    }
  }
  
  inverseFourier( dstIn, dstFreq);


  IMG* dst = createImage( FFT_SIZE, FFT_SIZE);
  for(int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0; w < FFT_SIZE; ++w){
      IMG_ELEM(dst, h, w) = dstIn[h][w];
    }
  }

  return dst;
  

}

void normalize( Complex arr[FFT_SIZE][FFT_SIZE] )
{
  double norm = 0.0;

  //compute norm
  for( int h = 0; h < FFT_SIZE; ++h){
    for(int w = 0; w < FFT_SIZE; ++w){
      double re = arr[h][w].Re;
      double im = arr[h][w].Im;
      norm += sqrt( re*re + im*im );
    }
  }

  printf("norm = %lf\n", norm);

  // normalize 
  for( int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0; w < FFT_SIZE; ++w){
      arr[h][w].Re /= norm;
      arr[h][w].Im /= norm;
    }
  }

  return;

}