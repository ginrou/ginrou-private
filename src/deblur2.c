#include "deblur2.h"


IMG* deblurFFTW( IMG* img, IMG* psf)
{
  
  double snr = 0.005;

  fftw_complex *src = NULL;
  fftw_complex *filter = NULL;
  fftw_complex *dbl = NULL;
  size_t memSize = sizeof(fftw_complex)*(img->height * img->width);
  
  src = (fftw_complex*)fftw_malloc( memSize );
  filter = (fftw_complex*)fftw_malloc( memSize );
  dbl = (fftw_complex*)fftw_malloc( memSize );

  int h, w;

  //window function
  Mat window = matrixAlloc( img->height, img->width);
  for(h=0;h<img->height;++h){
    for(w=0;w<img->width;++w){
      double wh = 0.5 - 0.5*cos( (double)h * M_PI * 2.0 / (double)img->height);
      double ww = 0.5 - 0.5*cos( (double)w * M_PI * 2.0 / (double)img->width);
      ELEM0( window, h, w) = wh * ww;
    }
  }


  //copy src
  for(h=0;h<img->height;++h){
    for(w=0;w<img->width;++w){
      int idx = h * img->width + w;
      src[idx][0] = (double)IMG_ELEM(img, h, w) * ELEM0(window, h, w);
      src[idx][1] = 0.0;
      filter[idx][0] = 0.0;
      filter[idx][1] = 0.0;
    }
  }
  
  //copy psf
  double sum = 0.0;
  for(h=0;h<psf->height; ++h){
    for(w=0;w<psf->width;++w){
      int idx = h * img->width + w;
      filter[idx][0] = (double)IMG_ELEM(psf, psf->height - h, psf->width -w);
      sum += (double)IMG_ELEM(psf, psf->height - h, psf->width -w);
    }
  }

  for(h=0;h<psf->height; ++h){
    for(w=0;w<psf->width;++w){
      int idx = h * img->width + w;
      filter[idx][0] /= sum;
    }
  }

  //makeplan & FFT
  fftw_plan pSrc = fftw_plan_dft_2d( img->height, img->width, src, src, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan pFil = fftw_plan_dft_2d( img->height, img->width, filter, filter, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pSrc);
  fftw_execute(pFil);
  
  //deconvolution
  for(h=0;h<img->height;++h){
    for(w=0;w<img->width;++w){
      int idx = h * img->width + w;
      double a = src[idx][0];
      double b = src[idx][1];
      double c = filter[idx][0];
      double d = filter[idx][1];

      dbl[idx][0] = ( a*c + b*d ) / (c*c + d*d + snr);
      dbl[idx][1] = ( b*c - a*d ) / (c*c + d*d + snr);

    }
  }

  //makeplan of IDFT
  fftw_plan pDbl = fftw_plan_dft_2d( img->height, img->width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute(pDbl);

  IMG *dst = createImage( img->height, img->width);
  double scale = img->height * img->width;
  for(h=0;h<dst->height;++h){
    for(w=0;w<dst->width;++w){
      int idx = h * img->width + w;
      double val = dbl[idx][0] * dbl[idx][0] + dbl[idx][1] * dbl[idx][1];
      val = sqrt(val) / scale;
      IMG_ELEM(dst, h, w) = (uchar)(val);
    }
  }

  return dst;

}


IMG* deblurFFTWInvariant( IMG* src,
			  IMG* psfBase,
			  IMG* disparityMap,
			  double param[2])
{
  int h, w, i;
  int maxDisparity = MAX_DISPARITY;
  int BlockRows = ceil( (double)src->height / (double)BLOCK_SIZE );
  int BlockCols = ceil( (double)src->width / (double)BLOCK_SIZE );
  size_t arraySize = CUT_OFF_SIZE * CUT_OFF_SIZE ;
  double snr = 0.005;

  //作業用領域
  fftw_complex *srcFFTW = (fftw_complex*)fftw_malloc( sizeof(fftw_complex) * arraySize );
  fftw_complex *dstFFTW = (fftw_complex*)fftw_malloc( sizeof(fftw_complex) * arraySize );
  fftw_complex *psfFFTW[MAX_DISPARITY]; 

  //結果保存用
  Mat dstMat = matrixAlloc( src->height, src->width );
  Mat weightMat = matrixAlloc( src->height, src->width );
  for( h = 0; h < dstMat.row; ++h){
    for( w = 0; w < dstMat.clm; ++w){
      ELEM0( dstMat, h, w) = 0.0;
      ELEM0( weightMat, h, w) = 0.0;
    }
  }


  //psfをいろいろとリサイズ
  for( int disparity = 0; disparity < MAX_DISPARITY; ++disparity){
    
    if(disparity == 0 || disparity > MAX_DISPARITY){
      psfFFTW[disparity] = NULL;
      continue;
    }

    //resize psf
    IMG* tmp = createImage( disparity, disparity);
    resizeImage( psfBase, tmp);

    //get norm
    double norm = 0.0;
    for(h=0;h<tmp->height;++h){
      for(w=0;w<tmp->width;++w){
	norm += (double)IMG_ELEM(tmp, h, w);
      }
    } 
    
    //malloc psf[disparity] and set zero
    psfFFTW[disparity] = (fftw_complex*)fftw_malloc( sizeof(fftw_complex) * arraySize );
    for( int i = 0; i < arraySize; ++i){
      psfFFTW[disparity][i][0] = 0.0;
      psfFFTW[disparity][i][1] = 0.0;
    }

    //copy to psfFFTW and normalize 
    for( h = 0; h < tmp->height ; ++h){
      for( w = 0; w < tmp->width ; ++w){
	psfFFTW[disparity][ h * CUT_OFF_SIZE + w][0] = (double)IMG_ELEM( tmp, disparity - h, disparity - w) / norm ;
      }
    }

    //make plan 
    fftw_plan psfPlan = fftw_plan_dft_2d( CUT_OFF_SIZE, CUT_OFF_SIZE, psfFFTW[disparity], psfFFTW[disparity], FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute( psfPlan );

    //release
    if( psfPlan != NULL )fftw_destroy_plan( psfPlan);
    releaseImage( &tmp);

  }


  //window function
  Mat window = matrixAlloc( CUT_OFF_SIZE, CUT_OFF_SIZE);
  for(h=0;h<CUT_OFF_SIZE;++h){
    for(w=0;w<CUT_OFF_SIZE;++w){
      double wh = 0.5 - 0.5*cos( (double)h * M_PI * 2.0 / (double)CUT_OFF_SIZE);
      double ww = 0.5 - 0.5*cos( (double)w * M_PI * 2.0 / (double)CUT_OFF_SIZE);
      ELEM0( window, h, w) = wh * ww;
    }
  }

  

  //deblur
  
  //make plan
  fftw_plan srcPlan = fftw_plan_dft_2d( CUT_OFF_SIZE, CUT_OFF_SIZE, srcFFTW, srcFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan dstPlan = fftw_plan_dft_2d( CUT_OFF_SIZE, CUT_OFF_SIZE, dstFFTW, dstFFTW, FFTW_BACKWARD, FFTW_ESTIMATE);
  

  for( int row = 0; row < BlockRows; ++row){
    for( int col = 0 ; col < BlockCols; ++col){

      
      //copy src image to srcFFTW
      for( h = 0; h < CUT_OFF_SIZE; ++h){
	for( w= 0 ; w < CUT_OFF_SIZE; ++w){
	  
	  int idx = h * CUT_OFF_SIZE + w;
	  srcFFTW[idx][0] = 0.0;
	  srcFFTW[idx][1] = 0.0;

	  int y = h + row * BLOCK_SIZE + (BLOCK_SIZE - CUT_OFF_SIZE) /2;
	  int x = w + col * BLOCK_SIZE + (BLOCK_SIZE - CUT_OFF_SIZE) /2;
      
	  if( y < 0 || y >= src->height || x < 0 || x >= src->width){
	    continue;
	  }else{
	    srcFFTW[idx][0] = (double)IMG_ELEM(src, y, x) * ELEM0(window, h, w);
	  }

	}
      }
      //copy done

      //kernelSizeの決定
      int disparity = (int)IMG_ELEM(disparityMap, row*BLOCK_SIZE + BLOCK_SIZE/2, col*BLOCK_SIZE + BLOCK_SIZE/2);
      int kernelSize = param[0] * (double)disparity + param[1];
      
      //fourier transform of src
      fftw_execute( srcPlan );

      //wiener part
      for(i = 0 ; i < arraySize; ++i){
	double a = srcFFTW[i][0];
	double b = srcFFTW[i][1];
	double c = psfFFTW[kernelSize][i][0];
	double d = psfFFTW[kernelSize][i][1];
	
	dstFFTW[i][0] = ( a*c + b*d ) / ( c*c + d*d + snr );
	dstFFTW[i][1] = ( b*c - a*d ) / ( c*c + d*d + snr );
      }

      //IDFT of dst
      fftw_execute( dstPlan );

      //add to dstMat
      for( h = 0; h < CUT_OFF_SIZE; ++h){
	for( w= 0 ; w < CUT_OFF_SIZE; ++w){
	  int idx = h * CUT_OFF_SIZE + w;
	  int y = h + row * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;
	  int x = w + col * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;

	  if( y < 0 || y >= src->height || x < 0 || x >= src->width){
	    continue;
	  }else{
	    double val = dstFFTW[idx][0] * dstFFTW[idx][0] + dstFFTW[idx][1] * dstFFTW[idx][1] ;
	    val = sqrt(val) / (double)arraySize;
	    ELEM0(dstMat, y, x) += val;
	    ELEM0(weightMat, y, x) += ELEM0(window, h, w);
	  }
	  
	}
      }
      //end of adding

    }//col
  }//row

  //get result
  IMG* dst = createImage( src->height, src->width);
  
  for(h=0;h<dst->height;++h){
    for(w=0;w<dst->width;++w){
      double val = ELEM0( dstMat, h, w) / ELEM0(weightMat, h, w);
      if(val > UCHAR_MAX ) val = UCHAR_MAX;
      if(val < 0 ) val = 0;

      IMG_ELEM(dst, h, w) = (uchar)val;
    }
  }

  //cleaning
  matrixFree(dstMat);
  matrixFree(window);
  fftw_free(srcFFTW);
  fftw_free(dstFFTW);
  for(i=0;i<MAX_DISPARITY;++i)
    if( psfFFTW[i] ) fftw_free( psfFFTW[i] );
  fftw_destroy_plan(srcPlan);
  fftw_destroy_plan(dstPlan);
  

  return dst;

}


IMG* deblurFFTWResize( IMG* img, IMG* psf, double size)
{
  double snr = 0.005;

  int h, w, i;
  
  size_t imgArrSize = img->height * img->width;
  size_t psfFH =  psf->height;
  size_t psfFW =  psf->width;
  size_t psfArrSize = psfFH * psfFW;
  
  printf("img Array size = %d, psf Array Size = %d\n", (int)imgArrSize, (int)psfArrSize);

  fftw_complex* imgF = (fftw_complex*)fftw_malloc( sizeof( fftw_complex ) * imgArrSize );
  fftw_complex* psfF = (fftw_complex*)fftw_malloc( sizeof( fftw_complex ) * psfArrSize );
  fftw_complex* dstF = (fftw_complex*)fftw_malloc( sizeof( fftw_complex ) * imgArrSize );  

  if( !imgF || !psfF ){
    fprintf(stderr, "error in allocatin fftw_complex\n");
    exit(1);
  }

  //copy img
  for( i = 0; i < imgArrSize; ++i){
    imgF[i][0] = (double)( img->data[i] );
    imgF[i][1] = 0.0;
  }

  printf("copy done\n");

  //copy psf
  for(i=0;i<psfArrSize;++i){
    psfF[i][0] = 0.0;
    psfF[i][1] = 0.0;
  }



  double sum = 0.0;
  for(i=0; i < psf->width * psf->height ;++i){
    sum += (double)( img->data[i] );
  }


  
  for(h=0;h<psf->height;++h){
    for(w=0;w<psf->width;++w){
      psfF[h*psfFW+w][0] = (double)IMG_ELEM( psf, psf->height - h, psf->width - w) / sum;
    }
  }
  

      
  //make plan & FFT
  fftw_plan imgPlan = fftw_plan_dft_2d( img->height, img->width, imgF, imgF, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan psfPlan = fftw_plan_dft_2d( psfFH, psfFW, psfF, psfF, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(imgPlan);
  fftw_execute(psfPlan);
  
  //deconvolution
  for(h=0;h<img->height;++h){
    for(w=0;w<img->width;++w){
      int imgIndex = h * img->width + w;
      double rh = size / (double)psf->height;
      double rw = size / (double)psf->width;
      int psfIndex = ( h / rh ) * psfFW + ( w / rw );
      
      double a = imgF[imgIndex][0];
      double b = imgF[imgIndex][1];
      double c = psfF[psfIndex][0] / ( rh * rw );
      double d = psfF[psfIndex][1] / ( rh * rw );

      dstF[imgIndex][0] = ( a*c + b*d ) / ( c*c + d*d + snr );
      dstF[imgIndex][1] = ( b*c - a*d ) / ( c*c + d*d + snr );

    }
  }
  
  printf("deconvolution done\n");

  //IDFT
  fftw_plan dstPlan = fftw_plan_dft_2d( img->height, img->width, dstF, dstF, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute( dstPlan );
					

  //dst
  IMG* dst = createImage( img->height, img->width );
  double scale = imgArrSize;

  for(i=0;i<imgArrSize;++i){
    double val = dstF[i][0] * dstF[i][0] + dstF[i][1] * dstF[i][1] ;
    val = sqrt(val) / scale;
    dst->data[i] = (uchar)val;
  }

  printf("all don to do in deblurFFTWResize\n");

  return dst;

}