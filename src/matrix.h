/*******************************************************
 *   matrix.h  matrix handring library header
 *
 * $Author: shinsaku $
 * $Revision: 1.3 $
 * $Date: 94/11/29 14:26:29 $
 * $Log:	matrix.h,v $
 * Revision 1.3  94/11/29  14:26:29  shinsaku
 * change matrix structure name
 * from Matrix to Mat.
 * 
 * Revision 1.2  94/11/10  23:20:53  shinsaku
 * support multiple including.
 * 
 * Revision 1.1  94/11/09  22:19:36  shinsaku
 * Initial revision
 * 
 *
*******************************************************/
#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

/* === matrix definition ===

  <---- clm --->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |
  [ 13  14  15 ] v

=========================== */

typedef struct {
	double *m;
	int row;
	int clm;
} Mat;

typedef struct {
        double *v;
        int    clm;
} Vec;


/* 0 origin */
#define ELEM0(mat,r,c) ((mat).m[(r)*(mat).clm+(c)])
/* 1 origin */
#define ELEM1(mat,row,clm) ELEM0(mat,row-1,clm-1)
#define ERR_MAT(mat)  ((mat).row==0||(mat).clm==0)
#define ERR_VEC(vec)  ((vec).clm==0)

#ifdef __cplusplus
extern "C" {
#endif

Mat    matrixAlloc(int row, int clm);
void   matrixFree(Mat m);
int    matrixDup(Mat dest, Mat source);
Mat    matrixAllocDup(Mat source);
int    matrixUnit(Mat unit);
void   matrixZero(Mat m);
Mat    matrixAllocUnit(int dim);
int    matrixMul(Mat dest, Mat a, Mat b);
Mat    matrixAllocMul(Mat a, Mat b);
int    matrixTrans(Mat dest, Mat source);
Mat    matrixAllocTrans(Mat source);
int    matrixSelfInv(Mat m);
int    matrixInv(Mat dest, Mat source);
Mat    matrixAllocInv(Mat source);
double matrixDet(Mat m);
void   matrixDisp(Mat m);
void   matrixDispSmall(Mat m);
int    matrixSave(Mat m, char *fname);
Mat    matrixAllocLoad(char *fname);
int    matrixPCA( Mat input, Mat evec, Vec ev, Vec mean );
int    matrixPCAnoMean( Mat input, Mat evec, Vec ev);
int    matrixPCA2( Mat input, Mat evec, Vec ev, Vec mean );
int    matrixPCA3( Mat input, Mat evec, Vec ev, Vec mean );

Vec    vecAlloc( int clm );
void   vecFree( Vec v );
void   vecDisp( Vec v );
double vecHousehold( Vec x );
double vecInnerproduct( Vec x, Vec y );
int    vecTridiagonalize( Mat a, Vec d, Vec e );

#ifdef __cplusplus
}
#endif

#endif
