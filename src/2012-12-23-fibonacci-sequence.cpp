/**
 * @title Accelerating recursive function calls in the Fibonacci sequence
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags recursion function benchmark featured
 * @summary This example shows how to call a recursive function
 *
 * A [StackOverflow post](http://stackoverflow.com/questions/6807068/why-is-my-recursive-function-so-slow-in-r)
 * once put the question directly in its title: <em>Why is my recursive function so slow in R?</em>
 *
 * To cut a long story short, function calls are among the less
 * performing parts of the R language. Operating on object and the
 * language gives us very powerful features, but the required state
 * and stack checking for function calls is one of the prices we pay.
 *
 * However, as the [Fibonacci
 * sequence](http://en.wikipedia.org/wiki/Fibonacci_number) has such a
 * simple definition, the simple R program can be translated easily
 * giving us a nice example for the power of C++ particularly for function evaluations.
 *
 * All that said, real computer scientists do of course insist that 
 * one should not call the sequence recursively. See for example the 
 * [this post](http://bosker.wordpress.com/2011/04/29/the-worst-algorithm-in-the-world/). 
 * and memoization approaches are easy in R too.
 * 
 * Let us start with the R function:
 */

/*** R
## create M as a sum of two outer products
fibR <- function(n) {
    if ((n == 0) | (n == 1)) 
        return(1)
    else
        return(fibR(n-1) + fibR(n-1))
}

fibR(10)
 */

/** 
 * This translates almost literally in C++:
 */


#include <Rcpp.h>

// [[Rcpp::export]]
int fibCpp(int n) {
    if ((n == 0) | (n == 1)) 
        return 1;
    else
        return fibCpp(n-1) + fibCpp(n-1);
}

/*** R
fibCpp(10)
 */

/** 
 * We can time this thanks to the rbenchmark package:
 */

/*** R
library(rbenchmark)

benchmark(fibR(10), fibCpp(10))
*/
