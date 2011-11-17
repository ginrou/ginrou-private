#include <stdio.h>
#include "include.h"

#include <cv.h>

int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように


  showDispMap( readImage(argv[1]));

  return 0;



  char imgName[256];
  sprintf( imgName , "test/gradation.jpg");
  char txtName[256];
  sprintf( txtName , "test/gradation.txt");

  int height = 120;
  int width = 512;
  IMG* img = createImage( height, width );
  int h, w;
  
  for( h  = 0 ; h < height; ++h){
    for( w = 0; w < width; ++w){
      IMG_ELEM( img, h, w) = w/2;
    }
  }

  saveImage(img, imgName);

  IMG* test = readImage( imgName );
  FILE* fp = fopen( txtName, "w" );

  for( h = 0; h < height; ++h){
    for( w = 0 ;w < width; ++w){
      fprintf( fp, "%d, %d\n", w/2, IMG_ELEM( test, h, w));
    }
  }

  fclose( fp );


  return 0;
}

