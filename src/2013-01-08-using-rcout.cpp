// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using Rcout for output synchronised with R
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics featured
 * @summary This post shows how to use Rcout (and Rcerr) for output
 *

 * The [Writing R Extensions](http://cran.r-project.org/doc/manuals/R-exts.html)
 * manual, which provides the gold standard of documentation as far as
 * extending R goes, strongly suggests to use `Rprintf` and `REprintf`
 * for output (from C/C++ code). The key reason is that these are
 * matched to the usual output and error streams maintained by R
 * itself.
 *
 * In fact, use of `std::cout` and `std::cerr` (as common in standard
 * C++ code) is flagged when running `R CMD check` and no longer
 * permitted when uploading to CRAN.
 *
 * Thanks to an initial patch by Jelmer Ypma, which has since been
 * reworked and extended, we have devices `Rcout` (for standard
 * output) and `Rcerr` (for standard error) which intercept output and
 * redirect it to R.
 *
 * To illustrate, we create a simple function which prints a value:
 */

#include <RcppArmadillo.h>   // as we use RcppArmadillo below
                             // this first example use only Rcpp 

using namespace Rcpp;

// [[Rcpp::export]]
void showValue(double x) {
    Rcout << "The value is " << x << std::endl;
}

/**
 * We can use this from R, and output will be properly synchronised:
 */

/*** R
cat("Before\n")
showValue(1.23)
cat("After\n")
 */

/**
 * During the 0.10.* abd 0.11.* releases, Rcpp itself still lacked the
 * converter code to "pretty-print simple non-scalar data
 * structures. But there always were alternatives. First,
 * RcppArmadillo can do so via its `operator<<()` as the
 * (Rcpp)Armadillo output is automatically redirected to R output
 * stream.  See below for recent alternatives from Rcpp itself.
 */

#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
void showMatrix(arma::mat X) {
    Rcout << "Armadillo matrix is" << std::endl << X << std::endl;
}

/*** R
M <- matrix(1:9,3,3)
print(M)
showMatrix(M)
*/

/**
 * Having output from R and C++ mix effortlessly is a very useful
 * feature. We hope to over time add more features to output more of
 * Rcpp basic objects. 
 */

/** 
 * Alternatively, starting with R version 0.11.5, we now have function
 * `print()` which can print any `SEXP` object -- by calling the internal R
 * function `Rf_PrintValue()`.
 *
 * A simple illustration follow. We first define helper function.
 */

// [[Rcpp::export]]
void callPrint(RObject x) { 
    Rcpp::print(x);             // will work on any SEXP object
}

/**
 * A few examples calls follow below.
 */

/*** R
callPrint(1:3)             # print a simple vector
callPrint(LETTERS[1:3])    # or characters
callPrint(matrix(1:9,3))   # or a matrix
callPrint(globalenv())    # or an environment object
*/

/**
 * Starting with version 0.12.1 of Rcpp, the `operator<<()` is also suppported for
 * vector and matrix types. See below for some examples.
 */

// [[Rcpp::export]]
void useOperatorOnVector(NumericVector x) { 
    Rcpp::Rcout << x << std::endl;
}

// [[Rcpp::export]]
void useOperatorOnMatrix(NumericMatrix x) { 
    Rcpp::Rcout << x << std::endl;
}

/*** R
v <- seq(0.0, 10.0, by=2.5)
useOperatorOnVector(v)
M <- matrix(seq(1.0, 16.0, by=1.0), 4, 4)
useOperatorOnMatrix(M)
*/
