/****************************************
 *stereo.c
 ****************************************/

#include "stereo.h"

point Point(int x, int y)
{
  point pt;
  pt.x = x;
  pt.y = y;
  return pt;
}

// noramlized cross corelation
// 正規化相関
double stereoEval( IMG_COL *srcLeft, IMG_COL* srcRight, 
		   point leftPt, point rightPt,
		   int blockSize)
{
  double result = 0.0;

  for( int c = 0 ; c < 3; ++c){

  double leftSum = 0.0;
  double rightSum = 0.0;
  double crossSum = 0.0;
    for( int h = -blockSize/2; h <= blockSize/2; ++h){
      for( int w = -blockSize/2; w <= blockSize/2; ++w){

	// region check
	if( leftPt.y + h < 0 || leftPt.y + h >= srcLeft->height || 
	    leftPt.x + w < 0 || leftPt.x + w >= srcLeft->width ||
	    rightPt.y + h < 0 || rightPt.y + h >= srcRight->height ||
	    rightPt.x + w < 0 || rightPt.x + w >= srcRight->width){
	  // out of allocated region
	}else{
	  double lv = IMG_ELEM( srcLeft->channel[c], leftPt.y + h , leftPt.x + w);
	  double rv = IMG_ELEM( srcRight->channel[c], rightPt.y + h , rightPt.x + w);
	  
	  leftSum += lv * lv;
	  rightSum += rv * rv;
	  crossSum += lv * rv;

	}//else 
      }//w
    }//h

    if( isnan( 1.0 / leftSum)  || isnan( 1.0 / rightSum )  ){
      result += 0.0;
    }else{
      result += crossSum / sqrt( leftSum * rightSum );
    }
  }//c

  //3channelで正規化相関を計算するので、
  //最も類似ている場合で3.0
  //argminを計算するので、3.0から引いた値を返す
  return 3.0 - result;

}

IMG* stereoRecursive( IMG_COL* srcLeft, 
		      IMG_COL* srcRight, 
		      Mat* FundMat, 
		      int maxDisparity, int minDisparity)
{
  


  return NULL;
}



IMG* stereoInitialDisparityMap( IMG_COL* srcLeft,
				IMG_COL* srcRight,
				Mat* FundMat,
				int maxDisparity)
{
  int height = srcLeft->height;
  int width = srcLeft->width;
  printf("stereo initial disparity map : size = %d * %d\n", height, width);
  
  // size check
  if(srcLeft->height != srcRight->height 
     || srcLeft->width != srcRight->width)
    {
      printf("input size is incorrect in stereoInitialDisparityMap \n");
      return NULL;
    }

  // disparity map ( to return)
  IMG* initDispMap = createImage(height, width);

  // vectores for epipoler
  Mat pt1 = matrixAlloc( 3, 1);
  Mat pt2 = matrixAlloc( 3, 1);

  int blockSize = MATCHING_BLOCK_SIZE;

  // stereo matching
  for(int  h = 0 ; h < height; ++h){
    for(int w = 0 ; w < width; ++w){

      point leftPoint = Point( w, h);
      double min = DBL_MAX;
      int count = 0;

      for( int x = w + 0; x < w + maxDisparity; ++x){
	
	// epipoler
	// a*x + b*y + c = 0
	ELEM0(pt1, 0, 0) = (double)w;
	ELEM0(pt1, 0, 1) = (double)h;
	ELEM0(pt1, 0, 2) = 1.0;
	
	matrixMul( pt2, *FundMat, pt1);
	double a = ELEM0(pt2, 0, 0);
	double b = ELEM0(pt2, 0, 1);
	double c = ELEM0(pt2, 0, 2);

	for( int y = (-a/b)*(double)x - c/b ; y <= (-a/b)*(double)(x+1) -c/b; ++y){
	  //範囲チェック
	  if( x < 0 || x >= width || y < 0 || y >= height || abs(x-w) > maxDisparity){
	    continue;
	  }else{
	    count++;
	    point rightPoint = Point(x, y);
	    double val = stereoEval( srcLeft, srcRight, leftPoint, rightPoint, blockSize);
	    if( val < min){
	      min = val;
	      IMG_ELEM( initDispMap, h, w) = (uchar)abs(x-w);
	    }//if
	  }//if
	}//for y
      }//for x

      if( w % 10 == 0 ){
	printf("%d, %d ->count = %d   ", h, w, count);
	printf("disparity = %d\n", (int)IMG_ELEM(initDispMap, h, w));
      }

    }//for w
  }//for h

  matrixFree(pt1);
  matrixFree(pt2);

  return initDispMap;

}

IMG* stereoNextDisparityMap( IMG_COL* srcLeft,
			     IMG_COL* srcRight,
			     Mat* FundMat,
			     IMG* prevDispMap,
			     int maxDisparity)
{
  return NULL;
}



