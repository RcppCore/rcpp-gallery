/**
 * @title Using Sugar Function diff()
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar
 * @summary Illustrates the use of sugar function diff()
 */

/**
 * The sugar function diff() computes the difference of consecutive elements
 * (i.e. lag = 1) of the input vector. Note that the size of the vector returned
 * is one less than the input vector. The sugar function diff() works the same
 * way as the diff() function in base R, except the lag is not specified.
 */

#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector diff_sug(NumericVector x){
   return diff(x);
}
 
/**
 * One can use the diff() function to compute one period simple returns of stock
 * prices.
 */
 
// [[Rcpp::export]]
NumericVector ret_simple(NumericVector x) {
   NumericVector vec_diff = diff(x);
   NumericVector res(x.size());
   // pad the front with an NA
   res[0] = NA_REAL;
   for(int i = 1; i < res.size(); i++) {
      res[i] = vec_diff[i-1] / x[i-1];
   }
   return res;
}

/*** R
 x <- rnorm(10)
 # Close prices of S&P 500
 y <- c(1418.55, 1427.84, 1428.48, 1419.45, 1413.58, 
        1430.36, 1446.79, 1435.81, 1443.69, 1430.15)
 diff_sug(x)
 # base R function
 diff(x)
 ret_simple(y)
 */
