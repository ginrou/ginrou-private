#ifndef __BATCH__
#define __BATCH__

#include "include.h"

int batch110731( int argc, char* argv[] );
int batch110801( int argc, char* argv[] );
int batch110801_2( int argc, char* argv[] );
int batch110802( int argc, char* argv[] );
int batch110804( int argc, char* argv[] ); // delburベースの奥行き推定
int batch110808( int argc, char* argv[] ); // stereo deblurの実験評価
int batch110809( int argc, char* argv[] ); // stereo deblurの実験評価
int batch110814( int argc, char* argv[] ); // 修論用システム完成!
int batch110815( int argc, char* argv[] ); // 修論用システムの実験
int batch110816( int argc, char* argv[] ); // 修論用システムの実験2 CAPairを改良

int batch110827( int argc, char* argv[] ); // 周波数領域におけるカーネルの線形補間を用いる事で中途半端なサイズのカーネルでもdeblurできるようになった

int batch110828_1( int argc, char* argv[] ); // サブピクセルオーダーの視差マップを用いる事で、より高精度のぼけ除去を行えるように

int batch111018( int argc, char* argv[] ); // バッチ処理 を行えるようにした

int batch_deblurTestCode( int argc, char* argv[]);
int parameterCalibration( int argc, char* argv[]);

#endif
