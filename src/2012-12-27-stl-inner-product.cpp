// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL Inner Product
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl
 * @summary This example shows how to use the STL's inner product
 *
 * The STL contains a large number of useful functions and algorithm.
 * One useful function is `inner_product` which can be used to compute
 * the sum oof the elements of two vectors.
 */

#include <Rcpp.h>
#include <numeric>

// [[Rcpp::export]]
double innerProduct(const std::vector<double>& x, 
                     const std::vector<double>& y) {
    double val = std::inner_product(x.begin(), x.end(), y.begin(), 0);
    return val;
}

/*** R
  x <- c(1,2,3)
  y <- c(4,5,6)
  cbind(x,y)

  innerProduct(x, y)
  sum(x*y)  # check from R
*/

/** 
  * Similarly, we can use it compute a sum of squares:
  */

/*** R
  innerProduct(x, x)
  sum(x^2)
*/
