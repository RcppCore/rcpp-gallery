/**
 * @title Using Sugar Function any()
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar
 * @summary Illustrates the use of sugar function any()
 *
 * The sugar function any() answers the question, "Are any of the values ... ?".
 * 
 */
 
/**
 * The any_sug() function takes a LogicalVector as an argument and allows one to
 * enter an expression for the argument as shown in the R examples. In this
 * example, it is simply wrapper around the sugar function any() and includes
 * is_true to return a boolean. 
 * 
 * Note that when comparing two vectors, it is an element-wise comparison. 
 * (i.e. `x[0] > y[0]`, ..., `x[n] > y[n]`)
 */
 
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
bool any_sug(LogicalVector x){
   // Note the use of is_true to return a bool type
   return is_true(any(x == TRUE));
}

/*** R
 x <- c(3, 9, 0, 2, 7, -1, 6)
 y <- c(8, 3, 2, 6, 1, 5, 0)
 any_sug(x < 10)
 any_sug(x != 3)
 any_sug(x >= y)
 any_sug(y == 0)
*/
 
/**
 * While the above function may seem trivial, it can be easy to forget is_true()
 * when using any() and will result in a compile error. The check_negative()
 * function below is an example of a simple utility function to check if a
 * vector contains negative values using the any_sug() function defined above.
 */
 
// [[Rcpp::export]]
void check_negative(NumericVector x) {
   if(any_sug(x < 0)) {
      Rcout << "The vector contains negative numbers" << std::endl;
      // do something
   } else {
      Rcout << "The vector does not contain negative numbers" << std::endl;
      // do something else
   }
}
 
/*** R
check_negative(x)
check_negative(y)
 */
