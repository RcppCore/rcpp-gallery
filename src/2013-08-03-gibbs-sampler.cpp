// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Gibbs Sampler in C++
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags rng sugar 
 * @summary This example illustrate a popular MCMC sampler
 *
 * Markov Chain Monte Carlo (MCMC) is a popular simulation method. As
 * it is somewhat demanding, it is also frequently used to benchmark
 * different implementations or algorithms.
 * 
 * One particular algorithm has been compared a number of times, starting
 * [with an article by Darren Wilkinson](http://darrenjw.wordpress.com/2010/04/28/mcmc-programming-in-r-python-java-and-c/),
 * and [Darren's follow--up article](http://darrenjw.wordpress.com/2011/07/16/gibbs-sampler-in-various-languages-revisited/) 
 * which in turns responded in part to 
 * [our article](http://dirk.eddelbuettel.com/blog/2011/07/14/).
 *
 * This post simply refreshes the implementation using Rcpp attributes.
 *
 * First we look at the R version, and its byte-compiled variant.
 */

/*** R
## Here is the actual Gibbs Sampler
## This is Darren Wilkinsons R code (with the corrected variance)
Rgibbs <- function(N,thin) {
    mat <- matrix(0,ncol=2,nrow=N)
    x <- 0
    y <- 0
    for (i in 1:N) {
        for (j in 1:thin) {
            x <- rgamma(1,3,y*y+4)
            y <- rnorm(1,1/(x+1),1/sqrt(2*(x+1)))
        }
        mat[i,] <- c(x,y)
    }
    mat
}

## We can also let the R compiler on this R function
library(compiler)
RCgibbs <- cmpfun(Rgibbs)
*/

/**
 * Creating a version in C++ is very straightforward thanks to Rcpp and
 * Rcpp Attributes. It transfers the integer arguments `n` (number of
 * simulations) and `thn` (number of extra thinning simulations),
 * initializes the R random number generator for us (eg no need to manual
 * nstantiate the `RNGScope` object), and returns the result matrix
 * 
 * Also, since the initial posts were written, we made the (scalar) RNGs
 * of the R API available directly via the `R` namespace. This is both
 * little nice to read than the poor-man's pseudo-namespace in C via the
 * `Rf_` prefix, and actually provides proper C++ namespace.
*/

// load Rcpp
#include <Rcpp.h>

using namespace Rcpp;		// shorthand

// [[Rcpp::export]]
NumericMatrix RcppGibbs(int n, int thn) {

    int i,j;
    NumericMatrix mat(n, 2);

    // The rest of the code follows the R version
    double x=0, y=0;

    for (i=0; i<n; i++) {
        for (j=0; j<thn; j++) {
            x = R::rgamma(3.0,1.0/(y*y+4));
            y = R::rnorm(1.0/(x+1),1.0/sqrt(2*x+2));
        }
        mat(i,0) = x;
        mat(i,1) = y;
    }

    return mat;             // Return to R
}


/**
 * With the functions in place, we can re-run the benchmark.
*/

/*** R
library(rbenchmark)
n <- 2000
thn <- 200
benchmark(Rgibbs(n, thn),
          RCgibbs(n, thn),
          RcppGibbs(n, thn),
          columns=c("test", "replications", "elapsed", "relative"),
          order="relative",
          replications=10)
*/

/**
 * As we have seen before, the C++ version is about 50 times faster,
 * and around 40 times faster than the byte-compiled version. 
 *
 * A [related article here on the Rcpp Gallery](http://gallery.rcpp.org/articles/timing-rngs/)
 * looks into timing different RNG implementation as this study revealed
 * that the generator for Gamma-distributed random number in R is not
 * particularly fast.  
*/
