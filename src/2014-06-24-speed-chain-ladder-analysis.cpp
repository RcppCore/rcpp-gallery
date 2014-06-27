// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Speed Chain Ladder Analysis with Rcpp
 * @author Chibisi Chima-Okereke
 * @license GPL (>= 2)
 * @tags modeling armadillo
 * @summary Speed-up of Chain Ladder analysis by calling C++ routines from R
 */

/**
 * The Chain Ladder method is an actuarial technique used for
 * projecting incurred insurance claims to their ultimate loss
 * values. The data exists as claims triangles where the claims for
 * each accounting year increments down the rows and the claims for
 * each development period increments along the columns.  This claims
 * triangle can be represented in a triangular upper matrix (along the
 * anti-diagonal) and the Chain Ladder technique works by filling in
 * the lower part of the matrix using ratios of claims in previous
 * accounting years and development periods.
 * 
 * In this example, we show how an implementation in R is sped up by
 * calling an equivaluent implementation in C++ from R using the Rcpp
 * interface.
 * 
 * We start with the C++ code for carrying out the Chain Ladder calculation.
 *
 */

// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>  // most of the algorithms 
#include <numeric> // some numeric algorithm

using namespace std;
using namespace arma;
using namespace Rcpp;

// Code for the age-to-age factor when the column index is given 
double GetFactor(int index, mat mTri) {
    int nRow = mTri.n_rows;
    mat subMat = mTri.submat(0, index, nRow - (index + 2), index + 1);
    rowvec colSums = arma::sum(subMat, 0);
    double inFact = colSums(1)/colSums(0);
    return inFact;
}

// Code for getting all the factors from the triangle 
vec GetFactors(mat mTri) {
    int nCol = mTri.n_cols;
    vec dFactors(nCol - 1);
    for (int i=0; i < nCol - 1; ++i) {
        dFactors(i) = GetFactor(i, mTri);
    }
    return dFactors;
}

// This is code for the cumulative product of a vector 
vec cumprod(vec mvec) {
    int nElem = mvec.n_elem;
    double cprod = mvec(0);
    for (int i = 1; i < nElem; ++i) {
        cprod *= mvec(i);
        mvec(i) = cprod;
    }
    return mvec;
}

/**
 * The following function returns the  fully projected triangle 
 */

// [[Rcpp::export]]
SEXP GetChainSquareCpp(SEXP mClaimTri) {
    NumericMatrix nMat(mClaimTri);
    int nRow = nMat.nrow(), nCol = nMat.ncol();
    mat armMat(nMat.begin(), nRow, nCol, FALSE);
	
    vec dFactors = GetFactors(armMat);
    mat revMat = fliplr(armMat);
    vec dAntiDiag = diagvec(revMat);
    dAntiDiag = dAntiDiag.subvec(1, nCol - 1);
    double dMult;
    vec prodVec;
    for (unsigned int index = 0; index < dAntiDiag.n_elem; ++index) {
        dMult = dAntiDiag(index);
        prodVec = dFactors.subvec(nCol - index - 2, nCol - 2);
        prodVec = cumprod(prodVec);
        armMat(index + 1, span(nCol - index - 1, nCol - 1)) = dMult*prodVec.st();
    }
    return wrap(armMat);
}

/**
 * The preceding C++ code is used by the following R code for simulating the claims triangles 
 */

/*** R

# Age-To-Age Factors
ageFact <- seq(1.9, 1, by = -.1)

# Inflation Rate
infRate <- 1.02

# Function to reverse matrix columns
revCols <- function(x) {
    x[,ncol(x):1]
}

# Similar to jitter()
shake <- function(vec, sigmaScale = 100) {
    rnorm(n = length(vec), mean = vec, sd = vec/sigmaScale)
}

# Row generation funtion
GenerateRow <- function(iDev, dFactors = cumprod(ageFact), 
                        dInflationRate = 1.02, initClaim = 154) {
    shake(initClaim)*shake(c(1, dFactors))*(dInflationRate^iDev)
}

# Function to generate a claims matrix
GenerateTriangle <- function(iSize, ...) {
    indices = 1:iSize
    mClaimTri = t(sapply(indices, GenerateRow, ...))
    # Reverse columns to get the claims triangle
    mClaimTri = revCols(mClaimTri)
    # Assign nan to lower triangle
    mClaimTri[lower.tri(mClaimTri)] = NA
    mClaimTri = revCols(mClaimTri)
    return(mClaimTri)
}
*/

/** 
 * We then use R code for projecting the traingle 
 */

/*** R
# Get claims factor at a particular column index
GetFactorR <- function(index, mTri) {
    fact = matrix(mTri[-c((nrow(mTri) - index + 1):nrow(mTri)), index:(index + 1)], ncol = 2)
    fact = c(sum(fact[,1]), sum(fact[,2]))  
    return(fact[2]/fact[1])
}

# Function to carry out Chain Ladder on a claims triangle
GetChainSquareR <- function(mClaimTri) {
    nCols <- ncol(mClaimTri)
    dFactors = sapply(1:(nCols - 1), GetFactorR, mTri = mClaimTri)
    dAntiDiag = diag(revCols(mClaimTri))[2:nCols]
    for(index in 1:length(dAntiDiag)) {
	mClaimTri[index + 1, (nCols - index + 1):nCols] = 
                dAntiDiag[index]*cumprod(dFactors[(nCols - index):(nCols - 1)])
    }
    mClaimTri
}
*/

/*** 
 * We now run a timed test comparing chain ladder running in R natively 
 * and being called from C++ functions using the Rcpp interface 
 */ 

/*** R
library(microbenchmark)
x <- GenerateTriangle(11)
microbenchmark(GetChainSquareR(x), GetChainSquareCpp(x), times = 10000L)
*/

