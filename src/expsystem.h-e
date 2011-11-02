#ifndef __EXP_SYSTEM__
#define __EXP_SYSTEM__

/*
  今までの実験とかのシステムをまとめたファイル
 */

#include "include.h"

// 現在の自分のシステムの奥行き推定システム
IMG* currentSystemDispmap
( IMG* srcLeft, 
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2][2],
  int maxDisparity,
  int blockSize
  );


// depth推定で、CAPairと同じ手法を採用した
IMG* currentSystemDispmap2
( IMG* srcLeft, 
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2][2],
  int maxDisparity,
  int blockSize
  );



// Coded aperture pair for DFD
// paramは奥行きとPSF径に関するパラメータ
// psfSize = param[0] * depth + param[1]になるように
IMG* CodedAperturePairDispmap
( IMG* srcLeft,
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2],
  int maxDepth,
  int blockSize
  );

IMG* CodedAperturePairDispmap2
( IMG* srcLeft,
  IMG* srcRight,
  IMG* apertureLeft,
  IMG* apertureRight,
  double param[2],
  int maxDepth,
  int blockSize
  );


// ピント位置を変更したDFD
// paramは奥行きとPSF径に関するパラメータ
// psfSize = param[0] * depth + param[1]になるように
IMG* DepthFromDeocus
( IMG* srcLeft,
  IMG* srcRight,
  IMG* aperture,
  double param[2][2], // param[0] = left, param[1] = right
  int blockSize
  );


#endif 
