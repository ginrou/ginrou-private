/****************************************
  deblur.c
 ****************************************/

#include <deblur.h>


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


IMG* deblur(const IMG* src, 
	    const IMG* psfBase,
	    const IMG* disparityMap,
	    double param[])
{
  int h,w;
  int maxDisparity = MAX_DISPARITY;
  int BlockRows = ceil( (double)src->height / (double)BLOCK_SIZE );
  int BlockCols = ceil( (double)src->width / (double)BLOCK_SIZE );

  
  //psf
  Complex psf[MAX_PSF_SIZE][CUT_OFF_SIZE][CUT_OFF_SIZE];
  createPSF(psf, psfBase, 1, MAX_PSF_SIZE-1);

  //窓関数
  Mat window = createWindowFunction();
  
  //最終的な結果を保存しておく場所
  Mat dstMat = matrixAlloc( src->height, src->width);
  Mat wegithMat = matrixAlloc( src->height, src->width);

  //0で初期化
  for( h = 0; h < dstMat.row; ++h){
    for( w = 0; w < dstMat.clm; ++w){
      ELEM0(dstMat, h, w) = 0.0;
      ELEM0(wegithMat, h, w) = 0.0;
    }
  }


  //作業用領域
  double srcIn[CUT_OFF_SIZE][CUT_OFF_SIZE];
  double dstIn[CUT_OFF_SIZE][CUT_OFF_SIZE];
  
  Complex srcF[CUT_OFF_SIZE][CUT_OFF_SIZE];
  Complex dstF[CUT_OFF_SIZE][CUT_OFF_SIZE];


  
  for(int row = 0; row < BlockRows; ++row){
    for(int col = 0 ; col < BlockCols; ++col){

      //copy & window function
      for( h = 0; h < CUT_OFF_SIZE; ++h){
	for( w = 0; w < CUT_OFF_SIZE; ++w){
	  srcIn[h][w] = 0.0;
	  
	  int y = h + row * BLOCK_SIZE + ( BLOCK_SIZE - CUT_OFF_SIZE ) / 2;
	  int x = w + col * BLOCK_SIZE + ( BLOCK_SIZE - CUT_OFF_SIZE ) / 2;

	  if( y < 0 || y >= src->height || w < 0 || w >= src->width){
	    continue;
	  }else{
	    srcIn[h][w] = (double)IMG_ELEM(src, y, x) * ELEM0(window, h, w);
	  }
	}
      }
      //copy done

      //kernel sizeの決定
      int disparity = (int)IMG_ELEM(disparityMap, row*BLOCK_SIZE + BLOCK_SIZE/2, col*BLOCK_SIZE + BLOCK_SIZE/2);
      int kernelSize =  param[0]*(double)disparity + param[1] ;
      
      //printf("disprity = %d, kernelSize = %d\n",disparity, kernelSize);

      //srcをDFT
      fourier(srcF, srcIn);

      //wiener deconvolution
      wienerdeconvolution(srcF, psf[kernelSize], dstF, SNR);

      //IDFT
      inverseFourier(dstIn, dstF);
      
      //copy to dstMat
      for(h=0;h<CUT_OFF_SIZE;++h){
	for(w=0;w<CUT_OFF_SIZE;++w){
	  int y = h + row * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;
	  int x = w + col * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;

	  if( y < 0 || y >= src->height || x < 0 || x >= src->width){
	    continue;
	  }else{
	    ELEM0(dstMat, y, x) += dstIn[h][w];
	    ELEM0(wegithMat, y, x) += ELEM0(window, h, w);
	  }

	}//w
      }//h
      


    }//col
  }//row

  printPassedTime();


  //最終的に返す構造体
  IMG* dst = createImage( src->height, src->width);

  //weright mean
  for(h=0;h<dstMat.row;++h){
    for(w=0;w<dstMat.clm;++w){
      ELEM0(dstMat, h, w) /= ELEM0(wegithMat, h, w);
    }
  }

  for( h = 0 ; h < dst->height ; ++h ){
    for( w = 0 ; w < dst->width ; ++w ){
      IMG_ELEM( dst, h, w) = fabs( ELEM0( dstMat, h, w) ) / 3.0;
    }
  }


  //後片付け
  matrixFree(dstMat);
  matrixFree(wegithMat);
  matrixFree(window);

  return dst;
}



//sizeがある程度より小さいときとか
//0スタートじゃないと使いづらいかも
void createPSF( Complex dst[MAX_PSF_SIZE][FFT_SIZE][FFT_SIZE],
		const IMG* basePsf, int minSize, int maxSize)
{
  double psf[FFT_SIZE][FFT_SIZE];
  int h ,w;
  for(int size = minSize; size <= maxSize; ++size)
    {
      if( size <= 0 ) continue;
      printf("createPSF size = %d\n",size);
      IMG* img = createImage(size, size);
      resizeImage( basePsf, img);
      
      for(h=0;h<FFT_SIZE;++h){
	for(w=0;w<FFT_SIZE;++w){
	  psf[h][w] = 0.0;
	}
      }
      
      for(h=0;h<size;++h){
	for(w=0;w<size;++w){
	  psf[h][w] = (double)IMG_ELEM(img, size-h, size-w);
	}
      }
      
      releaseImage(&img);

      fourier(dst[size], psf);

    }

  printf("createPSF done\n");

  return;
}

//ハミング窓を作って返す
//大きさは FFT_SIZE * FFT_SIZE
Mat createWindowFunction(void)
{
  Mat ret = matrixAlloc( FFT_SIZE, FFT_SIZE);
  for(int h = 0; h < FFT_SIZE; ++h)
    {
      for(int w = 0 ; w < FFT_SIZE; ++w)
	{
	  double wh = 0.5 - 0.5*cos( (double)h * M_PI * 2.0 / (double)FFT_SIZE);
	  double ww = 0.5 - 0.5*cos( (double)w * M_PI * 2.0 / (double)FFT_SIZE);
	  ELEM0(ret, h, w) = wh * ww;
	}
    }
  return ret;
}



IMG* deblur2( const IMG* src, const IMG* psf, int size)
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
      imgIn[h][w] = IMG_ELEM( src, h, w);
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
  wienerdeconvolution( imgFreq, psfFreq, dstFreq, 0.0002);

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
