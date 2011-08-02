#ifndef __DEBLUR__
#define __DEBLUR__

#include <stdio.h>
#include <math.h>

#include "fourier.h"
#include "matrix.h"

#include <imageData.h>
#include <imageProcessing.h>
#include <util.h>
#include <stereo.h>


#define SNR 0.005

// dst[i] = src[i] / filter[i] のような感じでぼけ除去を行う
void wienerdeconvolution( Complex src[FFT_SIZE][FFT_SIZE], //ぼけ画像(入力)
			  Complex filter[FFT_SIZE][FFT_SIZE], //ぼけ除去フィルタ
			  Complex dst[FFT_SIZE][FFT_SIZE], //ぼけ除去画像
			  double snr);//信号対雑音比(Signal Noise Ratio)


IMG* deblur(const IMG* src, //ぼけ画像
	    const IMG* psfBase,//psfの元画像
	    const IMG* disparityMap,//視差マップ
	    double param[]);//視差 <-> PSFサイズを変換するパラメータ


//basePSFをminSizeからmaxSizeまでのサイズで拡大縮小して
//フーリエ変換してdstへつめて返す
void createPSF( Complex dst[PSF_SIZE][FFT_SIZE][FFT_SIZE],
		const IMG* basePsf, int minSize, int maxSize);

//ハミング窓を作って返す
//大きさは FFT_SIZE * FFT_SIZE
Mat createWindowFunction(void);


IMG* deblur2( const IMG* src, const IMG* psf, int size);

#endif
