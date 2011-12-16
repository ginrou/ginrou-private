#ifndef __PSF__
#define __PSF__

#include "include.h"

/*
  PSFを生成したり、PSFに関するユーティテイのクラス
  make*　はPSFを生成する。どういうのを作るかはこれによる
  どのPSFも正規化してから返す設定
  
 */


void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam);

void makeBlurPSF( IMG* psf[MAX_DISPARITY], IMG* aperture, int maxDepth,  double param[2]);

void makeBlurPSFMat( IMG* aperture, double param[2], Mat dst[MAX_DISPARITY],
		     point imgSize, int maxDepth);

void makeBlurPSFFreq( IMG* aperture, double param[2], freq* dst[MAX_DISPARITY],
		     point imgSize, int maxDepth);


void makeShiftBlurPSF( Mat psf[MAX_DISPARITY], int cam,
		       IMG* aperture, double par[2]);

void makeShiftBlurPSFFreq( int height, int width, int cam,
			   freq* dst[MAX_DISPARITY],
			   IMG* aperture, double param[2] );

void makeShiftBlurPSFFreq2x( int height, int width, int cam,
			   freq* dst[MAX_DISPARITY],
			   IMG* aperture, double param[2] );



// utility of psf

// PSFのそのままの形状から、Wienerに適用できるような形にする
// FFTとかはしない
void PSFCopyForFFTW( const Mat src, freq *dst, point size);

// Mat形式のPSFを正規化
void PSFNormalize( Mat psf );

// 余分な0の領域を除外
Mat PSFCutoffZeroRegion( Mat src);

// 0でない数をカウント
int numOfNonZero( Mat mat );

#endif 
