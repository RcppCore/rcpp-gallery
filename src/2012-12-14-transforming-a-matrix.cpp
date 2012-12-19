/**
 * @title Transforming a Matrix
 * @author Dirk Eddelbuettel
 * @license MIT
 * @tags matrix stl
 * @summary Demonstrates transforming a matrix passed to a function 
 *   using std::transform.
 * 
 * Here we take the square root of each item of a matrix and return a 
 * new matrix with the tranformed values. We do this by using
 * `std::transform` to call the `sqrt` function on each element of
 * the matrix:
 */

#include <Rcpp.h>
#include <cmath>

using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix matrixSqrt(NumericMatrix orig) {

  // allocate the matrix we will return
  NumericMatrix mat(orig.nrow(), orig.ncol());
  
  // transform it 
  std::transform(orig.begin(), orig.end(), mat.begin(), ::sqrt);
  
  // return the new matrix
  return mat;
}

/**
 * Here we call the function from R:
 */


/*** R
m <- matrix(c(1,2,3, 11,12,13), nrow = 2, ncol=3)
matrixSqrt(m)
*/
