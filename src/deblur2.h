#ifndef __DEBLUR2__
#define __DEBLUR2__

#include "include.h"

/*
  winner deconvolutionに基づきdeblurを行う関数
  畳み込みはpsfを反転させなければならないので、
  予めpsfを反転させておく必要がある
  psfの正規化は行わなくてもよい(uchar型の配列なので実質は無理)
 */
IMG* deblurFFTW( IMG* img, IMG* psf);

IMG* deblurFFTW2( freq* src, freq* psf, double snr, int height, int width);

IMG* deblurFFTWInvariant( IMG* src,
			  IMG* psfBase,
			  IMG* disparityMap,
			  double param[2]);

IMG* deblurFFTWResize( IMG* img, IMG* psf, double size);

Mat hummingWindow( int imgHeight, int imgWidth, int psfHeight, int psfWidth);

// Zhouらの手法をベースにした処理手法
IMG* deblurFromTwoImagesLatent( IMG* imgLeft, IMG* imgRight,
				freq* psfLeft[], freq* psfRight[],
				IMG* disparityMap);

// 2枚のぼけ除去画像を生成して、PSFサイズの小さい方を取得する方法
IMG* deblurFromTwoImagesOriginal( const IMG* imgLeft, const IMG* imgRight,
				  freq* psfLeft[], const double paramLeft[2],
				  freq* psfRight[], const double paramRight[2],
				  IMG* disparityMap);



#endif

  
