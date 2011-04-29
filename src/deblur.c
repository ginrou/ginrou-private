/****************************************
  deblur.c
 ****************************************/

#include <deblur.h>

//ローカル関数
void normalize( Complex arr[FFT_SIZE][FFT_SIZE] );
double L1norm( Complex arr[FFT_SIZE][FFT_SIZE] );

void wienerdeconvolution( Complex src[FFT_SIZE][FFT_SIZE],
			  Complex filter[FFT_SIZE][FFT_SIZE],
			  Complex dst[FFT_SIZE][FFT_SIZE], 
			  double snr)
{
  for( int y = 0; y < FFT_SIZE; ++y){
    for( int x = 0 ; x < FFT_SIZE; ++x){
      double a, b, c, d;
      a = (src[y][x]).Re;
      b = (src[y][x]).Im;
      c = (filter[y][x]).Re;
      d = (filter[y][x]).Im;
      
      (dst[y][x]).Re = ( a*c + b*d ) / ( c*c + d*d + snr );
      (dst[y][x]).Im = ( b*c - a*d ) / ( c*c + d*d + snr );
    }
  }
}


IMG* deblur(const IMG *img,const IMG* psf)
{
  double imgIn[FFT_SIZE][FFT_SIZE] = { 0 };
  double psfIn[FFT_SIZE][FFT_SIZE] = { 0 };
  double dstIn[FFT_SIZE][FFT_SIZE];

  Complex imgFreq[FFT_SIZE][FFT_SIZE];
  Complex psfFreq[FFT_SIZE][FFT_SIZE];
  Complex dstFreq[FFT_SIZE][FFT_SIZE];

  //copy img->imgIn;
  for( int h = 0 ; h < FFT_SIZE; ++h){
    for(int w = 0 ; w < FFT_SIZE; ++w){
      imgIn[h][w] = IMG_ELEM( img, h, w);
    }
  }
  //copy psf -> psfIn
  //deblur過程でpsfは反転する
  for(int h = 0; h < psf->height; ++h){
    for(int w = 0; w < psf->width; ++w){
      int y = psf->height - h;
      int x = psf->width - w;
      psfIn[h][w] = IMG_ELEM(psf, y, x);
    }
  }


  //fourier transform
  fourier(imgFreq, imgIn);
  fourier(psfFreq, psfIn);


  printf("fourier transform done\n");
  printPassedTime();

  //wiener deconvolution
  wienerdeconvolution( imgFreq, psfFreq, dstFreq, 0.001);

  // invers fourier transform
  inverseFourier( dstIn, dstFreq);

  printf("wiener deconvolution and inverse fouiere transform done\n");
  printPassedTime();

  //copy to IMG structure
  IMG* dst = createImage(FFT_SIZE, FFT_SIZE);
  for(int h = 0; h < FFT_SIZE; ++h){
    for( int w = 0 ; w < FFT_SIZE; ++w){
      IMG_ELEM(dst, h, w) = fabs(dstIn[h][w]) / 3.0;

      if( h%10 == 0 && w%10 ==0)
	printf("dst[%03d][%03d] = %lf\n", h, w, dstIn[h][w]);

    }
  }

  return dst;
}



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

// L1ノルムが1となるように調整
// 複素数なので norm = sum( |z| )　で調整
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

double L1norm( Complex arr[FFT_SIZE][FFT_SIZE] )
{
  double norm = 0.0;
  for(int h = 0; h < FFT_SIZE; ++h){
    for(int w = 0 ; w < FFT_SIZE; ++w){
      double re = (arr[h][w]).Re;
      double im = (arr[h][w]).Im;
      norm += sqrt( re*re + im*im );
    }
  }
  return norm;
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
