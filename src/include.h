#ifndef __INCLUDE_HEADER__
#define __INCLUDE_HEADER__

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES

#define LEFT_CAM 0
#define CENTER_CAM 1
#define RIGHT_CAM 2

#define MAX_DISPARITY 32
#define MIN_DISPARITY 16
#define MAX_PSF_SIZE 32
#define FFT_SIZE 64

// deblurで切り分けるサイズ
#define CUT_OFF_SIZE FFT_SIZE //切り取る大きさはFFTのと同じにしなければならない
#define BLOCK_SIZE 16 //2^nのほうが都合が良い

// depth estimation でどれくらい近くまで探索するか
#define WINDOW_SIZE 20

// wiener deconvolutionにおける正則化パラメータ
#define SNR 0.002 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <fftw3.h>
typedef fftw_complex freq; 

typedef struct point{
  int x;
  int y;
}point;
point Point(int x, int y);


// name of dir to save debugging images
extern char tmpImagesDir[256];
extern int saveDebugImages;


#include "complex.h"
#include "imageData.h"
#include "util.h"
#include "imageProcessing.h"
#include "stereo.h"
#include "deblur.h"
#include "matrix.h"
#include "fourier.h"
#include "blur.h"
#include "deblur2.h"
#include "psf.h"
#include "expsystem.h"
#include "depthEstimation.h"
#include "batch.h"


// macros
#define SQSUM( a, b) ((a)*(a) + (b)*(b))
#define SQUARE(a) ( (a) * (a) )

#endif 
