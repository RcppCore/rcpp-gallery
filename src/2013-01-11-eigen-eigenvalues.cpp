// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using Eigen for eigenvalues
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags eigen matrix
 * @summary This example shows how to compute eigenvalues using Eigen
 *

 * [A previous post](../armadillo-eigenvalues) showed how to compute 
 * eigenvalues using the [Armadillo](http://arma.sf.net) library via RcppArmadillo.
 *
 * Here, we do the same using [Eigen](http://eigen.tuxfamily.org) and
 * the RcppEigen package.
 *
 */

#include <RcppEigen.h>

// [[Rcpp::depends(RcppEigen)]]

using Eigen::Map;               	// 'maps' rather than copies 
using Eigen::MatrixXd;                  // variable size matrix, double precision
using Eigen::VectorXd;                  // variable size vector, double precision
using Eigen::SelfAdjointEigenSolver;    // one of the eigenvalue solvers

// [[Rcpp::export]]
VectorXd getEigenValues(Map<MatrixXd> M) {
    SelfAdjointEigenSolver<MatrixXd> es(M);
    return es.eigenvalues();
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
 * Eigen has other _a lot_ of other decompositions, see [its documentation](http://eigen.tuxfamily.org/) 
 * for more details.
 */
