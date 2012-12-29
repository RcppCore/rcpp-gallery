/**
 * @title Run Functions with Rcpp
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags stl
 * @summary Demonstrates writing functions over a running window
 */

/**
 * Writing running functions in R can be slow because of the loops
 * involved. The TTR package contains several run functions
 * that are very fast because they call Fortran routines. With Rcpp and
 * the C++ STL one can easily write run functions to use in R.
 */

#include <Rcpp.h>
#include <algorithm>    // for max_element and min_element
#include <numeric>      // for accumulate

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector run_sum(NumericVector x, int n) {
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = std::accumulate(x.begin()+i, x.end()-sz+n+i, 0.0);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
	return res;
}

/*** R
 x <- sample(10)
 x
 n <- 3
 run_sum(x, n)
 */

/**
 * Now that we have the function `run_sum` written, we can easily
 * use this to write the function `run_mean` just by dividing
 * `run_sum` by `n`. Another point to note is the syntax
 * to cast `n` to a double so we do not lose the decimal points
 * with integer division.
 */

// [[Rcpp::export]]
NumericVector run_mean(NumericVector x, int n) {
    return run_sum(x, n) / (double)n;
}

/*** R
 run_mean(x, n)
 */

/**
 * With `min_element` and `max_element` from the algorithm header, one can
 * also easily write a function that calculates the min and the
 * max of a range over a running window. Note the `*` to dereference
 * the iterator to obtain the value. See [Finding the minimum of a vector](http://gallery.rcpp.org/articles/vector-minimum/)
 * for another example of using `min_element`.
 */

// [[Rcpp::export]]
NumericVector run_min(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = *std::min_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}

/*** R
 run_min(x, n)
 */

// [[Rcpp::export]]
NumericVector run_max(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = *std::max_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}

/*** R
 run_max(x, n)
 */

/**
 * This post demonstrates how to incorporate a few useful functions 
 * from the STL. The STL functions `accumulate`, `min_element`, and 
 * `max_element` utilize iterators to iterate over a range. The run
 * functions above demonstrate how to use a for loop and and 
 * some math with the iterators to write running functions.
 */
