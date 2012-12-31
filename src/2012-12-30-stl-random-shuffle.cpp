// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL random_shuffle for permutations
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl featured
 * @summary Using the STL's random_shuffle function
 *
 * The STL also contains random sampling and shuffling algorithms.
 * We start by looking at `random_shuffle`.
 *
 * There are two forms. The first uses an internal RNG with its own
 * seed; the second form allows for a function object conformant to
 * the STL's requirements (essentially, given `N` produce a uniform
 * draw greater or equal to zero and less than `N`).  This is useful
 * for us as it lets us tie this to the same RNG which R uses.
 */

#include <Rcpp.h>

// wrapper around R's RNG such that we get a uniform distribution over
// [0,n) as required by the STL algorithm
inline int randWrapper(const int n) { return floor(unif_rand()*n); }

// [[Rcpp::export]]
Rcpp::NumericVector randomShuffle(Rcpp::NumericVector a) {
    // already added by sourceCpp(), but needed standalone
    Rcpp::RNGScope scope;             

    // clone a into b to leave a alone
    Rcpp::NumericVector b = Rcpp::clone(a);

    std::random_shuffle(b.begin(), b.end(), randWrapper);

    return b;
}

/** 
 * We can illustrate this on a simple example or two:
 */

/*** R
a <- 1:8
set.seed(42)
randomShuffle(a)
set.seed(42)
randomShuffle(a)
*/

/** 
 * By tieing the STL implementation of the random permutation to the
 * RNG from R, we are able to compute reproducible permutations, fast
 * and from C++.
 */
