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
 * that are very fast because they call Fortran and C routines. With Rcpp and
 * the C++ STL one can easily write run functions to use in R.
 */

#include <Rcpp.h>
#include <numeric>      // for accumulate

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector run_sum(NumericVector x, int n) {
    int sz = x.size();
    NumericVector res(sz);
    
    res[n-1] = std::accumulate(x.begin(), x.end()-sz+n, 0.0);
    
    for(int i = n; i < sz; i++) {
       res[i] = res[i-1] + x[i] - x[i-n];
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}

/*** R
suppressMessages(library(TTR))

# use a small sample set for demonstration purposes
set.seed(123)
x <- rnorm(10)
x
n <- 4

# Check to make sure that the result of run_sum matches
# runSum from the TTR package
stopifnot(all.equal(run_sum(x, n), runSum(x, n)))

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
# Check to make sure that the result of run_mean matches
# runMean from the TTR package
stopifnot(all.equal(run_mean(x, n), runMean(x, n)))

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
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = *std::min_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}

/*** R
# Check to make sure that the result of run_min matches
# runMin from the TTR package
stopifnot(all.equal(run_min(x, n), runMin(x, n)))

run_min(x, n)
 */

// [[Rcpp::export]]
NumericVector run_max(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = *std::max_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}

/*** R
# Check to make sure that the result of run_max matches
# runMax from the TTR package
stopifnot(all.equal(run_max(x, n), runMax(x, n)))

run_max(x, n)
 */

/**
 * This post demonstrates how to incorporate a few useful functions 
 * from the STL, `accumulate`, `min_element`, and 
 * `max_element`, to write 'run' functions with Rcpp.
 */
