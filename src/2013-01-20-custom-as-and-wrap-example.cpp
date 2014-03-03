// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Custom as and wrap converters example
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics boost
 * @summary This post shows how to use custom as and wrap converters 
 *
 * The [RcppBDT package](http://cran.r-project.org/package=RcppBDT) interfaces
 * [Boost.Date_Time](http://www.boost.org/doc/libs/1_52_0/doc/html/date_time.html) with R. Both systems
 * have their own date representations---and this provides a nice
 * example of custom `as<>()` and `wrap()` converters. Here, we show a simplified example.
 *
 * We start with the forward declarations:
 */


#include <RcppCommon.h>

#include <boost/date_time/gregorian/gregorian_types.hpp> 	// Gregorian calendar types, no I/O

namespace Rcpp {

    // 'date' class boost::gregorian::date
    //
    // non-intrusive extension via template specialisation
    template <> boost::gregorian::date as(SEXP dt);
    //
    // non-intrusive extension via template specialisation
    template <> SEXP wrap(const boost::gregorian::date &d);
}

/**
 * Given these forward declarations, we can now define the converters.
 *
 * For `as()`, we first instantiate a date object and use it to obtain
 * the year, month and day fields to create a `boost::gregorian` date.
 *
 * Similarly, for the inverse operation, we construct an Rcpp date
 * from these components.
 */

#include <Rcpp.h>

// define template specialisations for as and wrap
namespace Rcpp {
    template <> boost::gregorian::date as(SEXP dtsexp) {
        Rcpp::Date dt(dtsexp);
        return boost::gregorian::date(dt.getYear(), dt.getMonth(), dt.getDay());
    }

    template <> SEXP wrap(const boost::gregorian::date &d) {
        boost::gregorian::date::ymd_type ymd = d.year_month_day();     // convert to y/m/d struct
        return Rcpp::wrap( Rcpp::Date( ymd.year, ymd.month, ymd.day ));
    }
}

/**
 * With these converters, we can now use a Boost Date_Time function.
 * As a simple example, we use the _compute the first given weekday
 * after a date_ function.
 */

// [[Rcpp::export]]
Rcpp::Date getFirstDayOfWeekAfter(int weekday, SEXP date) {
    boost::gregorian::first_day_of_the_week_after fdaf(weekday);
    boost::gregorian::date dt = Rcpp::as<boost::gregorian::date>(date);
    return Rcpp::wrap( fdaf.get_date(dt) );
}

/**
 * We can use this to, say, find the first Monday after New Year in 2020:
 *
 */

/*** R
getFirstDayOfWeekAfter(1, as.Date("2020-01-01"))
*/


/**
 * We can also write a more concise form, knowing that `compileAttributes()` will insert 
 * the call to `as<>()` as needed. 
 *
 */

// [[Rcpp::export]]
Rcpp::Date getFirstDayOfWeekAfter2(int weekday, boost::gregorian::date dt) {
    boost::gregorian::first_day_of_the_week_after fdaf(weekday);
    return Rcpp::wrap(fdaf.get_date(dt));
}
