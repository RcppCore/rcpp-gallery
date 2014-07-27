// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using RcppArmadillo with bigmemory
 * @author Scott Ritchie
 * @license GPL (>= 2)
 * @tags armadillo bigmemory matrix
 * @summary This example shows how to use RcppArmadillo with `big.matrix` objects from the `bigmemory` package.
 *
 * The [bigmemory package](www.bigmemory.org) allows users to create
 * matrices that are external to R, stored either in RAM or on disk,
 * allowing them to be bigger than the system RAM, and allowing them to
 * be shared across R sessions.
 *
 * While these objects are defined by the `big.matrix` class in R, they
 * are really just wrappers that point to external memory. The actual
 * objects are implemented in C++, thus can be easily manipulated from
 * `Rcpp`.
 *
 * `Armadillo` is a C++ library that provides fast linear algebra
 * functionality. It can be used in conjunction with `Rcpp` through
 * the `RcppArmadillo` package.
 *
 * This example demonstrates how to use the functionality provided by
 * `Armadillo` on `big.matrix` objects using `Rcpp`. To learn how to
 * work with `big.matrix` objects in `Rcpp` without using `RcppArmadillo`
 * see this [previous gallery post](http://gallery.rcpp.org/articles/using-bigmemory-with-rcpp/).
 *
 * Utilising the functions provided by `armadillo` simply requires the
 * `Rcpp` objects and `big.matrix` objects to be case to the appropriate
 * types that `armadillo` knows how to work with. To demonstrate this,
 * we again implement the equivalent of the `colSums` function, but allow
 * the user to specify which columns they want to apply the function to
 * in advance.
 */

// To enable the functionality provided by Armadillo's various macros,
// simply include them before you include the RcppArmadillo headers.
#define ARMA_NO_DEBUG

#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo, BH, bigmemory)]]

using namespace Rcpp;
using namespace arma;

// The following header file provides the definitions for the BigMatrix
// object
#include <bigmemory/BigMatrix.h>

// Bigmemory now accesses Boost headers from the BH package,
// we need to make sure we do so as well in this Rcpp::depends comment.
// Boost headers can generate some warning with the default compilation
// options for R.  To suppress these, we can enable C++11 mode which gets
// us 'long long' types.
//
// If your compiler is to old, just disable / remove the following line
// [[Rcpp::plugins(cpp11)]]


// Implementation of colSums
//
// Because BigMatrix objects can come with four different underlying
// types, we need to split our functionality into a "dispatch" function,
// which makes the appropriate typecasts based on the matrix type, and an
// "implementation" function which does the actual calculations.
//
template <typename T>
Col<T> BigArmaColSums(const Mat<T>& aBigMat, IntegerVector subsetCols) {
  // The functions for subsetting an arma::Mat expect the indices to
  // be in of type 'uvec' (shorthand for 'Col<unsigned int>'), so we
  // need to conver the IntegerVector to that type.
  //
  // We subtract 1 to convert the R indices into C++ indices, since
  // indices in R start at 1, but in C++ they start at 0.
  uvec colidx = as<uvec>(subsetCols) - 1;

  // To subset by columns, we use the .cols method for the arma::Mat
  // class. This allows us to access non-contiguous columns.
  return sum(aBigMat.cols(colidx));
}

// Dispatch function for colSums
//
// We need to write this wrapper to handle the different types used
// when constructing a big.matrix
//
// [[Rcpp::export]]
NumericVector BigArmaColSums(SEXP pBigMat, IntegerVector subsetCols) {
  // First we tell Rcpp that the object we've been given is an external
  // pointer.
  XPtr<BigMatrix> xpMat(pBigMat);

  // First we should make sure that none of the requested columns are
  // outside of the provided matrix. If we let the code access an area
  // of memory it shouldn't, bad things will happen!
  if (is_true(any(subsetCols < 1)) ||
      is_true(any(subsetCols > xpMat->ncol()))) {
    throw std::out_of_range("Some of requested columns are outside of the matrix!");
  }

  // The actual data for the matrix is stored in the matrix() field.
  // This is just a pointer to an array, which is laid out in memory in
  // the column major format. Armadillo matrices are also stored in column
  // major format. We can therefore use the advanced `arma::mat` constructor
  // with `copy_aux_mem` set to `false` to effectively "cast" this memory
  // to an object RcppArmadillo understands.
  //
  // Note that this is an 'unsafe' cast, since we're telling armadillo
  // to use existing memory, rather than to create a new matrix. So we need
  // to be careful that the memory we're telling it to use has the correct
  // dimensions!
  //
  unsigned int type = xpMat->matrix_type();
  // The data stored in the big.matrix can either be represent by 1, 2,
  // 4, or 8 bytes. See the "type" argument in `?big.matrix`.
  if (type == 1) {
    Col<char> colSums = BigArmaColSums(
      arma::Mat<char>((char *)xpMat->matrix(), xpMat->nrow(), xpMat->ncol(), false),
      subsetCols
    );
    return NumericVector(colSums.begin(), colSums.end());
  } else if (type == 2) {
    Col<short> colSums = BigArmaColSums(
      arma::Mat<short>((short *)xpMat->matrix(), xpMat->nrow(), xpMat->ncol(), false),
      subsetCols
    );
    return NumericVector(colSums.begin(), colSums.end());
  } else if (type == 4) {
    Col<int> colSums = BigArmaColSums(
      arma::Mat<int>((int *)xpMat->matrix(), xpMat->nrow(), xpMat->ncol(), false),
      subsetCols
    );
    return NumericVector(colSums.begin(), colSums.end());
  } else if (type == 8) {
    Col<double> colSums = BigArmaColSums(
      arma::Mat<double>((double *)xpMat->matrix(), xpMat->nrow(), xpMat->ncol(), false),
      subsetCols
    );
    return NumericVector(colSums.begin(), colSums.end());
  } else {
    // We should never get here, but it resolves compiler warnings.
    throw Rcpp::exception("Undefined type for provided big.matrix");
  }
}

/**
 * So lets see how this function performs in comparison to `colSums`,
 * [BigColSums](http://gallery.rcpp.org/articles/using-bigmemory-with-rcpp/).
 */

/*** R
library(microbenchmark)
library(bigmemory)
# Lets test the performance on 100 random columns from a 60*1000 matrix.
set.seed(4)
ridx <- sample(1:1000, 100)
m <- matrix(rnorm(60*1000), nrow=60)
bigm <- as.big.matrix(m)

options(width=100) # Make sure output looks ok
microbenchmark(res1 <- colSums(m[,ridx]), res2 <- BigArmaColSums(bigm@address, ridx))
# and make sure the results are the same:
all.equal(res1, res2)

*/

