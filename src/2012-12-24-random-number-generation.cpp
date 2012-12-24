/**
 * @title Random number generation
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags rmath rng sugar
 * @summary This example shows how to use the random numbers generators from R
 *
 * R comes with several random number generators supporting the
 * ability to draw random samples from a wide variety of statistical
 * distributions.
 *
 * Rcpp builds on this, and provides access to the same random number
 * generators, and distributions.  Moreover, thanks to Rcpp sugar,
 * these can be accessed in a vectorised manner (which we illustrated
 * in the post [simulating
 * pi](http://gallery.rcpp.org/articles/simulating-pi)).
 */


#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix rngCpp(const int N) {
  RNGScope scope;		// ensure RNG gets set/reset
  NumericMatrix X(N, 4);
  X(_, 0) = runif(N);
  X(_, 1) = rnorm(N);
  X(_, 2) = rt(N, 5);
  X(_, 3) = rbeta(N, 1, 1);
  return X;
}


/*** R
 set.seed(42)     # setting seed
 M1 <- rngCpp(5)
 M1

 set.seed(42)	  # resetting seed
 M2 <- cbind( runif(5), rnorm(5), rt(5, 5), rbeta(5, 1, 1))
 M2
 
 all.equal(M1, M2)
*/


/**
 * The other method of using the R random-number generator is in
 * <em>scalar</em> mode, one variable and draw at a time. This is very
 * similar to the description of this API in the
 * [Writing R Extensions](http://cran.r-project.org/doc/manuals/R-exts.html) 
 * manual, and provided by Rcpp in the <code>R</code> namespace:
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rngCppScalar() {
  RNGScope scope;		// ensure RNG gets set/reset
  NumericVector x(4);
  x[0] = R::runif(0,1);
  x[1] = R::rnorm(0,1);
  x[2] = R::rt(5);
  x[3] = R::rbeta(1,1);
  return(x);
}

/*** R
 set.seed(42)
 v1 <- rngCppScalar()
 v1

 set.seed(42)
 v2 <-c(runif(1), rnorm(1,0,1), rt(1,5), rbeta(1,1,1))
 v2

 all.equal(v1, v2)
*/
