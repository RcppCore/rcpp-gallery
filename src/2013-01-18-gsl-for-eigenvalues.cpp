// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using the GSL to compute eigenvalues
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags modeling gsl
 * @summary This example shows how to call a GSL function using RcppGSL
 *
 * Two posts showed how to compute eigenvalues 
 * [using Armadillo](../armadillo-eigenvalues) and 
 * [using Eigen](../eigen-eigenvalues/). As we also looked at using the  
 * [GNU GSL](http://www.gnu.org/software/gsl/), this post will show how to
 * conpute eigenvalues using GSL.
 *
 * As mentioned in the [previous GSL post](../gsl-colnorm-example), we
 * instantiate C language pointers suitable for GSL (here the matrix
 * `M`). Those *must* be freed manually, as shown before the `return`
 * statement.  
 */

// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>

// [[Rcpp::export]]
Rcpp::NumericVector getEigenValues(Rcpp::NumericMatrix sM) {

    RcppGSL::matrix<double> M(sM); 	// create gsl data structures from SEXP
    int k = M.ncol();
    Rcpp::NumericVector N(k); 		// to store results 

    gsl_vector *eigval = gsl_vector_alloc(k);
    gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(k);
    gsl_eigen_symm (M, eigval, w);
    gsl_eigen_symm_free (w);

    for (int j = 0; j < k; j++) {
        N[j] = gsl_vector_get(eigval, j);
    }
    M.free();                          // important: GSL wrappers use C structure
    return N;				// return vector  
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
