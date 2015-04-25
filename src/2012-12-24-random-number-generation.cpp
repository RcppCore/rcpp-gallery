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
 * these can be accessed in a vectorised manner, as we illustrated
 * in the post [simulating pi](../simulating-pi).
 */


#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix rngCpp(const int N) {
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


/**
 * ### RNG state
 * 
 * Section 6.3 of [Writing R
 * Extensions](http://cran.r-project.org/doc/manuals/r-release/R-exts.html#Random-number-generation)
 * describes an additional requirement for calling the R random number
 * generation functions: you must call `GetRNGState` prior to using them and
 * then `PutRNGState` afterwards. These functions (respectively) read 
 * `.Random.seed` and then write it out after use.
 * 
 * When using Rcpp attributes (as we do via the `// [[Rcpp::export]]` annotation
 * on the functions above) it is not necessary to call `GetRNGState` and
 * `PutRNGState` because this is done automatically within the wrapper code
 * generated for exported functions. In fact, since these calls don't nest it is
 * actually an error to call them when within a function exported via Rcpp
 * attributes.
 * 
 * In the case where you are writing an Rcpp function that doesn't use Rcpp
 * attributes (e.g. a function using a raw `SEXP` interface that is exported via
 * `extern C`) Rcpp exposes a convenience class you can use to get and put the
 * RNG state. For example:
 */

extern "C" SEXP rngScopeCppScalar() {
  
  NumericVector x(4);
  
  RNGScope rngScope;
  
  x[0] = R::runif(0,1);
  x[1] = R::rnorm(0,1);
  x[2] = R::rt(5);
  x[3] = R::rbeta(1,1);
  
  return wrap(x);
}

/**
 * The `RNGScope` object calls `GetRNGState` in it's constructor and `PutRNGState` in
 * it's destructor. It also implements an internal counter so that multiple instances
 * of `RNGScope` can co-exist on the stack and ensure that the get and put functions
 * are only called once.
 * 
 * One very important detail about the example above is that the `RNGScope` object
 * is declared *after* the object that will ultimately be returned to R. This ordering
 * is critically important, as the call to `PutRNGScope` can in some instances 
 * trigger a garbarge collection which collects the `SEXP` to be returned to R
 * (causing a crash). Here's another example to further illustrate:
 */

extern "C" SEXP rngScopeResultDemo() {
  
  RObject result;
  
  RNGScope rngScope;
  
  result = NumericVector::create(42, 21);
  
  return wrap(result);
}

/**
 * As discussed above, you generally don't need to worry about explicit use of 
 * `RNGScope`, however if you do it's very important to declare any Rcpp object
 * to be returned to R *prior* to declaring `RNGScope`.
 */



