/****************************************
  deblur.c
 ****************************************/

#include <deblur.h>

//ローカル関数
void normalize( Complex arr[SIZE][SIZE] );
double L1norm( Complex arr[SIZE][SIZE] );

void wienerdeconvolution( Complex src[SIZE][SIZE],
			  Complex filter[SIZE][SIZE],
			  Complex dst[SIZE][SIZE], 
			  double snr)
{
  for( int y = 0; y < SIZE; ++y){
    for( int x = 0 ; x < SIZE; ++x){
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
  unsigned char imgIn[SIZE][SIZE] = { 0 };
  unsigned char psfIn[SIZE][SIZE] = { 0 };
  unsigned char dstIn[SIZE][SIZE];

  Complex imgFreq[SIZE][SIZE];
  Complex psfFreq[SIZE][SIZE];
  Complex dstFreq[SIZE][SIZE];

  //copy img->imgIn;
  for( int h = 0 ; h < SIZE; ++h){
    for(int w = 0 ; w < SIZE; ++w){
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

  printf("img norm = %lf\n", L1norm(imgFreq));
  printf("psf norm = %lf\n", L1norm(psfFreq));


  printf("fourier transform done\n");
  printPassedTime();

  //normalize psfFreq
  //normalize(psfFreq);

  //wiener deconvolution
  wienerdeconvolution( imgFreq, psfFreq, dstFreq, 0.0001);

  // invers fourier transform
  inverseFourier( dstIn, dstFreq);

  printf("wiener deconvolution and inverse fouiere transform done\n");
  printPassedTime();

  //copy to IMG structure
  IMG* dst = createImage(SIZE, SIZE);
  for(int h = 0; h < SIZE; ++h){
    for( int w = 0 ; w < SIZE; ++w){
      IMG_ELEM(dst, h, w) = dstIn[h][w];
    }
  }

  return dst;
}



IMG* blur(IMG *img, IMG* psf)
{
  Complex psfIn[SIZE][SIZE] = { 0.0 };
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
void normalize( Complex arr[SIZE][SIZE] )
{
  double norm = 0.0;

  //compute norm
  for( int h = 0; h < SIZE; ++h){
    for(int w = 0; w < SIZE; ++w){
      double re = arr[h][w].Re;
      double im = arr[h][w].Im;
      norm += sqrt( re*re + im*im );
    }
  }

  printf("norm = %lf\n", norm);

  // normalize 
  for( int h = 0 ; h < SIZE; ++h){
    for( int w = 0; w < SIZE; ++w){
      arr[h][w].Re /= norm;
      arr[h][w].Im /= norm;
    }
  }

  return;

}

double L1norm( Complex arr[SIZE][SIZE] )
{
  double norm = 0.0;
  for(int h = 0; h < SIZE; ++h){
    for(int w = 0 ; w < SIZE; ++w){
      double re = (arr[h][w]).Re;
      double im = (arr[h][w]).Im;
      norm += sqrt( re*re + im*im );
    }
  }
  return norm;
}
