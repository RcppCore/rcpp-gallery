// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL Transform
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl
 * @summary This example shows how to use the STL's transform function
 *
 * The STL transform function can be used to pass a single function over
 * a vector. Here we use a simple function `square()`.
 */


#include <Rcpp.h>

using namespace Rcpp;

inline double square(double x) { return x*x ; }

// [[Rcpp::export]]
std::vector<double> transformEx(const std::vector<double>& x) {
    std::vector<double> y(x.size());
    std::transform(x.begin(), x.end(), y.begin(), square);
    return y;
}

/*** R
  x <- c(1,2,3,4)
  transformEx(x)
*/


/**
 * A second variant combines two input vectors.
 *
 */

inline double squaredNorm(double x, double y) { return sqrt(x*x + y*y); }

// [[Rcpp::export]]
NumericVector transformEx2(NumericVector x, NumericVector y) {
    NumericVector z(x.size());
    std::transform(x.begin(), x.end(), y.begin(), z.begin(), squaredNorm);
    return z;
}

/*** R
  x <- c(1,2,3,4)
  y <- c(2,2,3,3)
  transformEx2(x,y)
*/



