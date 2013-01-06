// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Calling R Functions from C++
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics function featured
 * @summary This post discusses calling R functions from C++
 *
 *
 * At its very essence, Rcpp permits easy access to native R objects at the C++ level. R objects can be 
 *  - simple vectors, list or matrices;
 *  - compound data structures created from these; 
 *  - objects of S3, S4 or Reference Class vintage; or
 *  - language objects as for example [environments](../accessing-environments).
 * 
 * Accessing a function object is no different.  And calling a
 * function can be very useful.  Maybe to pick up parameter
 * initializations, maybe to access a custom data summary that would
 * be tedious to recode, or maybe even calling a plotting routine.  We
 * already have examples for just about all of these use case in the
 * Rcpp examples or unit tests shipping with the package.
 *
 * So here were a just providing a simple example of calling a summary
 * function, namely the Tukey `fivenum()`.
 *
 * But before we proceed, a warning.  Calling a function is simple and
 * tempting. It is also slow as there are overheads involved.  And
 * calling it repeatedly from inside your C++ code, possibly buried
 * within several loops, is outright silly. This _has to be_ slower
 * than equivalent C++ code, and even slower than just the R code
 * (because of the marshalling of data).  Do it when it makes sense,
 * and not simply because it is available.
 */

/*** R
set.seed(42)
x <- rnorm(1e5)
fivenum(x)
*/

/**
 * Now via this C++ code:
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector callFunction(NumericVector x, Function f) {
    NumericVector res = f(x);
    return res;
}

/**
 * And unsurprisingly, calling the same function on the same data gets the same result:
 */

/*** R
callFunction(x, fivenum)
*/
