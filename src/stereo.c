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

  printf("stereo recursive :  max disparity = %d, min disparity = %d\n", maxDisparity, minDisparity);

  if( maxDisparity <= minDisparity){//これ以上小さい視差へは投げない
    if( maxDisparity == 1)
      {
	IMG* dst = createImage( srcLeft->height, srcLeft->height);
	convertScaleImage(dst, dst, 0.0, 0.0);
	return dst;
      }
    else
      {
	return stereoInitialDisparityMap( srcLeft, srcRight, FundMat, maxDisparity);
      }

  }else{//小さい視差へ投げる
    IMG_COL *minLeft = createImageColor( srcLeft->height/2, srcLeft->width/2 );
    IMG_COL *minRight = createImageColor( srcRight->height/2, srcRight->width/2 );
    
    //resize
    for(int c = 0; c < 3; ++c){
      resizeImage( srcLeft->channel[c], minLeft->channel[c]);
      resizeImage( srcRight->channel[c], minRight->channel[c]);
    }

    //epipoler
    Mat minFund = matrixAllocDup( *FundMat);
    ELEM0( minFund, 0, 0) *= 4.0;
    ELEM0( minFund, 0, 1) *= 4.0;
    ELEM0( minFund, 0, 2) *= 2.0;
    ELEM0( minFund, 1, 0) *= 4.0;
    ELEM0( minFund, 1, 1) *= 4.0;
    ELEM0( minFund, 1, 2) *= 2.0;
    ELEM0( minFund, 2, 0) *= 2.0;
    ELEM0( minFund, 2, 1) *= 2.0;
    ELEM0( minFund, 2, 2) *= 1.0;

    //小さい視差での結果を得る
    IMG* dispMin = stereoRecursive( minLeft, minRight, &minFund, 
				    maxDisparity/2, minDisparity);

    //小さい視差での結果を受けて次のサイズの物を計算
    IMG* dst = stereoNextDisparityMap( srcLeft, srcRight, FundMat,
				       dispMin, maxDisparity,
				       srcLeft->height, srcLeft->width);

    //お掃除
    releaseImageColor(&minLeft);
    releaseImageColor(&minRight);
    matrixFree(minFund);
    releaseImage(&dispMin);

    return dst;

  }


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

    }//for w
  }//for h

  matrixFree(pt1);
  matrixFree(pt2);

  saveImage( initDispMap, "img/initialMap.png");

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

  printf("stereo next disparity map : previous size = %d, %d next size = %d, %d\n",
	 prevDispMap->height, prevDispMap->width, nextHeight, nextWidth);


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

      if( h%10 == 0 && w%10 == 0)
	printf("y = %lf x + %lf  ",-a/b, -c/b);

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

      if( h%10 == 0 && w%10 == 0)
	printf("%3d, %3d -> %d\n",h, w, (int)IMG_ELEM(dst,h,w));

    }//w
  }//h

  matrixFree(pt1);
  matrixFree(pt2);

  return dst;
}



