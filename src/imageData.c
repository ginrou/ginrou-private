/*****************************************
  imageData.c
 *****************************************/
#include "imageData.h"

IMG* readImage( char* filename )
{
  printf("readImage : filename = %s \n", filename);
  IplImage* img = cvLoadImage( filename , CV_LOAD_IMAGE_GRAYSCALE);
  if(img == NULL)
    {
      printf("error in loading image... filename %s \n", filename);
      return NULL;
    }
  IMG* retImg = createImage(img->height, img->width);
  convertIpl2IMG(img, retImg);
  cvReleaseImage(&img);
  return retImg;
}

IMG_COL* readImageColor( char* filename )
{
  printf("readImageColor : filename = %s \n", filename);
  IplImage* img = cvLoadImage( filename , CV_LOAD_IMAGE_COLOR);
  if(img == NULL)
    {
      printf("error in loading image... filename %s \n", filename);
      return NULL;
    }
  IMG_COL* retImg = createImageColor(img->height, img->width);
  IplImage* imgSplit[3];
  for(int c = 0; c < 3; ++c)
    imgSplit[c] = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);

  cvSplit(img, imgSplit[0], imgSplit[1], imgSplit[2], NULL);
  for(int c = 0; c <3; ++c)
    {
      convertIpl2IMG(imgSplit[c], retImg->channel[c]);
      cvReleaseImage(&(imgSplit[c]));
    }
  cvReleaseImage(&img);
  return retImg;
}

void convertIMG2Ipl( const IMG* src, IplImage *dst)
{
  //size check
  if(src == NULL || dst == NULL){
    printf("convert image should'nt be NULL\n");
    return;
  }
  if(src->height != dst->height || src->width != dst->width){
    printf("size should be same\n");
    return;
  }

  //copying
  for(int h = 0; h < src->height; ++h)
    {
      for( int w =0 ; w < src->width; ++w)
	{
	  for(int c = 0; c < dst->nChannels; ++c)
	    {
	      CV_IMAGE_ELEM(dst, uchar, h, w*dst->nChannels+c) 
		= IMG_ELEM(src, h, w);
	    }
	}
    }

  return;

}

void convertIpl2IMG( const IplImage* src, IMG* dst)
{
  //size check
  if(src == NULL || dst == NULL)
    {
      printf("convert image should'nt be NULL\n");
      return;
    }
  if(src->height != dst->height || src->width != dst->width)
    {
      printf("size should be same\n");
      return;
    }

  //copying
  for(int h = 0; h < src->height; ++h)
    {
      for(int w = 0; w < src->width; ++w)
	{
	  IMG_ELEM(dst, h, w) 
	    = CV_IMAGE_ELEM(src, uchar, h, w*src->nChannels +0);
	}
    }

  return;

}




IMG* createImage( int height, int width)
{
  IMG* img = malloc(sizeof(IMG));
  if(img == NULL)
    {
      printf("error in allocating img \n");
      return NULL;
    }

  img->height = height;
  img->width = width;
  img->data = (unsigned char*)malloc(height*width*sizeof(unsigned char));

  if(img->data == NULL)
    {
      printf("allocation error in img->data\n");
      return NULL;
    }

  return img;
}


IMG_COL* createImageColor(int height, int width)
{
  IMG_COL* imgCol = malloc(sizeof(IMG_COL));
  if(imgCol == NULL)
    {
      printf("error in allocationg memory IMG_COL\n");
      return NULL;
    }

  imgCol->height = height;
  imgCol->width = width;

  for(int i = 0; i<3;++i)
    {
      imgCol->channel[i] = createImage(height, width);
      if(imgCol->channel[i] == NULL)
	{
	  printf("allocation error\n");
	  return NULL;
	}
    }

  return imgCol;
}

void releaseImage( IMG **img)
{
  free((*img)->data);
  free(*img);
  *img = NULL;
}


void releaseImageColor( IMG_COL **img)
{
  for(int i = 0; i < 3; ++i)
    {
      releaseImage(&((*img)->channel[i]));
    }
  free(*img);
  *img = NULL;
}


IMG* cloneImage( const IMG* src, IMG *dst)
{
  //error check
  if(src == NULL)
    {
      printf("src is NULL at cloneImage\n");
      return NULL;
    }

  IMG* tmp = createImage( src->height, src->width);
  
  for(int h  = 0; h < tmp->height; ++h)
    {
      for( int w = 0; w < tmp->width ; ++w)
	{
	  IMG_ELEM(tmp, h, w) = IMG_ELEM(src, h, w);
	}
    }

  return dst=tmp;

}

IMG* cloneImageFromMat( Mat mat)
{
  IMG* ret = createImage( mat.row, mat.clm );
  convertMat2IMG( &mat, ret );  
  return ret;
}

Mat  cloneMatFromImage( IMG* img)
{
  Mat ret = matrixAlloc( img->height, img->width );
  convertIMG2Mat( img, &ret);
  return ret;
}

void saveImage( IMG* img, char *filename)
{
  if(img==NULL || filename == NULL ) return;

  IplImage *buf = cvCreateImage( cvSize( img->width, img->height), IPL_DEPTH_8U, 1);
  convertIMG2Ipl(img, buf);
  if( cvSaveImage( filename, buf, 0) )
    printf("%s is saved\n", filename);
  else
    printf("%s cannot save\n", filename);

  cvReleaseImage(&buf);
  
  return;
}

void saveImageColor( IMG_COL *img, char *filename)
{

  if(img==NULL || filename == NULL ) return;

  CvSize sz = cvSize( img->width, img->height);
  IplImage* buf = cvCreateImage( sz, IPL_DEPTH_8U, 3);
  IplImage* tmp[3];

  for( int c = 0 ; c < 3 ; ++c)
    {
      tmp[c] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
      convertIMG2Ipl( img->channel[c], tmp[c]);
    }
  cvMerge( tmp[0], tmp[1], tmp[2], NULL, buf);
  int result = cvSaveImage( filename, buf, 0);

  for(int c = 0; c < 3 ; ++c)
    cvReleaseImage( &(tmp[c]) );
  cvReleaseImage(&buf);
  return;
}


IMG_COL* convertIMG2IMG_COL( const IMG* src)
{
  IMG_COL* dst = createImageColor( src->height, src->width);
  for(int c = 0; c < 3; ++c ){
    cloneImage( src , dst->channel[c]);
  }
  return dst;
}

IMG* convertIMG_COL2IMG( const IMG_COL* src)
{
  IMG* dst = createImage( src->height, src->width);
  for(int h = 0; h < dst->height; ++h){
    for( int w = 0 ;w < dst->width; ++w){
      double sum = 0.0;
      for( int c = 0; c < 3; ++c)
	sum += IMG_ELEM( src->channel[c], h, w);
      IMG_ELEM( dst, h, w) = sum / 3.0;
    }
  }
  return dst;
}



void convertIMG2Mat( IMG* src, Mat* dst)
{
  if( dst == NULL ) return;
  if( dst->row != src->height || dst->clm != src->width) return;
  
  for(int h = 0; h < dst->row; ++h){
    for(int w = 0 ; w < dst->clm; ++w){
      ELEM0(*dst, h, w) = IMG_ELEM( src, h, w);
    }
  }
  return;
}

void convertMat2IMG( Mat* src, IMG* dst)
{
 if( dst == NULL || src == NULL )return;
 if( dst->height != src->row || dst->width != src->clm ) return;

 for(int h = 0; h < dst->height ; h++){
   for( int w = 0 ; w < dst->width ; ++w){
     IMG_ELEM( dst, h, w) = (unsigned char)ELEM0(*src, h, w);
   }
 }
 return;
}
