// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL random_shuffle for permutations
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl featured
 * @updated Jan 31, 2023
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

// [[Rcpp::plugins(cpp11)]]    // see below

#include <Rcpp.h>

// wrapper around R's RNG such that we get a uniform distribution over
// [0,n) as required by the STL algorithm
inline int randWrapper(const int n) { return floor(unif_rand()*n); }

// [[Rcpp::export]]
Rcpp::NumericVector randomShuffle(Rcpp::NumericVector a) {

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
 * By connecting the STL implementation of the random permutation to the
 * random number generato from R, we are able to compute reproducible permutations, fast
 * and from C++.
 */

/**
 * Jan 2023 Update: With the C++17 language standard, the
 * `std::random_shuffle()` function has been removed with the signature used
 * here. The suggested alternative is now `std::shuffle()` taking as before
 * two iterators for the vector to be shuffled, but then an instance of a
 * C++ random number generator.
 *
 * That unfortunately breaks our illustration which
 * relied on using R's own RNG.  So another alternative is provided below; it
 * was kindly provided by Kenta Maehashi in
 * [GitHub issue #143](https://github.com/RcppCore/rcpp-gallery/issues/143).
 * We should note that it does not perfectly replicate the sequence though it
 * appears to shuffle appropriately.
 *
 * To make the initial version compile under current setups, we added an
 * explicit setting for C++11 to it.
 *
 */

// [[Rcpp::export]]
Rcpp::NumericVector randomShuffle2(Rcpp::NumericVector a) {
    // clone a into b to leave a alone
    Rcpp::NumericVector b = Rcpp::clone(a);
    int n = b.size();
    int j;

    // Fisher-Yates Shuffle Algorithm
    for (int i = 0; i < n - 1; i++) {
      j = i + randWrapper(n - i);
      std::swap(b[i], b[j]);
    }
    return b;
}


/*** R
a <- 1:8
set.seed(42)
randomShuffle(a)
set.seed(42)
randomShuffle2(a)
*/
