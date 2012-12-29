// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL transform + remove_copy for subsetting
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl featured
 * @summary The STL's transform and remove_copy functions can subset
 *
 * We have seen the use of the STL transform functions in the posts
 * [STL transform](../stl-transform) and 
 * [Transforming a matrix](../transforming-a-matrix).
 * We use the same logic in conjuction with a logical (ie boolean) 
 * vector in order subset an initial vector. 
 */

#include <Rcpp.h>

using namespace Rcpp;
using namespace std;

const double flagval = __DBL_MIN__; // works
//const double flagval = NA_REAL;   // does not

// simple double value 'flagging' function
inline double flag(double a, bool b) { return b ? a : flagval; }

// [[Rcpp::export]]
NumericVector subsetter(NumericVector a, LogicalVector b) {
    // We use the flag() function to mark values of 'a' 
    // for which 'b' is false with the 'flagval'
    transform(a.begin(), a.end(), b.begin(), a.begin(), flag);

    // We use sugar's sum to compute how many true values to expect
    NumericVector res = NumericVector(sum(b));

    // And then copy the ones different from flagval
    remove_copy(a.begin(), a.end(), res.begin(), flagval);
    return res;    
}

/** 
 * We can illustrate this on a simple example or two:
 */

/*** R
a <- 1:5
subsetter(a, a %% 2 == 0)
subsetter(a, a > 2)
*/

