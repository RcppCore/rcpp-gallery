// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Sparse matrix in Armadillo
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags armadillo matrix featured sparse
 * @summary This example shows how to create a sparse matrix in Armadillo
 *
 * The Matrix package in R supports sparse matrices, and we can use
 * the S4 class in Rcpp to attach the different component row indices,
 * column pointers and value which can then be used to initialize an
 * Armadillo sparse matrix.
 *
 * Let's start with creating a sparse matrix.
 *
 */

/*** R
suppressMessages({
  library(methods); library(Matrix)
})
i <- c(1,3:8) 
j <- c(2,9,6:10) 
x <- 7 * (1:7)
A <- sparseMatrix(i, j, x = x) 
print(A)
*/

/**
 * The following C++ function access the corresponding slots of the
 * `sparseMatrix` object, and creates a `sp_mat` Armadillo object.
 */


#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export]]
void convertSparse(S4 mat) {         // slight improvement with two non-nested loops

    IntegerVector dims = mat.slot("Dim");
    arma::urowvec i = Rcpp::as<arma::urowvec>(mat.slot("i"));
    arma::urowvec p = Rcpp::as<arma::urowvec>(mat.slot("p"));     
    arma::vec x     = Rcpp::as<arma::vec>(mat.slot("x"));

    int nrow = dims[0], ncol = dims[1];
    arma::sp_mat res(nrow, ncol);

    // create space for values, and copy
    arma::access::rw(res.values) = arma::memory::acquire_chunked<double>(x.size() + 1);
    arma::arrayops::copy(arma::access::rwp(res.values), x.begin(), x.size() + 1);

    // create space for row_indices, and copy
    arma::access::rw(res.row_indices) = arma::memory::acquire_chunked<arma::uword>(i.size() + 1);
    arma::arrayops::copy(arma::access::rwp(res.row_indices), i.begin(), i.size() + 1);
    
    // create space for col_ptrs, and copy 
    arma::access::rw(res.col_ptrs) = arma::memory::acquire<arma::uword>(p.size() + 2);
    arma::arrayops::copy(arma::access::rwp(res.col_ptrs), p.begin(), p.size() + 1);

    // important: set the sentinel as well
    arma::access::rwp(res.col_ptrs)[p.size()+1] = std::numeric_limits<arma::uword>::max();
    
    // set the number of non-zero elements
    arma::access::rw(res.n_nonzero) = x.size();

    Rcout << "SpMat res:\n" << res << std::endl;
}

/**
 * Running this example shows the same matrix printed to `stdout` by
 * Armadillo.
 */

/*** R
convertSparse(A)
*/

/**
 * Support for sparse matrix is currently still limited in Armadillo,
 * but expected to grow.  Likewise, RcppArmadillo does not yet have
 * converters such as `as<>()` (though the example above does 
 * essentially everything that is needed) and `wrap()` converters.
 * But we expect to add these
 * eventually --- at which point this example will be much simpler.
 */
