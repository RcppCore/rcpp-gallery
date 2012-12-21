/**
 * @title Finding the minimum of a vector
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl featured
 * @summary Demonstrates how STL's min_element can be used.
 * 
 * This example was motivated by http://stackoverflow.com/questions/5158219/find-minimum-of-vector-in-rcpp
 * and addresses to find how to find the minumum value and its position index in vector.
 *
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double vecmin(NumericVector x) {
  // Rcpp supports STL-style iterators
  NumericVector::iterator it = std::min_element(x.begin(), x.end());
  // we want the value so dereference 
  return *it;
}

/**
 * We can also use the iterator to compute the position, simply by
 * taking the offset to the vector beginning.
 */

// [[Rcpp::export]]
int vecminInd(NumericVector x) {
  // Rcpp supports STL-style iterators
  NumericVector::iterator it = std::min_element(x.begin(), x.end());
  // we want the value so dereference 
  return it - x.begin();
}

/** 
 * A quick illustration follows. Note that we pad the position by one to adjust for the 0-based versus 1-based indexing between C++ and R.
 */

/*** R
set.seed(5)
x <- sample(1:100, 10)  # ten out 100
x
cat("Min is ", vecmin(x), " and at position ", vecminInd(x)+1, "\n")
*/


/**
 * Of course, we subsequently added `min` and `which_min` as sugar
 * functions, but this example still illustrated how useful the STL
 * algoritms can be.
 */
