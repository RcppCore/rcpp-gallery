// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Creating xts objects from source
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics 
 * @summary This post shows how to create an xts object at C++ source level
 *
 * A recent post showed how to 
 * [access the attributes of an xts object](../getting-attributes-for-xts-example/). 
 * We can also do the inverse in order to _create_ an xts object.
 *
 */


#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::NumericVector createXts() {

    IntegerVector ind = seq(1, 10);

    NumericVector dv(ind);
    dv = dv * 86400;
    dv.attr("tzone")    = "UTC";
    dv.attr("tclass")   = "Date";

    NumericVector xv(ind);
    xv.attr("dim")      = IntegerVector::create(10,1);
    xv.attr("index")    = dv;
    CharacterVector klass = CharacterVector::create("xts", "zoo");
    xv.attr("class")    = klass;
    xv.attr(".indexCLASS") = "Date";
    xv.attr("tclass")   = "Date";
    xv.attr(".indexTZ") = "UTC";
    xv.attr("tzone")   = "UTC";
    
    return xv;

}


/**
 * We can run this, and look at the attributes the generated object:
 */

/*** R
suppressMessages(library(xts))
foo <- createXts() 
foo
attributes(foo)
*/

/**
 * It turns out that creating an `xts` object the usual creates an identical object:
 */

/*** R
bar <- xts(1:10, order.by=as.Date(1:10)) 
all.equal(foo, bar)
*/

/**
 * So now we can create `xts` objects at the source level.
 */
