// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Timing random number generators
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags rng sugar gsl
 * @summary This example shows how to time random numbers generators
 *
 * Simulation studies can run for a long time, and frequently lead to
 * benchmarking and timing studies comparing implementations and algorithms.
 *
 * One sometimes overlooked aspect when comparing across languages is
 * the difference in timing due to maily to differences in the random
 * number generators. The implementation in R is of very high quality,
 * but does not aim to be the fastest possible generator. For example,
 * when we compared different approaches for a simple Gibbs sampler,
 * it was seen that RBG accounted for a large proportion of the
 * difference between and Rcpp implementation in C++, using R's RNG,
 * and a GSL-based C implementation.
 *
 * Here we are comparing three approaches: Armadillo, GSL and R.
 */

/**
 * We start with the generator in Armadillo.
 */

#include <RcppArmadillo.h>

using namespace Rcpp;

// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::export]]
arma::vec armaNormal(const int N) {
    arma::vec x = arma::randn(N,1);
    return x;
}


/**
 * We can also use the GSL.
 */

#include <RcppGSL.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

using namespace Rcpp;

// [[Rcpp::depends(RcppGSL)]]
// [[Rcpp::export]]
NumericVector gslNormal(const int N) {
    NumericVector x(N);
    gsl_rng *r = gsl_rng_alloc(gsl_rng_mt19937);
    for (int i=0; i<N; i++) {
        x[i] = gsl_ran_gaussian(r,1.0);
    }
    gsl_rng_free(r);
    return x;
}

/**
 * Last, we look at the generator in R.
 */


#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rcppNormal(const int N) {
    return rnorm(N, 0, 1);
}

/**
 * And we can compare the approaches.
 */

/*** R
library(rbenchmark)

res <- benchmark(rcppNormal(1e6), 
                 gslNormal(1e6),
                 armaNormal(1e6),
                 order="relative")
res[,1:4]
*/

/** 
 * 
 * Here we find the speed of the Rcpp generator to be slightly ahead
 * of the GSL generator, but both are dominated by the one in
 * Armadillo.
 *
 * Of course, speed is not the only consideration. Rcpp still allows
 * us to generate the same random numbers as R itself which is
 * valuable.
 */
