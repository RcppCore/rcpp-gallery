// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Creating xts objects from source
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics 
 * @summary This post shows how to create an xts object at C++ source level
 *
 * A recent post showed how to access the 
 * [attributes of an xts object](../getting-attributes-for-xts-example/). 
 * We used an `xts` object as these are powerful and popular---but any R object
 * using attributed could be used to illustrate the point.
 *
 * In this short post, we show how one can also do the inverse in
 * order to _create_ an xts object at the C++ source level.
 *
 * We use a somewhat useless object with values from `1:10` index by
 * dates in the same range. As zero corresponds to the epoch, these
 * will be early 1970-dates. But the values do not matter when showing
 * the principle.
 */


#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::NumericVector createXts() {

    IntegerVector ind = seq(1, 10);      // values

    NumericVector dv(ind);               // date(time)s are real values
    dv = dv * 86400;                     // scaled to days
    dv.attr("tzone")    = "UTC";         // the index has attributes
    dv.attr("tclass")   = "Date";

    NumericVector xv(ind);               // data her same index
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
 * We can run this function, and look at the (numerous) attributes in the generated object:
 */

/*** R
suppressMessages(library(xts))
foo <- createXts() 
foo
attributes(foo)
*/

/**
 * It turns out that creating an `xts` object the usual way creates an object that is equal:
 */

/*** R
bar <- xts(1:10, order.by=as.Date(1:10)) 
all.equal(foo, bar)
*/

/**
 * So now we can create `xts` objects at the source level.  
 */
