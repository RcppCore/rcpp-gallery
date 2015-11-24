// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using the GSL to compute eigenvalues
 * @author Dirk Eddelbuettel
 * @updated Nov 23, 2015
 * @license GPL (>= 2)
 * @tags modeling gsl
 * @summary This example shows how to call a GSL function using RcppGSL
 *
 * Two previous posts showed how to compute eigenvalues 
 * [using Armadillo](../armadillo-eigenvalues) and 
 * [using Eigen](../eigen-eigenvalues/). As we have also looked at using the  
 * [GNU GSL](http://www.gnu.org/software/gsl/), this post will show how to
 * conpute eigenvalues using GSL.
 *
 * As mentioned in the [previous GSL post](../gsl-colnorm-example), we
 * instantiate C language pointers suitable for GSL (here the matrix
 * `M`). Prior to release 0.3.0 of RcppGSL, these *had to be freed manually*. 
 * However, since release 0.3.0 since is now taken care of via the standard
 * C++ mechanism of destructors.
 */

// Tell Rcpp to rely on the RcppGSL package to find GSL library and headers
// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>

// [[Rcpp::export]]
RcppGSL::Vector getEigenValues(RcppGSL::Matrix & M) {
    int k = M.ncol();

    RcppGSL::Vector ev(k);  	// instead of gsl_vector_alloc(k);
    gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(k);
    gsl_eigen_symm(M, ev, w);
    gsl_eigen_symm_free (w);

    return ev;				// return results vector  
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
