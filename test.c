#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


int main( int argc, char* argv[]){

  if( argc == 1 ){
    printf("input arguments are\n");
    printf("arg[1] : input filename\n");
    printf("arg[2] : output filename\n");
    printf("arg[3] : origin of x \n");
    printf("arg[4] : origin of y\n");
    printf("arg[5] : cut-off width\n");
    printf("arg[6] : cut-off height\n");
    return 0;
  }else if( argc < 6 ){
    printf("input arguments are wrong\n");
    return 1;
  }

  IMG_COL* input = readImageColor( argv[1] );
  int width = atoi(argv[5]);
  int height = atoi(argv[6]);
  IMG_COL* output = createImageColor(height, width);

  int x = atoi(argv[3]);
  int y = atoi(argv[4]);
  for( int c = 0; c < 3; ++c){
    for( int h = y; h < y + height; ++h){
      for( int w = x; w < x + width; ++w){
	IMG_ELEM( output->channel[c], h-y, w-x) 
	  = IMG_ELEM( input->channel[c], h, w);
      }
    }
  }
  saveImageColor( output, argv[2] );

  return 0;
}
