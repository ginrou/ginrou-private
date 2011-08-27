#ifndef __INCLUDE_HEADER__
#define __INCLUDE_HEADER__

#define YES 1
#define NO 0

#define LOAD_DISPARITY_MAP YES

#define LEFT_CAM 0
#define CENTER_CAM 1
#define RIGHT_CAM 2

#define MAX_DISPARITY 45
#define MAX_PSF_SIZE 32
#define FFT_SIZE 64
#define CUT_OFF_SIZE FFT_SIZE //切り取る大きさはFFTのと同じにしなければならない
#define BLOCK_SIZE 16 //2^nのほうが都合が良い


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
#include "batch.h"
#include "psf.h"
#include "expsystem.h"

#endif 
