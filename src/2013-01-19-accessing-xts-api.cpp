// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using Rcpp to access the C API of xts 
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags featured xts
 * @summary This post shows how to use the exported API functions of xts
 *
 * Jeff Ryan's [xts](https://r-forge.r-project.org/projects/xts/) package is an immensely powerful
 * tool that is widely used for timeseries work with R.  Recently, the question about how to
 * use it from Rcpp came up [on StackOverflow](http://stackoverflow.com/questions/14274055/how-to-use-c-api-of-xts-package-in-rcpp)
 * and in [a thread on the rcpp-devel list](http://thread.gmane.org/gmane.comp.lang.r.rcpp/4697).
 *
 * In fact, xts has had an exposed API since 2008, but it wasn't used
 * and as I found out also not quite for two key functions. Jeff kindly gave me SVN access, 
 * and I updated `init.c` (to export) and a new `xtsAPI.h` header (access these).
 *
 * This short post will show how to access this functionality using the new 
 * [RcppXts](http://cran.r-project.org/web/packages/RcppXts/index.html) package.
 *
 * We start by repeating the (updated) `createXts()` function from the
 * [previous post on xts and Rcpp](../creating-xts-from-c++). This
 * helper function (or an improved version of it) should probably go into RcppXts.
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::NumericVector createXts(int sv, int ev) {

    IntegerVector ind = seq(sv, ev);     // values

    NumericVector dv(ind);               // date(time)s are real values
    dv = dv * 86400;                     // scaled to days
    dv.attr("tzone")    = "UTC";         // the index has attributes
    dv.attr("tclass")   = "Date";

    NumericVector xv(ind);               // data her same index
    xv.attr("dim")         = IntegerVector::create(ev-sv+1,1);
    xv.attr("index")       = dv;
    CharacterVector klass  = CharacterVector::create("xts", "zoo");
    xv.attr("class")       = klass;
    xv.attr(".indexCLASS") = "Date";
    xv.attr("tclass")      = "Date";
    xv.attr(".indexTZ")    = "UTC";
    xv.attr("tzone")       = "UTC";
    
    return xv;

}

/*** R
createXts(2,5)
*/

/**
 * Next, we show how to use this. Rcpp attributes will find the xts
 * header file if use a `depends()` attribute, and as the functions we
 * access are registered with the surrounding R process, no linking is
 * needed.
 */

#include <Rcpp.h>

// next two lines connect us to the xts API
#include <xtsAPI.h>
// [[Rcpp::depends(xts)]]

using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::NumericVector rbindXts(NumericMatrix ma, NumericMatrix mb, bool dup=true) {
  NumericMatrix mc = xtsRbind(ma, mb, wrap(dup));
  return mc;
}

/**
 * Thanks to this new (tree-line !!) function, we can combine `xts` object at the source level. 
 */

/*** R
x1 <- createXts(2,5)
x2 <- createXts(4,9)
rbindXts(x1, x2)
rbindXts(x1, x2, FALSE)
*/

/**
 * Notice the difference between the results driven by the third
 * argument about removal of duplicates which has a default value of `TRUE`.
 *
 * While this example was obviously very simple, we can see the power and promise of this. 
 * It derives from being able to work on large numbers of `xts` objects directly at the C++ 
 * level without having to call back to R. So even though 
 * [xts](https://r-forge.r-project.org/projects/xts/) is about as efficient as it gets, we 
 * should be able to make nice gains (for simple enough tasks) by doing them at the C++ level.
 */
