#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


IMG_COL* imgCol;
IMG* disparityMap;
IMG* aperture;
char dstFileName[256];

char windowName[256];
void mouseRefocus(int event, int x, int y, int flags, void *param);// マウスのコールバック

IMG* refocusWithPSFMap( IMG* img, IMG* dispMap, IMG* psf[MAX_DISPARITY]);

int main( int argc, char* argv[]){

  if( argc < 4){
    printf("execution file name %s\n", argv[0]);
    printf("input arguments are..\n");
    printf("argv[1] : all-focus image\n");
    printf("argv[2] : disparity map\n");
    printf("argv[3] : aperture \n");
    printf("argv[4] : output blurred image\n");
    return 1;
  }

  // load inputs
  imgCol = readImageColor( argv[1] );
  disparityMap = readImage( argv[2] );
  aperture = readImage( argv[3] );
  sprintf(dstFileName, "%s", argv[4] );

  // used in display
  IplImage *screan = cvLoadImage( argv[1], CV_LOAD_IMAGE_COLOR);
  cvNamedWindow( windowName, CV_WINDOW_AUTOSIZE );
  cvSetMouseCallback( windowName, mouseRefocus, NULL);

  while(1){
    cvShowImage( windowName, screan );
    int c = cvWaitKey(0);
    if( c == '\x1b' || c == 'q' )
      break;
  }
  cvDestroyWindow( windowName );
  cvReleaseImage( &screan );
  
  return 0;
}




void mouseRefocus(int event, int x, int y, int flags, void *param)// マウスのコールバック
{
  if( event == CV_EVENT_LBUTTONDOWN ){

    // input paramters
    int d = IMG_ELEM( disparityMap, y, x);
    double diameter;
    printf("input diamter of aperture...");
    scanf("%lf", &diameter);
    double param[2];
    param[0] = diameter;
    param[1] = -diameter * (double)d;
    printf("focused on %d, %d, disparity = %d, diameter = %lf",y, x, d, diameter);
    printf(", parameter = %lf * disp + %lf\n", param[0], param[1]);

    // create PSF
    IMG* psf[MAX_DISPARITY*2];
    makeBlurPSF( psf, aperture, MAX_DISPARITY*2, param);

    printf("make PSF done\n");
    // blurring
    IMG_COL dst;
    dst.height = imgCol->height;
    dst.width = imgCol->width;
    for(int c = 0; c < 3; ++c ){
      dst.channel[c] = refocusWithPSFMap( imgCol->channel[c],disparityMap, psf);
      printf("channel %d done\n", c);
    }
    // save 
    saveImageColor( &dst, dstFileName );
    
    // release files

  }
}

IMG* refocusWithPSFMap( IMG* img, IMG* dispMap, IMG* psf[MAX_DISPARITY*2])
{
  int h, w;
  IMG* dst = createImage( img->width, img->height );

  convertScaleImage( dst, dst, 0.0, 0.0 );

  for( h = 0; h < dst->height; ++h ){
    for( w = 0; w < dst->width; ++w){
      int d = IMG_ELEM( dispMap, h, w);
      if( d < 10 ){
	IMG_ELEM(dst, h, w ) = IMG_ELEM( img, h, w );
	continue;
      }
      double s = 0;
      double n = 0;
      for( int y = 0; y < psf[d]->height; ++y){
	for( int x = 0; x < psf[d]->width; ++x){
	  n += IMG_ELEM( psf[d], y, x);
	  int py = h + y - psf[d]->height / 2;
	  int px = w + x - psf[d]->width / 2;
	  if( py < 0 || py >= img->height || px < 0 || px >= img->width ) 
	    continue;
	  else
	    s += (double)IMG_ELEM(img, py, px ) * IMG_ELEM( psf[d], y, x);
	}
      }
      s /= n;
      s = (s > 255)? 255 : s;
      IMG_ELEM( dst, h, w ) = s;
    }
  }
  return dst;
}
