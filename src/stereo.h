#ifndef __STEREO__
#define __STEREO__

#include <stdio.h>
#include <float.h>

#include <matrix.h>

#include <imageData.h>
#include <imageProcessing.h>
#include <util.h>


#define MATCHING_BLOCK_SIZE   (2 * maxDisparity)

typedef struct point{
  int x;
  int y;
}point;
point Point(int x, int y);

IMG* stereoRecursive( IMG_COL* srcLeft, 
		      IMG_COL* srcRight, 
		      Mat* FundMat, 
		      int maxDisparity, int minDisparity);


IMG* stereoInitialDisparityMap( IMG_COL* srcLeft,
				IMG_COL* srcRight,
				Mat* FundMat,
				int maxDisparity);

IMG* stereoNextDisparityMap( IMG_COL* srcLeft,
			     IMG_COL* srcRight,
			     Mat* FundMat,
			     IMG* prevDispMap,
			     int maxDisparity,
			     int nextHeight,
			     int nextWidth);


// argmin not argmax
// noramlized cross corelation
// 正規化相関
double stereoEval( IMG_COL *srcLeft, IMG_COL* srcRight, 
		   point leftPt, point rightPt,
		   int blockSize);
		   
			    


#endif
