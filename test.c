#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


int main( int argc, char* argv[]){


  printf("disparity map checker \n");

  if( argc == 1 ){
    printf("input argument is\n");
    printf("arg[1] : disparity map image\n");
    return 0;
  }

  showDispMap( readImage( argv[1] ));
  return 0;
}
