/**
 * @title Using LogicalVector
 * @author Ross Bennett
 * @license GPL (>= 2)
 * @tags sugar stl
 * @summary Illustrates the use of logical vectors
 */

/**
 * The fact that expressions with binary logical operators 
 * such as `x < 4` create a logical sugar expression 
 * (i.e. a LogicalVector type) is very powerful. This enables
 * one to easily write simple and expressive functions with
 * a LogicalVector as an argument.
 * Any of the logical operators `<, <=, >, >=, ==, !=` can be used
 * to create a logical sugar expression. The [Rcpp sugar](http://cran.r-project.org/web/packages/Rcpp/vignettes/Rcpp-sugar.pdf)
 * vignette has additional examples of using binary logical operators.
 */

#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
int count_if(LogicalVector x) {
    int counter = 0;
    for(int i = 0; i < x.size(); i++) {
        if(x[i] == TRUE) {
            counter ++;
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
 * Please see the references for the [functional](http://www.cplusplus.com/reference/functional/) and [algorithm](http://www.cplusplus.com/reference/algorithm/)
 * headers for information regarding `std::less`, `bind2nd`, and `count_if`.
 */

#include <vector>        // for std::vector
#include <functional>    // for std::less and bind2nd
#include <algorithm>     // for count_if

// [[Rcpp::export]]
int count_if_lt(std::vector<double> x, int n) {
    return count_if(x.begin(), x.end(), bind2nd(std::less<double>(), n));
}

/*** R
 x <- 1:10
 count_if_lt(x, 4)
 */
