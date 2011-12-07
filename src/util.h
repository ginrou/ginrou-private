/*****************************************
    util.h
画像の表示とか
時間の計測とか
 ****************************************/

#ifndef __UTIL__
#define __UTIL__


#include "include.h"

// 確認用に表示
void showImage(  const IMG *img, int keyWait);
void showDispMap( const IMG* img);

// テキストデータを画像に変換
/*
  h, w, 輝度値　
  形式で画素とその輝度値に入っているテキストデータを
  画像ファイルとして保存
 */
IMG* textToIMG( char filename[], int height, int width, double scale);

// 計算時間管理
void startClock(void);
double getPassedTime(void);
void printPassedTime(void);

// 手前の一行を消す
#define _ClearLine() { fputs("\r\x1b[2K", stdout); fflush(stdout); }
// 直前の行を一行消す
#define _ClearRetLine() { fputs("\r\x1b[1A", stdout);fputs("\r\x1b[2K", stdout);fflush(stdout);}

#endif
