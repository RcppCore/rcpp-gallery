// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Getting attributes to use xts objects
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics xts
 * @summary We show how to access attributes and illustrate the use with an xts object.
 *
 * [An earlier post](../setting-object-attributes) illustrated that R object attributes
 * can be set at the C++ level.  Naturally, we can also read them from an object. This proves 
 * particularly useful for [xts](http://cran.r-project.org/package=xts) objects which are, in 
 * essence, numerical matrices with added attributed that are used by a rich set of R operators 
 * and functions. 
 *
 * Here, we show how to access these attributes.
 *
 */


#include <Rcpp.h>

using namespace Rcpp;
 
// [[Rcpp::export]]
std::vector<std::string> xtsAttributes(NumericMatrix X) {
    std::vector<std::string> nm = X.attributeNames();
    return nm;
}

/** 
 * A first example simply creates a random `xts` object of twenty observations. 
 * We then examine the set of attributes and return it in a first program.
 */

/*** R
    suppressMessages(library(xts))
    set.seed(42)
    n <- 20
    Z <- xts(100+cumsum(rnorm(n)), order.by=ISOdatetime(2013,1,12,20,21,22) + 60*(1:n))
    xtsAttributes(Z)
*/

/**
 * The same result is seen directly in R:
 */

/*** R
    names(attributes(Z))
    all.equal(xtsAttributes(Z), names(attributes(Z)))
*/


/**
 * Now, given the attributes we can of course access some of these.
 * The `index()` function `xts` objects returns the index. Here, we know
 * we have a `Datetime` object so we can instantiate it at the C++ level.
 * (Real production code would test types etc).
 */


// [[Rcpp::export]]
DatetimeVector xtsIndex(NumericMatrix X) {
    DatetimeVector v(NumericVector(X.attr("index")));
    return v;
}

/*** R
    xtsIndex(Z)
*/

/**
 * Further operations such as subsetting based on the datetime vector
 * or adjustments to time zones are left as an exercise.
 */

/**
 * Edited on 2014-03-28 to reflect updated / simpliefied attributes functions.
 *
 */
