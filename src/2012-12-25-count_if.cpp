/**
 * @title Using LogicalVector
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar
 * @summary Illustrates the use of logical vectors
 */

/**
 * The fact that binary logical operators such as `x < 4` create a
 * logical sugar expression (i.e. a LogicalVector type)
 * is very powerful. This enables one to easily write simple
 * and expressive functions with a LogicalVector as an argument.
 * Any of the logical operators `<, <=, >, >=, ==, !=` can be used
 * to create a logical sugar expression. The Rcpp sugar documentation
 * has additional examples of using binary logical operators.
 */

#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
int count_if(LogicalVector x) {
	int counter = 0;
	for(int i = 0; i < x.size(); i++) {
		if(x[i] == TRUE) {
			counter += 1;
		}
	}
	return counter;
}

/*** R
 x <- 1:10
 count_if(x < 4)
 count_if(x != 8)
 */

/**
 * A simple function using just C++ and the STL to count the
 * number of elements in a vector less than a given number could
 * be written as follows. While this function is simple, the
 * downside is that additional functions will have to be
 * written for other logical operators and other types.
 */

#include <vector>
#include <functional>
#include <algorithm>

// [[Rcpp::export]]
int count_if_lt(std::vector<double> x, int n) {
	return count_if(x.begin(), x.end(), bind2nd(std::less<double>(), n));
}

/*** R
 x <- 1:10
 count_if_lt(x, 4)
 */
