#ifndef __FOURIER__
#define __FOURIER__

#define FFT_SIZE 256

#include <complex.h>

void fourier(Complex out[][FFT_SIZE],  unsigned char in[][FFT_SIZE] );
void inverseFourier( unsigned char out[][FFT_SIZE], Complex in[][FFT_SIZE]);
void fourierSpectrumImage(unsigned char out[][FFT_SIZE], Complex in[][FFT_SIZE]);

void fourier1D(Complex out[], unsigned char in[]);
void inverseFourier1D(unsigned char out[], Complex in[]);

#endif
