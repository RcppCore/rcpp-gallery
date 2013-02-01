// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using Boost via the new BH package
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags boost basics 
 * @summary This post shows how to use Boost via the new BH package
 *
 * Earlier today the new [BH](http://cran.r-project.org/package=BH) package arrived on [CRAN](http://cran.r-project.org). 
 * Over the years, [Jay Emerson](http://www.stat.yale.edu/~jay/), [Michael Kane](http://sites.google.com/site/kaneplusplus/) 
 * and I had numerous discussions about a basic [Boost](http://www.boost.org) infrastructure package providing Boost headers 
 * for other CRAN packages. JJ and Romain chipped in as well, and Jay finally took the lead by first creating a 
 * [repo on R-Forge](http://boostheaders.r-forge.r-project.org/). And now the package is out, so what follows is a little demo.
 *
 * This example borrows something already implemented in my [RcppBDT]((http://cran.r-project.org/package=RcppBDT) package which wraps
 * code from [Boost Date_Time](http://www.boost.org/doc/libs/1_52_0/doc/html/date_time.html) for R.  
 * Here, we compute the so-called [IMM Date](http://en.wikipedia.org/wiki/IMM_dates) -- generally the
 * the third Wednesday of the month (in the last month of the quarter).  Boost has a function computing the _Nth day of the Mth week_ 
 * for a given month in a given year: we use that here with _Wednesday_ and the _third_ week.
 *
 * The kicker is that Boost uses templates almost exclusively. So by declaring an _depends attribute_ on BH, we ensure that 
 * the compilation will see the headers files provided by BH.  Which happen to be the Boost headers, as that is what the package does. 
 * And that is all it takes.
 */

// Use brandnew CRAN package BH for Boost headers

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>

// One include file from Boost
#include <boost/date_time/gregorian/gregorian_types.hpp>

using namespace boost::gregorian;

// [[Rcpp::export]]
Rcpp::Date getIMMDate(int mon, int year) {
    // compute third Wednesday of given month / year
    date d = nth_day_of_the_week_in_month(nth_day_of_the_week_in_month::third,
                                          Wednesday, mon).get_date(year);
    date::ymd_type ymd = d.year_month_day();
    return Rcpp::wrap(Rcpp::Date(ymd.year, ymd.month, ymd.day));
}

/** 
 * We can test this from R for 2013 by computing the first two:
 */

/*** R
getIMMDate(3, 2013)
getIMMDate(6, 2013)
*/

/** 
 * And for kicks, the same for 2033:
 */

/*** R
getIMMDate(3, 2033)
getIMMDate(6, 2033)
*/

/**
 * The BH package is still pretty raw. For example, [yesterday's Rcpp
 * Gallery post on Boost foreach](../boost-foreach)] does not build as
 * we have not yet included the relevant Boost library. So far, BH reflects the needs of 
 * Jay and Mike in their (awesome) [bigmemory](http://www.bigmemory.org) project
 * and my needs in RcppBDT, and then some.  For the rest, let us know what may be missing.
 */
