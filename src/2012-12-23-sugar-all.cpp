/**
 * @title Using Sugar Function all()
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar
 * @summary Illustrates the use of sugar function all()
 *
 * The sugar function all() answers the question, "Are all of the values ... ?".
 */

/**
 * The all_sug() function takes a LogicalVector as an argument and allows one to
 * enter an expression for the argument as shown in the R examples. In this example,
 * it is simply wrapper around the sugar function all() and includes is_true to
 * return a boolean.
 * Note that when comparing two vectors, it is an element-wise comparison.
 * (i.e. `x[0] > y[0]`, ..., `x[n] > y[n]`)
 */

#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
bool all_sug(LogicalVector x) {
   // Note the use of is_true to return a bool type.
   return is_true(all(x == TRUE));
}
 
/**
 * While the above function may seem trivial, it can be easy to forget is_true() when
 * using all() and will result in a compile error. The check_equal() function below
 * is an example of a simple utility function to check two vectors for equality
 * using the all_sug() function defined above. 
 */
 
// [[Rcpp::export]]
void check_equal(NumericVector x, NumericVector y) {
	if(all_sug(x == y)) {
		Rcout << "Success! The input vectors are equal" << std::endl;
		// do something
	} else {
		Rcout << "Fail! The input vectors are not equal" << std::endl;
		// do something else
	}
}
 
 
/*** R
x <- c(3, 9, 0, 2, 7, 5, 6)
y <- c(0, 0, 0, 0, 0, 0, 0)
all_sug(x < 10)
all_sug(x != 3)
all_sug(x >= y)
all_sug(y == 0)
check_equal(x, y)
check_equal(x, c(3, 9, 0, 2, 7, 5, 6))
*/

