/**
 * @title Simulating pi from R or C++ in about five lines
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags sugar featured
 * @summary Demonstrates compactness of Rcpp sugar's expression, and how 
 *   they carry over from R.
 * 
 * This example takes the standard simulation for the value of pi 
 * and in implements it in both R and C++. We will see that the C++ version 
 * is very similar to the R version.
 *
 * We start with an R version. 
 *
 * The basic idea is that for a point (x,y), we compute the distance to the origin
 * using Pythargoras' well-known expression, or in this context a standard distance 
 * norm. We do this repeatedly for a number of points, the ratio of those below one 
 * ("inside the unit circle") to the number N of simulation will approach pi/4 -- as
 * we were filling the area of one quarter of the unit circle by limiting ourselves
 * the first quadrant (as we forced x and y to be between 0 and 1).
 * 
 * The key here is that we do this in a _vectorised_ way: by drawing
 * all N x and y coordinates, then computing all distances and comparing to 1.0. After
 * scaling up to the full circle, we have an estimate of pi.
 *
 */

/*** R
piR <- function(N) {
    x <- runif(N)
    y <- runif(N)
    d <- sqrt(x^2 + y^2)
    return(4 * sum(d < 1.0) / N)
}
*/

/**
 * The neat thing about Rcpp sugar enables us to write C++ code that
 * looks almost as compact.  much like we do in R. Re-writing using
 * the sugar `ifelse` function and numeric and comparison operators
 * yields the identical one-line implementation:
 */

#include <Rcpp.h>
using namespace Rcpp;

double piSugar(const int N) {
  RNGScope scope;		// ensure RNG gets set/reset
  NumericVector x = runif(N);
  NumericVector y = runif(N);
  NumericVector d = sqrt(x*x + y*y);
  return 4.0 * sum(d < 1.0) / N;
}

/**
 * Apart from using types (hey, this is C++) and assuring the RNG gets
 * set and reset, the code is essentially identical.
 */
