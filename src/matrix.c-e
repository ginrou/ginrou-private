#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"

Mat matrixAlloc(int row, int clm) {
	Mat m;

	m.m = (double *)malloc(sizeof(double) * row * clm);
	if(m.m == NULL) {
		m.row = m.clm = 0;
	}
	else {
		m.row = row;
		m.clm = clm;
	}
	return m;
}

Mat matrixAllocDup(Mat source) {
	Mat dest;

	dest = matrixAlloc(source.row, source.clm);
	matrixDup(dest, source);
	return dest;
}

Mat matrixAllocInv(Mat source) {
	Mat dest;

	dest = matrixAlloc(source.row, source.row);
	if(matrixInv(dest, source)) {
		dest.row = dest.clm = 0;
	}
	return dest;
}

Mat matrixAllocLoad(char *fname) {
	int row, clm;
	Mat m;
	FILE *fp;

	m.row = m.clm = 0;  /* m is ERR_MAT */
	fp = fopen(fname, "r");
	if(fp == NULL) {
		return m;
	}
	fread(&row, sizeof(int), 1, fp);
	fread(&clm, sizeof(int), 1, fp);
	m = matrixAlloc(row, clm);
	fread(m.m, sizeof(double), row * clm, fp);
	fclose(fp);

	return m;
}

Mat matrixAllocMul(Mat a, Mat b) {
	Mat dest;

	dest = matrixAlloc(a.row, b.clm);
	matrixMul(dest, a, b);
	return dest;
}

Mat matrixAllocTrans(Mat source) {
	Mat dest;

	dest = matrixAlloc(source.clm, source.row);
	matrixTrans(dest, source);
	return dest;
}

Mat matrixAllocUnit(int dim) {
	Mat m;

	m = matrixAlloc(dim, dim);
	matrixUnit(m);
	return m;
}

#define MATRIX(name,x,y,width)  ( *(name + (width) * (x) + (y)) )

static double mdet( double *ap, int dimen, int rowa );

double matrixDet(Mat m) {
	if(m.row != m.clm) {
		return 0;
	}
	return mdet(m.m, m.row, m.row);
}

/*
double  *ap;            input matrix
int     dimen;          Dimension of linre and row, those must be equal,
                        that is square matrix.
int     rowa;           ROW Dimension of matrix A
*/
static double mdet(double *ap, int dimen, int rowa)
{
    double det = 1.0;
    double work;
    int    is = 0;
    int    mmax;
    int    i, j, k;

    for(k = 0; k < dimen - 1; k++) {
        mmax = k;
        for(i = k + 1; i < dimen; i++)
            if (fabs(MATRIX(ap, i, k, rowa)) > fabs(MATRIX(ap, mmax, k, rowa)))
                mmax = i;
        if(mmax != k) {
            for (j = k; j < dimen; j++) {
                work = MATRIX(ap, k, j, rowa);
                MATRIX(ap, k, j, rowa) = MATRIX(ap, mmax, j, rowa);
                MATRIX(ap, mmax, j, rowa) = work;
            }
            is++;
        }
        for(i = k + 1; i < dimen; i++) {
            work = MATRIX(ap, i, k, rowa) / MATRIX(ap, k, k, rowa);
            for (j = k + 1; j < dimen; j++)
                MATRIX(ap, i, j, rowa) -= work * MATRIX(ap, k, j, rowa);
        }
    }
    for(i = 0; i < dimen; i++)
        det *= MATRIX(ap, i, i, rowa);
    for(i = 0; i < is; i++) 
        det *= -1.0;
    return(det);
}

void matrixDisp(Mat m) {
	int r, c;

	if(ERR_MAT(m)) {
		printf("ERROR MATRIX\n");
		return;
	}
	printf(" === matrix (%d,%d) ===\n", m.row, m.clm);
	for(r = 0; r < m.row; r++) {
		printf(" |");
		for(c = 0; c < m.clm; c++) {
			printf(" %10g", ELEM0(m, r, c));
		}
		printf(" |\n");
	}
	printf(" ======================\n");
}

void matrixDispSmall(Mat m) {
	int r, c;

	if(ERR_MAT(m)) {
		printf("ERROR MATRIX\n");
		return;
	}
	printf(" === matrix (%d,%d) ===\n", m.row, m.clm);
	for(r = 0; r < m.row; r++) {
		printf(" |");
		for(c = 0; c < m.clm; c++) {
			printf(" %4g", ELEM0(m, r, c));
		}
		printf(" |\n");
	}
	printf(" ======================\n");
}

int matrixDup(Mat dest, Mat source) {
	int r,c;

	if(dest.row != source.row || dest.clm != source.clm) {
		return 1;
	}
	for(r = 0; r < source.row; r++) {
		for(c = 0; c < source.clm; c++) {
			ELEM0(dest, r, c) = ELEM0(source, r, c);
		}
	}
	return 0;
}

void matrixFree(Mat m) {
	free(m.m);
}

int matrixInv(Mat dest, Mat source) {
	if(matrixDup(dest, source)) {
		return 1;
	}
	return matrixSelfInv(dest);
}

int matrixMul(Mat dest, Mat a, Mat b) {
	int r, c, i;

	if(a.clm != b.row || dest.row != a.row || dest.clm != b.clm) {
		return 1;
	}
	for(r = 0; r < dest.row; r++) {
		for(c = 0; c < dest.clm; c++) {
			ELEM0(dest, r, c) = 0.0;
			for(i = 0; i < a.clm; i++) {
				ELEM0(dest, r, c) += ELEM0(a, r, i) * ELEM0(b, i, c);
			}
		}
	}
	return 0;
}

#define     VZERO           1e-16
#define     EPS             1e-6
#define     MAX_ITER        100
#define     xmalloc(V,T,S)  { if( ((V) = (T *)malloc( sizeof(T) * (S) ))\
                               == NULL ) exit(1); }

static int EX( Mat input, Vec mean );
static int CENTER( Mat inout, Vec mean );
static int PCA( Mat input, Mat output, Vec ev );
static int x_by_xt( Mat input, Mat output );
static int xt_by_x( Mat input, Mat output );
static int EV_create( Mat input, Mat u, Mat output, Vec ev );
static int QRM( Mat u, Vec ev );


/* === matrix definition ===

Input:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Sample number)
  [ 13  14  15 ] v

Evec:
  <---- clm (Eigen vector)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Number of egen vec)
  [ 13  14  15 ] v

Ev:
  <---- clm (Number of eigen vector)--->
  [ 10  20  30 ] eigen value

Mean:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] mean value

=========================== */


int matrixPCA( Mat input, Mat evec, Vec ev, Vec mean )
{
    Mat     work;
    double  srow, sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input.row;
    clm = input.clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec.clm != input.clm || evec.row != check ) return(-1);
    if( ev.clm   != check )     return(-1);
    if( mean.clm != input.clm ) return(-1);

    work = matrixAllocDup( input );
    if( work.row != row || work.clm != clm ) return(-1);

    srow = sqrt((double)row);
    if( EX( work, mean ) < 0 ) {
        matrixFree( work );
        return(-1);
    }
    if( CENTER( work, mean ) < 0 ) {
        matrixFree( work );
        return(-1);
    }
    for(i=0; i<row*clm; i++) work.m[i] /= srow;

    rval = PCA( work, evec, ev );
    matrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev.clm; i++ ) sum += ev.v[i];
    for( i = 0; i < ev.clm; i++ ) ev.v[i] /= sum;

    return( rval );
}

int matrixPCAnoMean( Mat input, Mat evec, Vec ev )
{
    Mat     work;
    double  srow, sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input.row;
    clm = input.clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec.clm != input.clm || evec.row != check ) return(-1);
    if( ev.clm   != check )     return(-1);

    work = matrixAllocDup( input );
    if( work.row != row || work.clm != clm ) return(-1);

    srow = sqrt((double)row);
    for(i=0; i<row*clm; i++) work.m[i] /= srow;

    rval = PCA( work, evec, ev );
    matrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev.clm; i++ ) sum += ev.v[i];
    for( i = 0; i < ev.clm; i++ ) ev.v[i] /= sum;

    return( rval );
}

static int PCA( Mat input, Mat output, Vec ev )
{
    Mat     u;
    double  *m1, *m2;
    int     row, clm, min;
    int     i, j;

    row = input.row;
    clm = input.clm;
    min = (clm < row)? clm: row;
    if( row < 2 || clm < 2 )      return(-1);
    if( output.clm != input.clm ) return(-1);
    if( output.row != min )       return(-1);
    if( ev.clm != min )           return(-1);

    u = matrixAlloc( min, min );
    if( u.row != min || u.clm != min ) return(-1);
    if( row < clm ) {
        if( x_by_xt( input, u ) < 0 ) { matrixFree(u); return(-1); }
    }
    else {
        if( xt_by_x( input, u ) < 0 ) { matrixFree(u); return(-1); }
    }

    if( QRM( u, ev ) < 0 ) { matrixFree(u); return(-1); }

    if( row < clm ) {
        if( EV_create( input, u, output, ev ) < 0 ) {
            matrixFree(u);
            return(-1);
        }
    }
    else{
        m1 = u.m;
        m2 = output.m;
	for( i = 0; i < min; i++){
	    if( ev.v[i] < VZERO ) break;
            for( j = 0; j < min; j++ ) *(m2++) = *(m1++);
        }
	for( ; i < min; i++){
            ev.v[i] = 0.0;
            for( j = 0; j < min; j++ ) *(m2++) = 0.0;
        }
    }

    matrixFree(u);

    return( 0 );
}

static int EX( Mat input, Vec mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = input.row;
    clm = input.clm;
    if( row <= 0 || clm <= 0 ) return(-1);
    if( mean.clm != clm )      return(-1);

    for( i = 0; i < clm; i++ ) mean.v[i] = 0.0;

    m = input.m;
    for( i = 0; i < row; i++ ) {
        v = mean.v;
        for( j = 0; j < clm; j++ ) *(v++) += *(m++);
    }

    for( i = 0; i < clm; i++ ) mean.v[i] /= row;

    return(0);
}

static int CENTER( Mat inout, Vec mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = inout.row;
    clm = inout.clm;
    if( mean.clm != clm ) return(-1);

    m = inout.m;
    for( i = 0; i < row; i++ ) {
        v = mean.v;
        for( j = 0; j < clm; j++ ) *(m++) -= *(v++);
    }

    return(0);
}

static int x_by_xt( Mat input, Mat output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input.row;
    clm = input.clm;
    if( output.row != row || output.clm != row ) return(-1);

    out = output.m;
    for( i = 0; i < row; i++ ) {
        for( j = 0; j < row; j++ ) {
            if( j < i ) {
                *out = output.m[j*row+i];
            }
            else {
                in1 = &(input.m[clm*i]);
                in2 = &(input.m[clm*j]);
                *out = 0.0;
                for( k = 0; k < clm; k++ ) {
                    *out += *(in1++) * *(in2++);
                }
            }
            out++;
        }
    }

    return(0);
}

static int xt_by_x( Mat input, Mat output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input.row;
    clm = input.clm;
    if( output.row != clm || output.clm != clm ) return(-1);

    out = output.m;
    for( i = 0; i < clm; i++ ) {
        for( j = 0; j < clm; j++ ) {
            if( j < i ) {
                *out = output.m[j*clm+i];
            }
            else {
                in1 = &(input.m[i]);
                in2 = &(input.m[j]);
                *out = 0.0;
                for( k = 0; k < row; k++ ) {
                    *out += *in1 * *in2;
                    in1 += clm;
                    in2 += clm;
                }
            }
            out++;
        }
    }

    return(0);
}

static int EV_create( Mat input, Mat u, Mat output, Vec ev )
{
    double  *m, *m1, *m2;
    double  sum, work;
    int     row, clm;
    int     i, j, k;

    row = input.row;
    clm = input.clm;
    if( row <= 0 || clm <= 0 ) return(-1);
    if( u.row != row || u.clm != row ) return(-1);
    if( output.row != row || output.clm != clm ) return(-1);
    if( ev.clm != row ) return(-1);

    m = output.m;
    for( i = 0; i < row; i++ ) {
        if( ev.v[i] < VZERO ) break;
        work = 1 / sqrt(fabs(ev.v[i]));
        for( j = 0; j < clm; j++ ) {
            sum = 0.0;
            m1 = &(u.m[i*row]);
            m2 = &(input.m[j]);
            for( k = 0; k < row; k++ ) {
                sum += *m1 * *m2;
                m1++;
                m2 += clm;
            }
            *(m++) = sum * work;
        }
    }
    for( ; i < row; i++ ) {
        ev.v[i] = 0.0;
        for( j = 0; j < clm; j++ ) *(m++) = 0.0;
    }

    return(0);
}

static int QRM( Mat a, Vec dv )
{
    Vec     ev, ev1;
    double  w, t, s, x, y, c;
    double  *v1, *v2;
    int     dim, iter;
    int     i, j, k, h;

    dim = a.row;
    if( dim != a.clm || dim < 2 ) return(-1);
    if( dv.clm != dim ) return(-1);

    ev = vecAlloc( dim );
    if( ev.clm != dim ) return(-1);

    ev1.clm = dim-1;
    ev1.v = &(ev.v[1]);
    if( vecTridiagonalize( a, dv, ev1 ) < 0 ) {
        vecFree( ev );
        return(-1);
    }

    ev.v[0] = 0.0;
    for( h = dim-1; h > 0; h-- ) {
        j = h;
        while(fabs(ev.v[j]) > EPS*(fabs(dv.v[j-1])+fabs(dv.v[j]))) j--;
        if( j == h ) continue;

        iter = 0;
        do{
            iter++;
            if( iter > MAX_ITER ) break;

            w = (dv.v[h-1] - dv.v[h]) / 2;
            t = ev.v[h] * ev.v[h];
            s = sqrt(w*w+t); 
            if( w < 0 ) s = -s;
            x = dv.v[j] - dv.v[h] + t/(w+s);
            y = ev.v[j+1];
            for( k = j; k < h; k++ ) {
                if( fabs(x) >= fabs(y) ) {
		    if( fabs(x) > VZERO ) {
			t = -y / x;
			c = 1 / sqrt(t*t+1);
			s = t * c;
		    }
		    else{
			c = 1.0;
			s = 0.0;
		    }
                }
                else{
		    t = -x / y;
		    s = 1.0 / sqrt(t*t+1);
		    c = t * s;
                }
                w = dv.v[k] - dv.v[k+1];
                t = (w * s + 2 * c * ev.v[k+1]) * s;
                dv.v[k]   -= t;
                dv.v[k+1] += t;
                if( k > j) ev.v[k] = c * ev.v[k] - s * y;
                ev.v[k+1] += s * (c * w - 2 * s * ev.v[k+1]);

                for( i = 0; i < dim; i++ ) {
                    x = a.m[k*dim+i];
                    y = a.m[(k+1)*dim+i];
                    a.m[k*dim+i]     = c * x - s * y;
                    a.m[(k+1)*dim+i] = s * x + c * y;
                }
                if( k < h-1 ) {
                    x = ev.v[k+1];
                    y = -s * ev.v[k+2];
                    ev.v[k+2] *= c;
                }
            }
        } while(fabs(ev.v[h]) > EPS*(fabs(dv.v[h-1])+fabs(dv.v[h])));
    }

    for( k = 0; k < dim-1; k++ ) {
        h = k;
        t = dv.v[h];
        for( i = k+1; i < dim; i++ ) {
            if( dv.v[i] > t ) {
                h = i;
                t = dv.v[h];
            }
        }
        dv.v[h] = dv.v[k];
        dv.v[k] = t;
        v1 = &(a.m[h*dim]);
        v2 = &(a.m[k*dim]);
        for( i = 0; i < dim; i++ ) {
            w = *v1;
            *v1 = *v2;
            *v2 = w;
            v1++;
            v2++;
        }
    }

    vecFree( ev );
    return(0);
}

int matrixSave(Mat m, char *fname) {
	FILE *fp;

	fp = fopen(fname, "w");
	if(fp == NULL) {
		return 1;
	}
	fwrite(&(m.row), sizeof(int), 1, fp);
	fwrite(&(m.clm), sizeof(int), 1, fp);
	fwrite(m.m, sizeof(double), m.row * m.clm, fp);
	fclose(fp);

	return 0;
}

static double *minv( double *ap, int dimen, int rowa );

int matrixSelfInv(Mat m) {
	if(minv(m.m, m.row, m.row) == NULL) {
		return 1;
	}
	return 0;
}

/*                              */
/*    MATRIX inverse function   */
/*    計算不能の時はNULLを返す  */
/*                              */
/*
double *ap;                     Input & output matrix
int rowa;                       ROW Dimension of matrix A
int dimen;                      正方行列のサイズ = rowa
*/
/********************************/
static double *minv(double *ap,int dimen,int rowa)
{
        double *wap, *wcp, *wbp;/* work pointer                 */
        int i,j,n,ip,nwork;
        int nos[50];
        double epsl;
        double p,pbuf,work;

        epsl = 1.0e-10;         /* Threshold value      */

        switch (dimen) {
                case (0): return(NULL);                 /* check size */
                case (1): *ap = 1.0 / (*ap);
                          return(ap);                   /* 1 dimension */
        }

        for(n = 0; n < dimen ; n++)
                nos[n] = n;

        for(n = 0; n < dimen ; n++) {
                wcp = ap + n * rowa;

                for(i = n, wap = wcp, p = 0.0; i < dimen ; i++, wap += rowa)
                        if( p < ( pbuf = fabs(*wap)) ) {
                                p = pbuf;
                                ip = i;
                        }
                if (p <= epsl)
                        return(NULL);

                nwork = nos[ip];
                nos[ip] = nos[n];
                nos[n] = nwork;

                for(j = 0, wap = ap + ip * rowa, wbp = wcp; j < dimen ; j++) {
                        work = *wap;
                        *wap++ = *wbp;
                        *wbp++ = work;
                }

                for(j = 1, wap = wcp, work = *wcp; j < dimen ; j++, wap++)
                        *wap = *(wap + 1) / work;
                *wap = 1.0 / work;

                for(i = 0; i < dimen ; i++) {
                        if(i != n) {
                                wap = ap + i * rowa;
                                for(j = 1, wbp = wcp, work = *wap;
                                                j < dimen ; j++, wap++, wbp++)
                                        *wap = *(wap + 1) - work * (*wbp);
                                *wap = -work * (*wbp);
                        }
                }
        }

        for(n = 0; n < dimen ; n++) {
                for(j = n; j < dimen ; j++)
                        if( nos[j] == n) break;
                nos[j] = nos[n];
                for(i = 0, wap = ap + j, wbp = ap + n; i < dimen ;
                                        i++, wap += rowa, wbp += rowa) {
                        work = *wap;
                        *wap = *wbp;
                        *wbp = work;
                }
        }
        return(ap);
}

int matrixTrans(Mat dest, Mat source) {
	int r, c;

	if(dest.row != source.clm || dest.clm != source.row) {
		return 1;
	}
	for(r = 0; r < dest.row; r++) {
		for(c = 0; c < dest.clm; c++) {
			ELEM0(dest, r, c) = ELEM0(source, c, r);
		}
	}
	return 0;
}

int matrixUnit(Mat unit) {
	int r, c;

	if(unit.row != unit.clm) {
		return 1;
	}
	for(r = 0; r < unit.row; r++) {
		for(c = 0; c < unit.clm; c++) {
			if(r == c) {
				ELEM0(unit, r, c) = 1.0;
			}
			else {
				ELEM0(unit, r, c) = 0.0;
			}
		}
	}
	return 0;
}

void   matrixZero(Mat m)
{
  int r, c;
  for( r = 0; r < m.row; ++r){
    for( c = 0; c < m.clm; ++c){
      ELEM0(m, r, c ) = 0.0;
    }
  }

}

Vec vecAlloc( int clm )
{
    Vec     v;

    v.v = (double *)malloc(sizeof(double) * clm);
    if( v.v == NULL ) v.clm = 0;
    else              v.clm = clm;

    return v;
}

void vecDisp( Vec v )
{
    int    c;
    
    if( ERR_VEC( v ) ){
	printf( "ERROR VECTOR\n" );
	return;
    }
    printf(" === vector (%d) ===\n", v.clm);
    printf(" |");
    for( c = 0; c < v.clm; c++ ){
	printf( " %10g", v.v[c] );
    }
    printf(" |\n");
    printf(" ===================\n");
}

void vecFree( Vec v )
{
    free( v.v );
}

double vecHousehold( Vec x )
{
    double s, t;
    int    i;

    s = sqrt( vecInnerproduct(x,x) );

    if( s != 0.0 ) {
        if(x.v[0] < 0) s = -s;
        x.v[0] += s;
        t = 1 / sqrt(x.v[0] * s);
        for( i = 0; i < x.clm; i++ ) {
            x.v[i] *= t;
        }
    }

    return(-s);
}

double vecInnerproduct( Vec x, Vec y )
{
    double   result = 0.0;
    int      i;

    if( x.clm != y.clm ) exit(0);
    for( i = 0; i < x.clm; i++ ) {
        result += x.v[i] * y.v[i];
    }

    return( result );
}

int vecTridiagonalize( Mat a, Vec d, Vec e )
{
    Vec     wv1, wv2;
    double  *v;
    double  s, t, p, q;
    int     dim;
    int     i, j, k;

    if( a.clm != a.row )   return(-1);
    if( a.clm != d.clm )   return(-1);
    if( a.clm != e.clm+1 ) return(-1);
    dim = a.clm;

    for( k = 0; k < dim-2; k++ ) {
        v = &(a.m[k*dim]);
        d.v[k] = v[k];

        wv1.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        e.v[k] = vecHousehold(wv1);
        if( e.v[k] == 0.0 ) continue;

        for( i = k+1; i < dim; i++ ) {
            s = 0.0;
            for( j = k+1; j < i; j++ ) {
                s += a.m[j*dim+i] * v[j];
            }
            for( j = i; j < dim; j++ ) {
                s += a.m[i*dim+j] * v[j];
            }
            d.v[i] = s;
        }

        wv1.clm = wv2.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        wv2.v = &(d.v[k+1]);
        t = vecInnerproduct( wv1, wv2 ) / 2;
        for( i = dim-1; i > k; i-- ) {
            p = v[i];
            q = d.v[i] -= t*p;
            for( j = i; j < dim; j++ ) {
                a.m[i*dim+j] -= p*d.v[j] + q*v[j];
            }
        }
    }

    if( dim >= 2) {
        d.v[dim-2] = a.m[(dim-2)*dim+(dim-2)];
        e.v[dim-2] = a.m[(dim-2)*dim+(dim-1)];
    }

    if( dim >= 1 ) d.v[dim-1] = a.m[(dim-1)*dim+(dim-1)];

    for( k = dim-1; k >= 0; k-- ) {
        v = &(a.m[k*dim]);
        if( k < dim-2 ) {
            for( i = k+1; i < dim; i++ ) {
                wv1.clm = wv2.clm = dim-k-1;
                wv1.v = &(v[k+1]);
                wv2.v = &(a.m[i*dim+k+1]);
                t = vecInnerproduct( wv1, wv2 );
                for( j = k+1; j < dim; j++ ) a.m[i*dim+j] -= t * v[j];
            }
        }
        for( i = 0; i < dim; i++ ) v[i] = 0.0;
        v[k] = 1;
    }

    return(0);
}
