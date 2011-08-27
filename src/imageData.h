#ifndef __IMAGE_DATA__
#define __IMAGE_DATA__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>

#include <matrix.h>

typedef struct _IMG{
  int width;
  int height;
  unsigned char *data;
}IMG;

typedef struct _IMG_COL{
  int width;
  int height;
  IMG* channel[3];//img[0] = b, img[1] = g, img[2] = r
}IMG_COL;


//macro access to data of image data
#define IMG_ELEM(img, h, w) (((img)->data)[ (h)*( (img)->width ) + (w)  ] )

//macro access to data of image data as double 
#define DBL_ELEM(img, h, w) ((double)IMG_ELEM(img, h, w))


//converting image from Ipl <--> IMG
void convertIMG2Ipl( const IMG* src, IplImage *dst);
void convertIpl2IMG( const IplImage* src, IMG* dst);

//load image from file
//readable extention : .jpg, .png, .ppm, etc
IMG* readImage( char* filename );
IMG_COL* readImageColor( char* filename );

//save IMG structure to file
void saveImage( IMG* img, char *filename);
void saveImageColor( IMG_COL *img, char *filename);

//create new image strcut
IMG* createImage( int height, int width);
IMG_COL* createImageColor(int height, int width);

//release image struct
void releaseImage( IMG **img);
void releaseImageColor( IMG_COL **img);

//clone image (create and convert);
IMG* cloneImage( const IMG* src, IMG *dst);

//convert image <--> matrix
void convertIMG2Mat( IMG* src, Mat* dst); 
void convertMat2IMG( Mat* src, IMG* dst);


#endif __IMAGE_DATA__
