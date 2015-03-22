// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Parsing Dates and Times 
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags boost featured
 * @mathjax false
 * @summary We demonstrate a new utility function built around the Boost Date_Time parsing functions.
 *
 * 
 * ## Motivation
 * 
 * R has excellent for dates and times via the built-in `Date` and `POSIXt`
 * classes.  Their usage, however, is not always as straightforward as one
 * would want.  Certain conversions are more cumbersome than we would like: while
 * `as.Date("2015-03-22")`, would it not be nice if `as.Date("20150322")` (a
 * format often used in logfiles) also worked, or for that matter
 * `as.Date(20150322L)` using an integer variable, or even
 * `as.Date("2015-Mar-22")` and `as.Date("2015Mar22")`?
 * 
 * Similarly, many date and time formats suitable for `POSIXct` (the short form)
 * and `POSIXlt` (the long form with accessible components) often require rather too
 * much formatting, and/or defaults. Why for example does
 * `as.POSIXct(as.numeric(Sys.time()), origin="1970-01-01")` require the
 * `origin` argument on the conversion back (from fractional seconds since the
 * epoch) into datetime---when it is not required when creating the
 * double-precision floating point representation of time since the epoch?
 * 
 * But thanks to [Boost](http://www.boost.org) and its excellent
 * [Boost Date_Time](http://www.boost.org/doc/libs/1_57_0/doc/html/date_time.html)
 * library---which we already mentioned in
 * [this post about the BH package](/articles/using-boost-with-bh)--- we can
 * address parsing of dates and times.  It permitted us to write a new function
 * `toPOSIXct()` which now part of the
 * [RcppBDT](http://cran.r-project.org/package=RcppBDT) package (albeit right
 * now just the [GitHub version](https://github.com/eddelbuettel/rcppbdt) but we
 * expect this to migrate to [CRAN](http://cran.r-project.org) "soon" as well).
 * 
 * ## Implementation
 * 
 * We will now discuss the outline of this implementation.  For full details,
 * see
 * [the source file](https://github.com/eddelbuettel/rcppbdt/blob/master/src/toPOSIXct.cpp).
 *
 *
 * ### Headers and Constants
 */

#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <Rcpp.h>

// [[Rcpp::depends(BH)]]

namespace bt = boost::posix_time;

const std::locale formats[] = {    // this shows a subset only, see the source file for full list
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%m-%d %H:%M:%S%f")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%Y/%m/%d %H:%M:%S%f")),

    std::locale(std::locale::classic(), new bt::time_input_facet("%Y-%m-%d")),
    std::locale(std::locale::classic(), new bt::time_input_facet("%b/%d/%Y")),
};
const size_t nformats = sizeof(formats)/sizeof(formats[0]);


/**
 * Note that we show only two datetime formats along with two date
 * formats. The [actual implementation](https://github.com/eddelbuettel/rcppbdt/blob/master/src/toPOSIXct.cpp) has many more.
 *
 * 
 * ### Core Converter
 * 
 * The actual conversion from string to a double (the underlying format in
 * `POSIXct`) is done by the following function.  It loops over all given
 * formats, and returns the computed value after the first match. In case of
 * failure, a floating point `NA` is returned.
 */


double stringToTime(const std::string s) {

    bt::ptime pt, ptbase;

    // loop over formats and try them til one fits
    for (size_t i=0; pt == ptbase && i < nformats; ++i) {
        std::istringstream is(s);
        is.imbue(formats[i]);
        is >> pt;
    }
    
    if (pt == ptbase) {
        return NAN;
    } else { 
        const bt::ptime timet_start(boost::gregorian::date(1970,1,1));
        bt::time_duration diff = pt - timet_start;

        // Define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG to use nanoseconds
        // (and then use diff.total_nanoseconds()/1.0e9;  instead)
        return diff.total_microseconds()/1.0e6;
    }
}

/**
 * 
 * ### Convenience Wrappers
 * 
 * We want to be able to convert from numeric as well as string formats. For
 * this, we write a templated (and vectorised) function which invokes the actual
 * conversion function for each argument.  It also deals (somewhat
 * heuristically) with two corner cases: we want `20150322` be converted from
 * either integer or numeric, but need in the latter case distinguish this value
 * and its rangue from the (much larger) value for seconds since the epoch.
 * That creates a minir ambiguity: we will not be able to convert back for inputs
 * from seconds since the epoch for the first few years since January 1, 1970.
 * But as these are rare _in the timestamp form_ we can accept the trade-off.
 *
 */

template <int RTYPE>
Rcpp::DatetimeVector toPOSIXct_impl(const Rcpp::Vector<RTYPE>& sv) {

    int n = sv.size();
    Rcpp::DatetimeVector pv(n);
    
    for (int i=0; i<n; i++) {
        std::string s = boost::lexical_cast<std::string>(sv[i]);
        //Rcpp::Rcout << sv[i] << " -- " << s << std::endl;

        // Boost Date_Time gets the 'YYYYMMDD' format wrong, even
        // when given as an explicit argument. So we need to test here.
        // While we are at it, may as well test for obviously wrong data.
        int l = s.size();
        if ((l < 8) ||          // impossibly short
            (l == 9)) {         // 8 or 10 works, 9 cannot
            Rcpp::stop("Inadmissable input: %s", s);
        } else if (l == 8) {    // turn YYYYMMDD into YYYY/MM/DD
            s = s.substr(0, 4) + "/" + s.substr(4, 2) + "/" + s.substr(6,2);
        }
        pv[i] = stringToTime(s);
    }
    return pv;
}

/** 
 * ### User-facing Function
 * 
 * Finally, we can look at the user-facing function.  It accepts input in either
 * integer, numeric or character vector form, and then dispatches accordingly to
 * the templated internal function we just discussed.   Other inputs are
 * unsuitable and trigger an error.
 */

// [[Rcpp::export]]
Rcpp::DatetimeVector toPOSIXct(SEXP x) {
    if (Rcpp::is<Rcpp::CharacterVector>(x)) {
        return toPOSIXct_impl<STRSXP>(x);
    } else if (Rcpp::is<Rcpp::IntegerVector>(x)) {
        return toPOSIXct_impl<INTSXP>(x); 
    } else if (Rcpp::is<Rcpp::NumericVector>(x)) {
        // here we have two cases: either we are an int like
        // 200150315 'mistakenly' cast to numeric, or we actually
        // are a proper large numeric (ie as.numeric(Sys.time())
        Rcpp::NumericVector v(x);
        if (v[0] < 21990101) {  // somewhat arbitrary cuttoff
            // actual integer date notation: convert to string and parse
            return toPOSIXct_impl<REALSXP>(x);
        } else {
            // we think it is a numeric time, so treat it as one
            return Rcpp::DatetimeVector(x);
        }
    } else {
        Rcpp::stop("Unsupported Type");
        return R_NilValue;//not reached
    }
}

/** 
 * ## Illustration
 * 
 * A simply illustration follows.   A fuller demonstration is
 * [part of the RcppBDT package](https://github.com/eddelbuettel/rcppbdt/blob/master/demo/toPOSIXct.R).
 * This already shows support for subsecond granularity and a variety of date formats.
 */

/*** R
## parsing character
s <- c("2004-03-21 12:45:33.123456",    # ISO
       "2004/03/21 12:45:33.123456",    # variant
       "20040321",                      # just dates work fine as well
       "Mar/21/2004",                   # US format, also support month abbreviation or full
       "rapunzel")                      # will produce a NA

p <- toPOSIXct(s)

options("digits.secs"=6)                # make sure we see microseconds in output
print(format(p, tz="UTC"))              # format UTC times as UTC (helps for Date types too)
*/

/** 
 * We can also illustrate integer and numeric inputs:
 */

/*** R
## parsing integer types
s <- c(20150315L, 20010101L, 20141231L)
p <- toPOSIXct(s)
print(format(p, tz="UTC"))

## parsing numeric types
s <- c(20150315, 20010101, 20141231)
print(format(p, tz="UTC"))
*/

/**
 * Note that we always forced display using UTC rather local time, the R default.
 */

