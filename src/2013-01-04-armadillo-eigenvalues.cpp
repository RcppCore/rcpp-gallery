// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Armadillo eigenvalues
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags armadillo matrix featured
 * @summary This example shows how to compute eigenvalues easily
 *

 * Today a (slightly confused) question on 
 * [StackOverflow](http://stackoverflow.com/questions/14164972/eigenvalues-calculations-in-c-within-r-codes/14165455) 
 * wondered how to access R's facilities for eigenvalues calculations
 * from C code.
 *
 * For this, we need to step back and consider how this is done. In
 * fact, R farms the calculation out to the BLAS. On could possibly
 * access R's functions---but would then have to wrestle with the data
 * input/output issues which make Rcpp shine in comparison.  Also,
 * Rcpp gets us access to Armadillo (via the RcppArmadillo) package
 * and Armadillo's main focus are exactly the linear algebra
 * calculations and decompositions.  
 *
 * And with facilities that were added to Rcpp in the 0.10.* release
 * series, this effectively becomes a one-liner of code! (Nitpickers
 * will note that there are also one include statement, two attributes
 * declarations and the function name itself.)
 */


#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
arma::vec getEigenValues(arma::mat M) {
    return arma::eig_sym(M);
}

/**
 * We can illustrate this easily via a random sample matrix.
 */

/*** R
set.seed(42)
X <- matrix(rnorm(4*4), 4, 4)
Z <- X %*% t(X)

getEigenValues(Z)
*/

/**
 * In comparison, R gets the same results (in reverse order) and also returns the eigenvectors.
 */

/*** R
eigen(Z)
*/

/** 
 * Armadillo has other eigenvector computations too, see [its documentation](http://arma.sourceforge.net/docs.html#eig_sym).
 */
