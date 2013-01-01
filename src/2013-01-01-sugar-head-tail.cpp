/**
 * @title Sugar Functions head and tail
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar stl
 * @summary Illustrate use of sugar functions head and tail
 */

/**
 * The R functions `head` and `tail` return the first (last) n elements
 * of the input vector.  With Rcpp sugar, the functions `head` and `tail`
 * work the same way as they do in R.
 *  
 * Here we use `std::sort` from the STL and then `tail` to return the top
 * n items (items with the highest values) of the input vector.
 */

#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector top_n(NumericVector y, int n){
    NumericVector x = clone(y);
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return tail(x, n);
}

/** 
 * A simple illustration:
 */

/*** R
set.seed(42)
x <- rnorm(10)
x
top_n(x, 3)
*/

/**
 * Here we use `std::sort` from the STL and then `head` to return the bottom
 * n items (items with the lowest values) of the input vector.
 */

// [[Rcpp::export]]
NumericVector bottom_n(NumericVector y, int n){
    NumericVector x = clone(y);
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return head(x, n);
}

/*** R
bottom_n(x, 3)
*/
