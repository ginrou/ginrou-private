#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include.h"


int main( int argc, char* argv[]){

  if( argc == 1 ){
    printf("input argument : disparity map\n");
    return 0;
  }

  showDispMap(readImage(argv[1]));

  return 0;
}
