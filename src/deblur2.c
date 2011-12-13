#include "deblur2.h"



IMG* deblurFFTW( IMG* img, IMG* psf)
{
  int h, w;  
  double snr = 0.002;
  size_t memSize = sizeof(freq)*(img->height * img->width);
  freq *src = (freq*)fftw_malloc( memSize );
  freq *filter = (freq*)fftw_malloc( memSize );
  freq *dbl = (freq*)fftw_malloc( memSize );

  //window function
  Mat window = hummingWindow( img->height, img->width, psf->height, psf->width);
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
  double norm = imageNormL1(psf);
  for(h=0;h<psf->height; ++h){
    for(w=0;w<psf->width;++w){
      
      int y = h - psf->height/2;
      int x = w - psf->width/2;
      y += (y<0) ? img->height : 0;
      x += (x<0) ? img->width : 0;

      int idx = y * img->width + x;
      filter[idx][0] = (double)IMG_ELEM(psf, h, w) / norm;

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
      IMG_ELEM(dst, h, w) = sqrt(val) / scale ;
    }
  }

  //clean
  fftw_free(src);
  fftw_free(filter);
  fftw_free(dbl);
  matrixFree(window);


  return dst;

}

IMG* deblurFFTW2( freq* src, freq* psf, double snr, int height, int width)
{
  freq* dbl = (freq*)fftw_malloc( sizeof(freq)*height*width );
  fftw_plan plan = fftw_plan_dft_2d( height, width, dbl, dbl, FFTW_BACKWARD, FFTW_ESTIMATE);
  
  //deblur
  for(int i = 0; i < height * width ; ++i){
    double a = src[i][0];
    double b = src[i][1];
    double c = psf[i][0];
    double d = psf[i][1];
    dbl[i][0] = ( a*c + b*d ) / ( c*c + d*d +snr );
    dbl[i][1] = ( b*c - a*d ) / ( c*c + d*d +snr );
  }
  
  //IFFT
  fftw_execute( plan );
  
  IMG* ret = createImage( height, width);
  double scale = height * width ;
  for(int h = 0; h < height; ++h){
    for(int w = 0; w < width; ++w){
      int idx = h * width + w;
      double val = dbl[idx][0] * dbl[idx][0] +dbl[idx][1] * dbl[idx][1] ;
      IMG_ELEM( ret, h ,w) = (uchar) (sqrt(val) / scale );
    }
  }

  fftw_free(dbl);
  fftw_destroy_plan(plan);
  return ret;

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
  freq *srcFFTW = (freq*)fftw_malloc( sizeof(freq) * arraySize );
  freq *dstFFTW = (freq*)fftw_malloc( sizeof(freq) * arraySize );
  freq *psfFFTW[MAX_DISPARITY];

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

    double size = disparity * param[0] + param[1];
    if( abs(size) == 0 ) size = 1.0;
    
    //resize psf
    IMG* tmp = createImage( abs(size), abs(size));
    resizeImage( psfBase, tmp);

    //get norm
    double norm = 0.0;
    for(h=0;h<tmp->height;++h){
      for(w=0;w<tmp->width;++w){
	norm += (double)IMG_ELEM(tmp, h, w);
      }
    } 
    
    printf( "disparity = %d, size = %lf, norm = %lf\n", disparity, size, norm);

    char filename[256];
    sprintf( filename, "img/MPro/exp/psf%02d.png", disparity);
    //saveImage( tmp, filename);

    //malloc psf[disparity] and set zero
    psfFFTW[disparity] = (freq*)fftw_malloc( sizeof(freq) * arraySize );
    for( int i = 0; i < arraySize; ++i){
      psfFFTW[disparity][i][0] = 0.0;
      psfFFTW[disparity][i][1] = 0.0;
    }

    //copy to psfFFTW and normalize 
    for( h = 0; h < tmp->height ; ++h){
      for( w = 0; w < tmp->width ; ++w){
	psfFFTW[disparity][h*CUT_OFF_SIZE+w][0]
	  = (double)IMG_ELEM( tmp, h, w) / norm ;
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
      
      //fourier transform of src
      fftw_execute( srcPlan );

      //wiener part
      for(i = 0 ; i < arraySize; ++i){
	double a = srcFFTW[i][0];
	double b = srcFFTW[i][1];
	double c = psfFFTW[disparity][i][0];
	double d = psfFFTW[disparity][i][1];
	
	dstFFTW[i][0] = ( a*c + b*d ) / ( c*c + d*d + snr );
	dstFFTW[i][1] = ( b*c - a*d ) / ( c*c + d*d + snr );
      }

      //IDFT of dst
      fftw_execute( dstPlan );

      IMG* dbg = createImage( CUT_OFF_SIZE, CUT_OFF_SIZE);
      

      //add to dstMat
      for( h = 0; h < CUT_OFF_SIZE; ++h){
	for( w= 0 ; w < CUT_OFF_SIZE; ++w){

	  int idx = h * CUT_OFF_SIZE + w;
	  int y = h + row * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;
	  int x = w + col * BLOCK_SIZE + (BLOCK_SIZE-CUT_OFF_SIZE)/2;

	  double hoge = dstFFTW[idx][0] * dstFFTW[idx][0] + dstFFTW[idx][1] * dstFFTW[idx][1] ;
	  IMG_ELEM( dbg, h, w) = hoge/ELEM0(window, h, w);

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
      
      char filename[256];
      sprintf( filename, "img/MPro/exp/test/blk%02d-%02d.png", row, col);
      //saveImage( dbg, filename );
      releaseImage(&dbg);

    }//col
  }//row


  printf("end of block processing\n");

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

  printf("end of merge\n");
  
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

  freq* imgF = (freq*)fftw_malloc( sizeof( freq ) * imgArrSize );
  freq* psfF = (freq*)fftw_malloc( sizeof( freq ) * psfArrSize );
  freq* dstF = (freq*)fftw_malloc( sizeof( freq ) * imgArrSize );  

  if( !imgF || !psfF ){
    fprintf(stderr, "error in allocatin freq\n");
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

Mat hummingWindow( int imgHeight, int imgWidth, int psfHeight, int psfWidth)
{
  Mat win = matrixAlloc( imgWidth, imgHeight);
  for(int h = 0; h < win.row; ++h){
    for(int w = 0; w < win.clm; ++w){
      double wh, ww;
      if( h < psfHeight )
	wh = 0.5 - 0.5 * cos( (double)h*M_PI / (double)psfHeight );
      else if( h >= imgHeight - psfHeight )
	wh = 0.5 - 0.5 * cos( (double)(h-imgHeight)*M_PI / (double)psfHeight);
      else
	wh = 1.0;

      if( w < psfWidth )
	ww = 0.5 - 0.5 * cos( (double)w*M_PI / (double)psfWidth );
      else if( w >= imgWidth - psfWidth )
	ww = 0.5 - 0.5 * cos( (double)(w-imgWidth)*M_PI / (double)psfWidth);
      else
	ww = 1.0;

      ELEM0( win, h, w) = ww * wh;
    }
  }
  return win;
}



IMG* deblurFromTwoImages( IMG* imgLeft, IMG* imgRight,
			  freq* psfLeft[], freq* psfRight[],
			  IMG* disparityMap)
{
  int h, w;
  int height = imgLeft->height;
  int width = imgLeft->width;

  size_t memSize = sizeof(freq) * height * width;
  freq* capLeft  = (freq*)fftw_malloc( memSize );
  freq* capRight = (freq*)fftw_malloc( memSize );
  freq* latent[MAX_DISPARITY];

  // FFT of captured images
  copySrc( imgLeft, capLeft );
  copySrc( imgRight, capRight );
  fftw_plan capLeftPlan = fftw_plan_dft_2d( height, width, capLeft, capLeft,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan capRightPlan = fftw_plan_dft_2d( height, width, capRight, capRight,
					    FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute( capLeftPlan );
  fftw_execute( capRightPlan );
  fftw_destroy_plan( capLeftPlan );
  fftw_destroy_plan( capRightPlan );
  
  // compute latent image at each disparity map
  for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d){
    
    // compute latent image
    latent[d] = (freq*)fftw_malloc( memSize );
    fftw_plan dftplan = fftw_plan_dft_2d( height, width, latent[d], latent[d],
					  FFTW_BACKWARD, FFTW_ESTIMATE);
    for( int i = 0; i < height * width; ++i){

      double fLr = psfLeft[d][i][0];
      double fLi = psfLeft[d][i][1];
      double fRr = psfRight[d][i][0];
      double fRi = psfRight[d][i][1];
      double yLr = capLeft[i][0];
      double yLi = capLeft[i][1];
      double yRr = capRight[i][0];
      double yRi = capRight[i][1];

      double denom = fLr*fLr + fLi*fLi + fRr*fRr + fRi*fRi + SNR ;
      latent[d][i][0] = ( fLr*yLr + fLi*yLi + fRr*yRr + fRi*yRi ) / denom ;
      latent[d][i][1] = ( fLr*yLi - fLi*yLr + fRr*yRi - fRi*yRr ) / denom ;
    }

    fftw_execute( dftplan );
    fftw_destroy_plan( dftplan );

  }
  
  
  // patch up deblurred images
  IMG* dst = createImage( height, width);
  double scale = height * width;
  for(int h = 0; h < height; ++h){
    for( int w = 0; w < width; ++w){
      int disp = IMG_ELEM( disparityMap, h, w);
      int idx = h * width + w;
      
      double re = latent[disp][idx][0];
      double im = latent[disp][idx][1];
      double val= sqrt( re*re + im*im ) / scale;
      if( val < 0 ) val = 0.0;
      else if( val > 255 ) val = 255.0;
      IMG_ELEM( dst, h, w) = val;
    }
  }

  
  // clean up
  for( int d = MIN_DISPARITY; d < MAX_DISPARITY; ++d)
    fftw_free( latent[d] );
  fftw_free( capLeft );
  fftw_free( capRight );

  return dst;

}
