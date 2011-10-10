まだReadmeファイルはかけてません

久しぶりに触ったときに思い出せるように関数とか色々書いとく


ディレクトリについて
bin/ コンパイルしたオブジェクトファイルが入ってる
img/ 実験で使う画像とかを入れておく
lib/ 日浦先生に頂いたソースが入ってる
     主に使ってるのは matrix.hとか
src/ 自分で書いたソースが入ってるのはだいたいここ

各ソースコードについて

////////////////////////////////////////////////
batch
実験のバッチ処理を行うソースが入ってる。ほとんどスクリプト的な使い方
////////////////////////////////////////////////

////////////////////////////////////////////////
blur
画像のぼかす(blur)処理を行う
IMG* blur(IMG *img, IMG* psf); 畳み込み演算でぼけを生成
IMG* blurFilter( IMG *img, IMG *psf); フーリエ変換をつかってぼかす
IMG* blurWithPSFMap( IMG* img, Mat psf[], IMG* psfMap); 各画素ごとにぼけカーネルが違う
////////////////////////////////////////////////

////////////////////////////////////////////////
deblur
日浦先生のFFTライブラリを用いてdeblurを行う
どうもバグがあるっぽい
////////////////////////////////////////////////

////////////////////////////////////////////////
deblur2
オープンソースのFFTライブラリ、FFTWを用いてdeblurを行う

IMG* deblurFFTW( IMG* img, IMG* psf)
 最も基本的な関数で、IMG形式の観測画像とPSFを渡してdelburを行う。
 imgとPSFは決して同じサイズでなくてもよい
////////////////////////////////////////////////

////////////////////////////////////////////////
expsystem
今までの実験とかのシステムを残しておいてまとめたファイル

IMG* currentSystemDispmap
　現在の自分のシステムの奥行き推定システム


IMG* currentSystemDispmap2
　depth推定で、CAPairと同じ手法を採用した

IMG* CodedAperturePairDispmap
 Coded aperture pair for DFD
 paramは奥行きとPSF径に関するパラメータ
 psfSize = param[0] * depth + param[1]になるように


IMG* CodedAperturePairDispmap2
　上の関数を書き直した

IMG* DepthFromDeocus
 ピント位置を変更したDFD
 paramは奥行きとPSF径に関するパラメータ
 psfSize = param[0] * depth + param[1]になるように
////////////////////////////////////////////////


////////////////////////////////////////////////
imageData
画像構造体IMG の定義とそのユーテリティ関数を集めたファイル
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
////////////////////////////////////////////////


////////////////////////////////////////////////
imageProcessing
簡単だけどよく使うような処理を集めたファイル
resizeImage(const  IMG* src, IMG* dst)
 画像のリサイズ(拡大縮小)

double imageNormL1(const IMG* img)
 L1ノルムの計算

void normalizeMat(Mat src, Mat dst)
 L1ノルムで正規化

void convertScaleImage( const IMG* src,  IMG* dst, double scale, double shift)
 画像の線形変換 dst[i] = scale * img[i] + shift

void putnoise(const IMG* src, IMG* dst, double mean, double var)
 平均mean, 分散varのホワイトノイズを付加

void flipImage( IMG* img, int horizontal, int vertcial);
 画像をひっくり返す

////////////////////////////////////////////////



////////////////////////////////////////////////
include.h
インクルードファイルをまとめて指定するファイル
ほかにも#defineの定義はこの辺にまとめる
////////////////////////////////////////////////


////////////////////////////////////////////////
psf
PSF関連の処理をいろいろと入れておく。k
大本のPSFからリサイズとシフトをしながらパラメータ通りのpsfを作って行くのが主な仕事

void makeShiftPSF(Mat psf[MAX_DISPARITY], int cam);
 視差が0からMAX_DISPARITYまでの間でのshiftを作る

void makeBlurPSF( IMG* psf[MAX_DISPARITY], IMG* aperture, int maxDepth, double param[2]);
 ぼけ径だけを考慮したPSFをつくる

void makeShiftBlurPSF( Mat psf[MAX_DISPARITY], int cam, IMG* aperture, double par[2]);
 shiftとresizeの両方を考慮した場合

void makeShiftBlurPSFFreq( int height, int width, int cam, fftw_complex* dst[MAX_DISPARITY], IMG* aperture, double param[2] );
 周波数領域でのPSFを作って返す
////////////////////////////////////////////////


////////////////////////////////////////////////
stereo
ステレオ法の実装
アルゴリズムとしては、ピラミッド法に近いと思う
原画像の1/4スケールの画像を作り、再帰呼び出しを行う
十分小さくなったらウィンドウマッチングで対応点を探索
得られた結果を元に，一つ小さいサイズでの視差の前後を探索する
IMG* stereoRecursive( IMG_COL* srcLeft, IMG_COL* srcRight, Mat* FundMat, int maxDisparity, int minDisparity);
 この関数がステレオ法実装のメイン

IMG* stereoInitialDisparityMap( IMG_COL* srcLeft, IMG_COL* srcRight,Mat* FundMat,int maxDisparity);
 初期解の生成

IMG* stereoNextDisparityMap( IMG_COL* srcLeft, IMG_COL* srcRight, Mat* FundMat,IMG* prevDispMap,int maxDisparity,int nextHeight,int nextWidth);
 再帰呼び出し
////////////////////////////////////////////////


////////////////////////////////////////////////
util
実験とかをしやすくするためのユーティリティ

void showImage(  const IMG *img, int keyWait);
 画像を表示

void showDispMap( const IMG* img);
 視差マップを表示

void startClock(void);
 計算時間の計測スタート

double getPassedTime(void);
 経過した時間[ミリ秒]を取得

void printPassedTime(void);
 経過した時間を表示

// 手前の一行を消す
#define _ClearLine() { fputs("\r\x1b[2K", stdout); fflush(stdout); }
////////////////////////////////////////////////


