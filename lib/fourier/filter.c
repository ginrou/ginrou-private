// 画像取り込み／保存プログラム。テスト用

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fourier.h>
#include <ppm.h>

unsigned char image[SIZE][SIZE];
Complex fimage[SIZE][SIZE];

#define RAD 20

main (int argc, char *argv[]) {
  int width, height, x, y, off_x, off_y;
  unsigned char *data;
  double maxPower;
  char fname[1000];

  if(argc != 2) {
    fprintf(stderr, "filter : test fourier transform library.\n");
    fprintf(stderr, "Usage : fourier <file name prefix>\n"); 
    fprintf(stderr, "file : PGM monochrome\n"); 
    exit(0);
  }

  // 読み出し ( 画素値型 unsigned char )
  data = readPGM(argv[1], &width, &height);
  printf("size = %d %d\n", width, height);

  fourier(fimage, (unsigned char (*)[SIZE])data);

  fourierSpectrumImage(image, fimage);

  strcpy(fname, argv[1]);
  strcat(fname, ".fft.pbm");
  writePGM(fname, (unsigned char *)image, SIZE, SIZE);

#if 1
  for( x = 0; x < SIZE; x++) {
    for( y = 0; y < SIZE; y++) {
      if((x > RAD && x < SIZE - RAD) || (y > RAD && y < SIZE - RAD)) {
	fimage[x][y].Re = fimage[x][y].Im = 0;
      }
    }
  }
#endif
  inverseFourier(image, fimage);

  strcpy(fname, argv[1]);
  strcat(fname, ".filt.pbm");
  writePGM(fname, (unsigned char *)image, SIZE, SIZE);

  free(data);
}

