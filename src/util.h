/*****************************************
    util.h
画像の表示とか
時間の計測とか
 ****************************************/

#ifndef __UTIL__
#define __UTIL__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


#include <cv.h>
#include <highgui.h>

#include "imageData.h"

void showImage(  const IMG *img, int keyWait);
void showDispMap( const IMG* img);
void startClock(void);
double getPassedTime(void);
void printPassedTime(void);

// 手前の一行を消す
#define _ClearLine() { fputs("\r\x1b[2K", stdout); fflush(stdout); }


#endif
