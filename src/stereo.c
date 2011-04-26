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
	  double lv = (double)IMG_ELEM( srcLeft->channel[c], leftPt.y + h , leftPt.x + w);
	  double rv = (double)IMG_ELEM( srcRight->channel[c], rightPt.y + h , rightPt.x + w);
	  
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



//再帰的にステレオマッチングを行う
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

	if( h%10 == 0 && w%10 == 0){
	  if(x==w)printf("(h, w) = (%3d, %3d)\n", h, w);
	  int y =(-a/b)*(double)x - c/b;
	  printf("\t(y,x) = (%3d,%3d) -> ",y ,x);
	  y = (-a/b)*(double)(x+1) - c/b; 
	  printf("(%3d,%3d) \n",y ,x+1);
	}

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
			     int maxDisparity,
			     int nextHeight,
			     int nextWidth)
{
  //サイズチェック
  if(srcLeft->width != srcRight->width ||
     srcLeft->height != srcRight->height)
    {
      printf("error in stereoNextDisparityMap\n");
      printf("size is incorrect srcLeft-> %d * % d ", srcLeft->width, srcLeft->height);
      printf("srcRight-> %d * %d\n return NULL \n", srcRight->width , srcRight->height);
      return NULL;
    }

  //視差マップ( prevDispmap )をコンバート
  IMG* dispMap = createImage( nextHeight, nextWidth );
  resizeImage( prevDispMap, dispMap);


  //結果格納用
  IMG* dst = createImage( nextHeight, nextWidth);

  //その他の変数
  //エピポーラまわり
  double a,b,c;
  Mat pt1 = matrixAlloc( 3,1);
  Mat pt2 = matrixAlloc( 3,1);

  int blkSize = MATCHING_BLOCK_SIZE;  //ブロックサイズ
  int searchWidth;//探索幅
  int prevDisparity;
  point leftPt, rightPt;


  for(int h = 0 ; h < dst->height; ++h){
    for( int w = 0; w < dst->width; ++w){

      leftPt.x = w;
      leftPt.y = h;

      //エピポーラ線
      ELEM0( pt1, 0, 0 ) = (double)w;
      ELEM0( pt1, 0, 1 ) = (double)h;
      ELEM0( pt1, 0, 2 ) = 1.0;
	   
      matrixMul( pt2, *FundMat, pt1);
      a = ELEM0(pt2, 0, 0);
      b = ELEM0(pt2, 0, 1);
      c = ELEM0(pt2, 0, 2);

      prevDisparity = (int)IMG_ELEM( dispMap, h, w);

      //探索幅の決定
      //探索幅が3となる条件：以前の探索では存在しなかった領域
      //maxDisparityが奇数でかつprevmaxDisaprityがmaxDisparity-1となる場合
      //またはprevmaxDisparityが1の場合
      if( maxDisparity % 2 == 1 && ( maxDisparity/2 == 1 || prevDisparity == maxDisparity/2-1)){
	searchWidth = 3;
      }else{
	searchWidth = 2;
      }
	
      //探索
      double min = DBL_MAX;
      double val;
      for( int x = w + prevDisparity*2 - 1; x < w+prevDisparity*2 +searchWidth; ++x){
	for( int y = (-a/b)*(double)x -c/b ; y < (-a/b)*(double)(x+1) -c/b ; ++y){

	  rightPt.x = x;
	  rightPt.y = y;
	  
	  //範囲チェック
	  if( x < 0 || x >= dst->width || y < 0 || y >= dst->height ||
	      abs(x-w) > maxDisparity)
	    {
	      continue;
	    }else
	    {
	      val = stereoEval(srcLeft, srcRight, leftPt, rightPt, blkSize);
	    }

	  if( val < min ){
	    min = val;
	    IMG_ELEM(dst, h, w) = (uchar)abs(x-w);
	  }

	}//x
      }//y

    }//x
  }//y

  matrixFree(pt1);
  matrixFree(pt2);

  return dst;
}



