// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL random_sample
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl 
 * @summary Using the STL's random_sample
 *
 * An earlier post looked at [random shuffle](../stl-random-shuffle)
 * for permutations. The STL also supports creation of random samples.
 * 
 * Alas, it seems that this functionality has not been promoted to the
 * C++ standard yet --- so we will have to do with what is an
 * extensions by the GNU g++ compiler.
 *
 * The other drawback is the sampling without replacement.
 *
 * As in the previous post, we use a function object conformant to the
 * STL's requirements for a random number generator to be able to use
 * R's RNG.
 */

#include <Rcpp.h>

// wrapper around R's RNG such that we get a uniform distribution over
// [0,n) as required by the STL algorithm
inline int randWrapper(const int n) { return floor(unif_rand()*n); }

// it would appear that randomSample is still only a GNU g++ extension ?
#include <ext/algorithm>

// [[Rcpp::export]]
Rcpp::NumericVector randomSample(Rcpp::NumericVector a, int n) {
    // clone a into b to leave a alone
    Rcpp::NumericVector b(n);
    __gnu_cxx::random_sample(a.begin(), a.end(), 
                             b.begin(), b.end(), randWrapper);
    return b;
}

/** 
 * We can illustrate this on a simple example:
 */

/*** R
a <- 1:8
set.seed(42)
randomSample(a, 4)
set.seed(42)
randomSample(a, 4)
*/

