// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Accessing environments
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics environment
 * @summary This example shows how to access R environments
 *

 * Extending R with C++ code by using Rcpp typically involves function
 * calls by leveraging the existing `.Call()` interface of the R API.
 * Passing values back and forth is then done in manner similar to
 * programming with functions.
 *
 * However, on occassion it is useful to access enviroments (such as
 * the global environment). We can also pass environments (which are
 * first-class datatypes for R) around to instantiate the Rcpp class
 * `Environment`.
 *
 * Here we illustrating how to access values from the global environment.
 *
 * First, let us set some values:
 *
 */

/*** R
someNumber <<- 42
stooges <<- c("moe", "larry", "curly")
*/

/**
 * We can access these values in a C++ function setup with Rcpp:
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
int checkEnv() {
    // access the global env.
    Environment env = Environment::global_env();
    CharacterVector v = env["stooges"];
    Rcout << "Stooge Nb 2 is: " << v[1] << std::endl;
    return env["someNumber"];
}

/**
 * Running the example yields:
 */

/*** R
checkEnv()
*/
