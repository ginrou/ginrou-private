#ifndef __DEPTH_ESTIMATION__
#define __DEPTH_ESTIMATION__

/*
  奥行き推定を行う関数が入っている
  depthといいつつ実はdisparityを計測する
  戻り値はIMG形式とMat形式にする予定

  blur base方式
  yL * fR(d) = yR * fL(d)
  となる性質を利用してdepthを求める
  残差 residual(d) = | yL*fR(d) - yR*fL(d) |
  を計算して、最も残差が小さかったdを採用する
  d = argmin_d ( redsidual(d) )

  deblur base 方式
  それぞれのpsfでdeblurを行うと，その元シーンは同じになるはず
  その性質を利用して奥行き推定を行う
  xL(d) = IFFT( yL*fL(d) / fL(d)^2 + SNR ) <-- deblur
  xR(d) = IFFT( yR*fR(d) / fR(d)^2 + SNR ) <-- deblur
  残差 residual(d) = | xL(d) - xR(d) |
  を計算し、最もこの残差が小さかったdを奥行きとする
  d = argmin_d ( residual(d) )

  latent base 方式
  ふたつのPSF用いて元シーンをフーリエ領域で求めてから、
  PSFを畳み込んでその残差を計算する
  X(d) = YL*FL(d) + YR*FR(d) / FL(d)^2 + FR(d)^2 + SNR  
  residual(d) = | YL - X(d)*FL(d) + YR - X(d)*FR(d) |
  残差最小となるdが奥行き
  d = argimin_d ( residual(d) )

 */


#include "include.h"

IMG* blurBaseEstimationIMG(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);
IMG* deblurBaseEstimationIMG(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);
IMG* latentBaseEstimationIMG( IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);

Mat blurBaseEstimationMat(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);
Mat deblurBaseEstimationMat(IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);
Mat latentBaseEstimationMat( IMG* left, IMG* right, Mat psfLeft[], Mat psfRight[]);


#endif
