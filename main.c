#include <stdio.h>
#include "include.h"

#include <cv.h>


int main(int argc, char* argv[])
{
  setbuf( stdout, NULL); // 改行をまたないように

  return batch110808( argc, argv );
}
