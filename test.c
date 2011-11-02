#include <stdio.h>
#include <string.h>

#include "include.h"


int main( int argc, char* argv[]){

  /*
    argv[1] : texture image
    argv[2] : DTP param
    argv[3] : DTP param 
    argv[4] : blurred image left
    argv[5] : blurred image right
    argv[6] : ground truth disparity
    argv[7] : estimated disparity
    output  : errors
   */

  
  int height = 256;
  int width = 256;
  int h, w;
  IMG* texture = readImage( argv[1] );
  IMG* disparityMap = createImage( height, width );
  freq* psfLeft[MAX_DISPARITY], *psfRight[MAX_DISPARITY];
  double param[2];
  param[0] = atof( argv[2] );
  param[1] = atof( argv[3] );

 
  //  saveImage( textToIMG( argv[8], height, width , 4.0), argv[9] );
  //  return 0;


  // convert images
  IMG* img = createImage( height, width );
  resizeImage( texture, img);



  // create dispmap
  for( h = 0 ; h < height; ++h){
    for( w = 0; w < width; ++w){
      IMG_ELEM( disparityMap, h, w) =  4 * (h/(height/4)) + w/(width/4);
      if( IMG_ELEM( disparityMap, h, w) >= MAX_DISPARITY ) 
	IMG_ELEM( disparityMap, h, w) = MAX_DISPARITY-1;
    }
  }
  saveImage( disparityMap, argv[6] );

  // create PSF : aperture is Zhou0002
  IMG* aperture = readImage("img/aperture/Zhou0002.png");
  makeShiftBlurPSFFreq( height, width, LEFT_CAM, psfLeft, aperture, param ); 
  makeShiftBlurPSFFreq( height, width, RIGHT_CAM, psfRight, aperture, param ); 
  printf("create psf done\n");


  // blurring
  printf("dtp param = %lf, %lf\n", param[0], param[1]);
  IMG* blurredLeft = blurFreqWithMap( img, psfLeft, disparityMap);
  IMG* blurredRight = blurFreqWithMap( img, psfRight, disparityMap);
  printf("blurring done\n");
  saveImage( blurredLeft , argv[4] );
  saveImage( blurredRight , argv[5] );



  // depth estimation
  IMG* estimatedMap = latentBaseEstimationIMG( blurredLeft, blurredRight, psfLeft, psfRight);
  saveImage( estimatedMap, argv[7] );
  printf("depth estimation done\n");

  
  double sum = 0.0;
  for( h = 0; h < height; ++h){
    for( w = 0; w < width; ++w){
      double dif = IMG_ELEM( disparityMap, h, w) - IMG_ELEM( estimatedMap, h, w);
      if( dif != 0.0 )
	printf("%d, %d, %lf \n",h, w,  dif);
      sum += dif * dif;
    }
  }
  printf("avg = %lf\n", sqrt(sum) / (double)(height*width));


    
  return 0;
}
