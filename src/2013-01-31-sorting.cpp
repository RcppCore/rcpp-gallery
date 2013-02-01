/**
 * @title Sorting Numeric Vectors in C++ and R
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags stl benchmark
 * @summary Illustrates the comparison of different sorting algorithms with R
 *     and the C++ STL.
 */


/**
 * Consider the problem to sort all elements of the given vector in ascending
 * order. We can simply use the function `std::sort` from the C++ STL.
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector stl_sort(NumericVector x) {
   NumericVector y = clone(x);
   std::sort(y.begin(), y.end());
   return y;
}

/*** R
library(rbenchmark)
set.seed(123)
z <- rnorm(100000)
x <- rnorm(100)

# check that stl_sort is the same as sort
stopifnot(all.equal(stl_sort(x), sort(x)))

# benchmark stl_sort and sort
benchmark(stl_sort(z), sort(z), order="relative")[,1:4]
*/

/**
 * Consider the problem of sorting the first `n` elements of a given vector.
 * The function `std::partial_sort` from the C++ STL does just this. 
 */

// [[Rcpp::export]]
NumericVector stl_partial_sort(NumericVector x, int n) {
   NumericVector y = clone(x);
   std::partial_sort(y.begin(), y.begin()+n, y.end());
   return y;
}

/**
 * An alternate implementation of a partial sort algorithm is to use 
 * `std::nth_element` to partition the given vector at the nth sorted
 * element and then use `std::sort`, both from the STL,  to sort the vector
 * from the beginning to the nth element.
 * 
 * For an equivalent implementation in R, we can use the `sort` function by
 * specifying a vector of `1:n` for the partial argument (i.e. `partial=1:n`).
 */

// [[Rcpp::export]]
NumericVector nth_partial_sort(NumericVector x, int nth) {
   NumericVector y = clone(x);
   std::nth_element(y.begin(), y.begin()+nth, y.end());
   std::sort(y.begin(), y.begin()+nth);
   return y;
}

/*** R
n <- 25000

# check that stl_partial_sort is equal to nth_partial_sort
stopifnot(all.equal(stl_partial_sort(x, 50)[1:50], 
                    nth_partial_sort(x, 50)[1:50]))

# benchmark stl_partial_sort, nth_element_sort, and sort
benchmark(stl_partial_sort(z, n),
          nth_partial_sort(z, n),
          sort(z, partial=1:n),
          order="relative")[,1:4]
*/

/**
 * An interesting result to note is the gain in speed of 
 * `nth_partial_sort` over `stl_partial_sort`. In this case, for the given
 * data, it is faster to use the combination of`std::nth_element` and 
 * `std::sort` rather than `std::partial_sort` to sort the first `n` elements 
 * of a vector.
 */

// [[Rcpp::export]]
NumericVector stl_nth_element(NumericVector x, int n) {
   NumericVector y = clone(x);
   std::nth_element(y.begin(), y.begin()+n, y.end());
   return y;
}

/**
 * Finally, consider a problem where you only need a single element of a
 * sorted vector. The function `std::nth_element` from the C++ STL does just 
 * this. An example of this type of problem is computing the median of a given
 * vector.
 * 
 * For an equivalent implementation in R, we can use the `sort` function by
 * specifying a scalar value for the argument partial (i.e. `partial=n`).
 */

/*** R
# check that the nth sorted elements of the vectors are equal
stopifnot(all.equal(stl_nth_element(x, 43)[43], sort(x, partial=43)[43]))

# benchmark nth_element and sort
benchmark(stl_nth_element(z, n),
         sort(z, partial=n),
         order="relative")[,1:4]
*/

/**
 * While these are not huge speed improvements over the base R sort function, 
 * this post demonstrates how to easily access sorting functions in the C++
 * STL and is a good exercise to better understand the differences and 
 * performance of the sorting algorithms available in C++ and R.
 */
