#ifndef __DEBLUR__
#define __DEBLUR__

#include <stdio.h>
#include <math.h>

#include <hiuraLib.h>
#include <imageData.h>
#include <imageProcessing.h>
#include <util.h>


// dst[i] = src[i] / filter[i] のような感じでぼけ除去を行う
void wienerdeconvolution( Complex src[SIZE][SIZE], //ぼけ画像(入力)
			  Complex filter[SIZE][SIZE], //ぼけ除去フィルタ
			  Complex dst[SIZE][SIZE], //ぼけ除去画像
			  double snr);//信号対雑音比(Signal Noise Ratio)

IMG* deblur(const IMG *img,const IMG* psf);

IMG* blur(IMG *img, IMG* psf);


#endif
