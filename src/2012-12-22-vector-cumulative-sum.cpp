/**
 * @title Using Sugar Function cumsum()
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar
 * @summary Demonstrates different ways to compute the cumulative
 *   sum of a vector and illustrates the use of sugar function cumsum().
 */
 
/**
 * The traditional way to compute the cumulative sum of a vector is with a
 * for loop. This is demonstrated with the function cumsum1().
 */
 
#include <Rcpp.h>
#include <numeric>   	// for std::partial_sum
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector cumsum1(NumericVector x){
    // initialize an accumulator variable
    double acc = 0;
        
    // initialize the result vector
    NumericVector res(x.size());
        
    for(int i = 0; i < x.size(); i++){
         acc += x[i];
         res[i] = acc;
    }
    return res;
}
 
/**
 * The C++ standard template library (STL) has the partial_sum() function
 * that computes the cumulative sum of a vector. This is demonstrated with
 * the function cumsum2().
 */
 
// [[Rcpp::export]]
NumericVector cumsum2(NumericVector x){
    // initialize the result vector
    NumericVector res(x.size());
    std::partial_sum(x.begin(), x.end(), res.begin());
    return res;
 }
 
/**
 * With Rcpp sugar, there is a cumsum() function which makes writing 
 * this function in C++ very similar to using the cumsum function in R.
 */

 
// [[Rcpp::export]]
NumericVector cumsum_sug(NumericVector x){
    return cumsum(x);    // compute the result vector and return it
}

/**
 * And we can of course compare the versions discussed here with the base R variant.
 */

/*** R
x <- 1:10
all.equal(cumsum1(x), cumsum2(x), cumsum_sug(x), cumsum(x))
*/
 
