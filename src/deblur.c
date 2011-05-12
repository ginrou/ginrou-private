/****************************************
  deblur.c
 ****************************************/

#include <deblur.h>


void wienerdeconvolution( Complex src[FFT_SIZE][FFT_SIZE],
			  Complex filter[FFT_SIZE][FFT_SIZE],
			  Complex dst[FFT_SIZE][FFT_SIZE], 
			  double snr)
{

  printf("wiener deconvolution...");

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
  printf("done\n");
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
  Complex psf[PSF_SIZE][CUT_OFF_SIZE][CUT_OFF_SIZE];
  createPSF(psf, psfBase, 1, PSF_SIZE-1);

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
      
      printf("block %d, %d\n", row, col);

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
      
      printf("disprity = %d, kernelSize = %d\n",disparity, kernelSize);

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
      
      printPassedTime();

    }//col
  }//row


  //最終的に返す構造体
  IMG* dst = createImage( src->height, src->width);

  //weright mean
  for(h=0;h<dstMat.row;++h){
    for(w=0;w<dstMat.clm;++w){
      ELEM0(dstMat, h, w) /= ELEM0(wegithMat, h, w);
    }
  }

  //normalize
  double min = DBL_MAX, max = -DBL_MAX;
  point pt, PT;
  for( h = CUT_OFF_SIZE/2 ; h < dstMat.row - CUT_OFF_SIZE/2 ;++h){
    for( w = CUT_OFF_SIZE/2 ; w < dstMat.clm -CUT_OFF_SIZE/2 ;++w){
      double val = ELEM0(dstMat, h, w);
      if(val < min) {min = val; pt = Point(w, h);}
      if(val > max) {max = val; PT = Point(w,h);}

      if( h % 10 == 0 && w % 10 == 0)
	printf("%d, %d, val = %lf\n",h,w,val);

    }
  }

  printf("min = %lf at %d,%d , max = %lf at %d, %d\n",min, pt.y, pt.x, max, PT.y, PT.x);

  for( h = 0 ; h < dst->height ; ++h ){
    for( w = 0 ; w < dst->width ; ++w ){
      IMG_ELEM(dst, h, w) = (uchar)((255.0/(max-min))*( ELEM0(dstMat, h, w) - min));
      IMG_ELEM(dst, h, w) = (uchar)((255.0/(max-min))*( ELEM0(dstMat, h, w) - min));
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
void createPSF( Complex dst[PSF_SIZE][FFT_SIZE][FFT_SIZE],
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
