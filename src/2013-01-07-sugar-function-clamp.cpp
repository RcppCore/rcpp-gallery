// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using the Rcpp sugar function clamp
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags sugar benchmark featured
 * @summary This post illustrates the sugar function clamp
 *
 * Since the 0.10.* release series, Rcpp contains a new sugar function `clamp`
 * which can be used to limit vectors to both a minimum and maximim value.
 * [This recent StackOverflow question]() permitted `clamp` to
 * shine. We retake some of the answers, including the `clamp` entry
 * by Romain.
 *
 * We first define the three R versions.
 */

/*** R
pminpmaxClamp <- function(x, a, b) {
    pmax(a, pmin(x, b) )
}

ifelseClamp <- function(x, a, b) {
    ifelse(x <= a,  a, ifelse(x >= b, b, x))
}

operationsClamp <- function(x, a, b) {
    a + (x-a > 0)*(x-a) - (x-b > 0)*(x-b)
}
*/

/** 
 * We then define some data, and ensure that these versions all
 * producing identical results.
 */

/*** R
set.seed(42)
x <- rnorm(100000)

a <- -1.0
b <- 1.0
stopifnot(all.equal(pminpmaxClamp(x,a,b), ifelseClamp(x,a,b), operationsClamp(x,a,b)))
*/

/**
 * Next is the C++ solution: a one-liner thanks to the existing sugar function.
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rcppClamp(NumericVector x, double mi, double ma) {
    return clamp(mi, x, ma);
}

/**
 * We can then check and benchmark the new C++ version.
 */

/*** R
stopifnot(all.equal(pminpmaxClamp(x,a,b), rcppClamp(x,a,b)))

library(rbenchmark)
benchmark(pminpmaxClamp(x, a, b), 
          ifelseClamp(x, a, b), 
          operationsClamp(x, a, b),
          rcppClamp(x, a, b),
          order="relative")[,1:4]
*/

/**
 * We see a decent gain of the Rcpp version even relative to these
 * vectorised R solutions. Among these, the simplest (based on
 * `ifelse`) is by far the slowest.  The parallel min/max version is
 * about as faster as the clever-but-less-readable expression-based
 * solution.
 *
 * Real "production" solutions will of course need some more testing
 * of inputs etc.  However, as an illustration of `clamp` this example
 * has hopefully been compelling.
 */

