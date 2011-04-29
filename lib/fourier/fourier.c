#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fourier.h>

/* Fast Fourier Transform */

#define PI 3.14159265358979323846

static void make_sintbl( int n, double sintbl[] )
{
  int i;
  int n2, n4, n8;
  double c, s, dc, ds, t;

  n2 = n / 2;    n4 = n / 4;    n8 = n / 8;
  t = sin( PI / n );
  dc = 2 * t * t;    ds = sqrt( dc * ( 2 - dc ) );
  t = 2 * dc;    c = sintbl[n4] = 1;    s = sintbl[0] = 0;

  for( i = 1; i < n8; i++ ){
    c -= dc;    dc += t * c;
    s += ds;    ds -= t * s;
    sintbl[i] = s;    sintbl[n4 - i] = c;
  }
  if( n8 != 0 ) sintbl[n8] = sqrt( 0.5 );
  for( i = 0; i < n4; i++ ) sintbl[n2 - i] = sintbl[i];
  for( i = 0; i < n2 + n4; i++ ) sintbl[i + n2] = - sintbl[i];
}

static void make_bitrev( int n, int bitrev[] )
{
  int i, j, k, n2;

  n2 = n / 2;    i = j = 0;
  for( ; ; ){
    bitrev[i] = j;
    if( ++i >= n ) break;
    k = n2;
    while( k <= j ){
      j -= k;
      k /= 2;
    }
    j += k;
  }
}

static int fft( int n, double re[], double im[] )
{
  static int     last_n = 0;
  static int    *bitrev = NULL;
  static double *sintbl = NULL;

  int i, j, k, ik, h, d, k2, n4, inverse;
  double t, s, c, dre, dim;

  /* FFT or IFFT */
  if( n < 0 ){
    n = -n;
    inverse = 1;
  }
  else inverse = 0;
  n4 = n / 4;

  /* allocation */
  if( n != last_n || n == 0 ){
    last_n = n;
    if( sintbl != NULL ) free( sintbl );
    if( bitrev != NULL ) free( bitrev );
    if( n == 0 ) return 0;
    sintbl = malloc( ( n + n4 ) * sizeof( double ) );
    bitrev = malloc( n * sizeof( int ) );
    if( sintbl == NULL || bitrev == NULL ){
      fprintf( stderr, "Memory allocation error!\n" );
      return 1;
    }
    make_sintbl( n, sintbl );
    make_bitrev( n, bitrev );
  }

  /* bit reverse */
  for( i = 0; i < n; i++ ){
    j = bitrev[i];
    if( i < j ){
      t = re[i];    re[i] = re[j];    re[j] = t;
      t = im[i];    im[i] = im[j];    im[j] = t;
    }
  }

  /* transform */
  for( k = 1; k < n; k = k2 ){
    h = 0;    k2 = k + k;    d = n / k2;
    for( j = 0; j < k; j++ ){
      c = sintbl[h + n4];
      if( inverse ) s = - sintbl[h];
      else          s =   sintbl[h];
      for( i = j; i < n; i += k2 ){
	ik = i + k;
	dre = s * im[ik] + c * re[ik];
	dim = c * im[ik] - s * re[ik];
	re[ik] = re[i] - dre;    re[i] += dre;
	im[ik] = im[i] - dim;    im[i] += dim;
      }
      h += d;
    }
  }
  if( ! inverse )
    for( i = 0; i < n; i++ ){
      re[i] /= n;    im[i] /= n;
    }
  return 0;
}

/* 2D Fast Fourier Transform for PPM : Processing part */


void fourier(Complex out[][FFT_SIZE], unsigned char in[][FFT_SIZE] ) {
  int x, y;
  double re[FFT_SIZE], im[FFT_SIZE];

  /* FFT */
  fprintf( stderr, "Fourier transforming...\n" );

  for( x = 0; x < FFT_SIZE; x++ ){
    for( y = 0; y < FFT_SIZE; y++ ){
      re[y] = ( double )in[x][y];
      im[y] = 0;
    }
    if( fft( FFT_SIZE, re, im ) ) exit(0);
    for( y = 0; y < FFT_SIZE; y++ ){
      out[x][y].Re = re[y];
      out[x][y].Im = im[y];
    }
  }
  for( y = 0; y < FFT_SIZE; y++ ){
    for( x = 0; x < FFT_SIZE; x++ ){
      re[x] = out[x][y].Re;
      im[x] = out[x][y].Im;
    }
    if( fft( FFT_SIZE, re, im ) ) exit(0);
    for( x = 0; x < FFT_SIZE; x++ ){
      out[x][y].Re = re[x];
      out[x][y].Im = im[x];
    }
  }
}

void inverseFourier(unsigned char out[][FFT_SIZE], Complex in[][FFT_SIZE] ){
  int x, y;
  double re[FFT_SIZE], im[FFT_SIZE];
  double Real[FFT_SIZE][FFT_SIZE], Imag[FFT_SIZE][FFT_SIZE];

  /* IFFT */
  fprintf( stderr, "Inverse fourier transforming...\n" );

  for( y = 0; y < FFT_SIZE; y++ ){
    for( x = 0; x < FFT_SIZE; x++ ){
      re[x] = in[x][y].Re;
      im[x] = in[x][y].Im;
    }
    if( fft( -FFT_SIZE, re, im ) ) exit(0);
    for( x = 0; x < FFT_SIZE; x++ ){
      Real[x][y] = re[x];
      Imag[x][y] = im[x];
    }
  }
  for( x = 0; x < FFT_SIZE; x++ ){
    for( y = 0; y < FFT_SIZE; y++ ){
      re[y] = Real[x][y];
      im[y] = Imag[x][y];
    }
    if( fft( -FFT_SIZE, re, im ) ) exit(0);
    for( y = 0; y < FFT_SIZE; y++ ){
      int i = (int)re[y];

      if( i > 255) {
	out[x][y] = 255;
      }
      else if(i < 0) {
	out[x][y] = 0;
      }
      else {
	out[x][y] = i;
      }
      Real[x][y] = re[y];
      Imag[x][y] = im[y];
    }
  }
}

void fourierSpectrumImage(unsigned char out[][FFT_SIZE], Complex in[][FFT_SIZE])
{
  int x, y, i;
  int max_exp = -100, exp;
  double fra;
  int spectrum[FFT_SIZE][FFT_SIZE];

  /* Power Spectrum */
  fprintf( stderr, "Calculating power spectrum...\n" );

  for( y = 0; y < FFT_SIZE; y++ ) {
    for( x = 0; x < FFT_SIZE; x++ ) {
      exp = log10(compAbs(in[x][y])) * 100.0;
      spectrum[x][y] = exp;
      if(max_exp < exp) {
	max_exp = exp;
      }
    }
  }

  fprintf(stderr, "max_exp = %d\n", max_exp);

  for( y = 0; y < FFT_SIZE; y++ ) {
    for( x = 0; x < FFT_SIZE; x++ ) {
      int rx, ry;

      rx = (x + FFT_SIZE / 2) % FFT_SIZE;
      ry = (y + FFT_SIZE / 2) % FFT_SIZE;
#if 1
      i = spectrum[rx][ry] - max_exp + 255;
#else
      i = spectrum[rx][ry] + 128;
#endif
      if(i < 0) {
	out[x][y] = 0;
      }
      else if(i > 255) {
	out[x][y] = 255;
      }
      else {
	out[x][y] = i;
      }
    }
  }
}

void fourier1D(Complex out[], unsigned char in[]) {
  int x;
  double re[FFT_SIZE], im[FFT_SIZE];

  for( x = 0; x < FFT_SIZE; x++ ){
    re[x] = ( double )in[x];
    im[x] = 0;
  }
  if( fft( FFT_SIZE, re, im ) ) exit(0);
  for( x = 0; x < FFT_SIZE; x++ ){
    out[x].Re = re[x];
    out[x].Im = im[x];
  }
}

void inverseFourier1D(unsigned char out[], Complex in[] ){
  int x, y;
  double re[FFT_SIZE], im[FFT_SIZE];

  /* IFFT */
  for( x = 0; x < FFT_SIZE; x++ ){
    re[x] = in[x].Re;
    im[x] = in[x].Im;
  }
  if( fft( -FFT_SIZE, re, im ) ) exit(0);
  for( x = 0; x < FFT_SIZE; x++ ){
    int i = (int)re[x];

    if( i > 255) {
      out[x] = 255;
    }
    else if(i < 0) {
      out[x] = 0;
    }
    else {
      out[x] = i;
    }
  }
}
