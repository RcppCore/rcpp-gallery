/**
 * @title Reversing a Vector
 * @author Dirk Eddelbuettel
 * @license MIT
 * @tags stl
 * @summary Compares reversing a numeric vector the R C API and Rcpp 
 *   with an STL algorithm.
 */
 
/** 
 * To start with and for purposes of comparison, we reverse a numeric
 * vector using the R C API (note that this example was taken from 
 * Jeff Ryan's esotericR package):
 */
 
#include <Rcpp.h>
using namespace Rcpp;

SEXP rev (SEXP x) {
  SEXP res;
  int i, r, P=0;
  PROTECT(res = Rf_allocVector(REALSXP, Rf_length(x))); P++;
  for(i=::Rf_length(x), r=0; i>0; i--, r++) {
     REAL(res)[r] = REAL(x)[i-1];
  }
  ::Rf_copyMostAttrib(x, res);
  UNPROTECT(P);
  return res;
}

/**
 * Here's the same operation implemented using Rcpp and calling the 
 * `std::reverse` function from the C++ standard library:
 */

// [[Rcpp::export]]
NumericVector rcppRev(NumericVector x) {
   NumericVector revX = clone<NumericVector>(x);
   std::reverse(revX.begin(), revX.end());
   ::Rf_copyMostAttrib(x, revX); 
   return revX;
} 

/**
 * Here's an illustration of calling our `rcppRev` function from R:
 */

/*** R
obj <- structure(seq(0, 1, 0.1), obligatory="hello, world!")
obj
rcppRev(obj)
*/

/**
 * Both the `obj` variable and the new copy contain the desired data attribute,
 * the new copy is reversed, the original is untouched. All in four lines of
 * C++ not requiring explicit memory managment or easy to get wrong array
 * manipulations.
 */


