/**
 * @title Sugar Functions head and tail
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar stl
 * @summary Illustrate use of sugar functions head and tail
 */

/**
 * The sugar function `head` returns the first n elements of the input vector.
 * The sugar function `tail` returns the last n elements of the input vector.
 * With Rcpp sugar, the functions `head` and `tail` work the same way
 * as they do in R.
 */

/**
 * Here we use `std::sort` from the STL and then `tail` to return the top
 * n items (items with the highest values) of the input vector.
 */

#include <algorithm>	// for sort
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector top_n(NumericVector x, int n){
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return tail(x, n);
}

/*** R
x <- rnorm(10)
x
top_n(x, 3)
*/

/**
 * Here we use `std::sort` from the STL and then `head` to return the bottom
 * n items (items with the lowest values) of the input vector.
 */

// [[Rcpp::export]]
NumericVector bottom_n(NumericVector x, int n){
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return head(x, n);
}

/*** R
bottom_n(x, 3)
*/