/**
 * @title Using Sugar for High Level Vector Operations
 * @author Romain Francois 
 * @license GPL (>= 2)
 * @tags sugar featured
 * @summary Demonstrates using Rcpp sugar's numeric and comparison operators
 *   and high-level logical functions to simplify code.
 * 
 * This example will take a C++ function written using the standard Rcpp
 * API and transform it to something looking much more like it's R 
 * equivilant using Rcpp sugar.
 * 
 * The following function `foo` transforms two numeric vectors into a 
 * third one:
 */

#include <Rcpp.h>
using namespace Rcpp;

NumericVector foo(NumericVector x, NumericVector y) {
   int n = x.size() ;
   NumericVector res( n ) ;
   double x_ = 0.0, y_ = 0.0 ;
   for( int i=0; i<n; i++) {
      x_ = x[i] ;
      y_ = y[i] ;
      if( x_ < y_ ){
         res[i] = x_ * x_ ;
      } else {
         res[i] = -( y_ * y_) ;
      }
   }
   return res ;
}

/**
 * While this code is performant, the equivilant R code would be much shorter:
 */

/*** R
foo <- function(x, y){
   ifelse( x < y, x*x, -(y*y) )
}
*/

/**
 * Rcpp sugar enables us to write C++ code that operates on entire vectors
 * much like we do in R. Re-writing using the sugar `ifelse` function and
 * numeric and comparison operators yields the identical one-line
 * implementation:
 */

NumericVector fooSugar(NumericVector x, NumericVector y) {
   return ifelse( x < y, x*x, -(y*y) );
}

