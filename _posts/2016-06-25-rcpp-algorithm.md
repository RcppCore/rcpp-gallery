---
title: Introducing Rcpp&#58;&#58;algorithm
author: Daniel C. Dillon
license: GPL (>= 2)
tags: sugar
summary: An overview of iterator-based algorithms for Rcpp
layout: post
src: 2016-06-25-rcpp-algorithm.Rmd
---

## Introduction

A while back, I saw a post on StackOverflow where the user was trying to use
`Rcpp::sugar::sum()` on an `RcppParallel::RVector`.  Obviously, this does not
work (as Rcpp Sugar pertains to Rcpp types, but not RcppParallel which cannot
rely on `SEXP`-based representation to allow multi-threaded execution). It
raised the question "Why doesn't something more generic exist to 
provide functions with R semantics that can be used on arbitrary data
structures?"  As a result, I set out to create a set of such functions
in `Rcpp::algorithm` which follow the pattern of `std::algorithm`.

## Rcpp::algorithm

Currently `Rcpp::algorithm` contains only a few simple
functions. If these are found to be useful, more will be added.  Examples
of using the currently implemented iterator-based functions are below.

### sum, sum\_nona, prod, and prod\_nona

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
double sum_of_matrix_row(NumericMatrix m, int row) {
    NumericMatrix::Row r = m.row(row);

    return algorithm::sum(r.begin(), r.end());
}
{% endhighlight %}

### min, max, and mean

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
double mean_of_matrix_row(NumericMatrix m, int row) {
    NumericMatrix::Row r = m.row(row);

    return algorithm::mean(r.begin(), r.end());
}
{% endhighlight %}

### log, exp, and sqrt

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector log_of_matrix_row(NumericMatrix m, int row) {
    NumericMatrix::Row r = m.row(row);

    NumericVector retval(m.cols());
    algorithm::log(r.begin(), r.end(), retval.begin());

    return retval;
}
{% endhighlight %}

## Additional Benefits
Through the coding of these simple "algorithms", a few needs arose.

First, the ability to deduce the appropriate `C` numeric type given an `Rcpp`
iterator was necessary.  This gave birth to the
`Rcpp::algorithm::helpers::decays_to_ctype` and
`Rcpp::algorithm::helpers::ctype` type traits.  Given a type, these allow you
to determine whether it can be cast to a `C` numeric type and which type that
would be.

Second, the need arose for more information about `R` types.  This gave birth
to the `Rcpp::algorithm::helpers::rtype` traits.  These are defined as
follows:


{% highlight cpp %}
template< typename T >
struct rtype_helper {};

template<>
struct rtype_helper< double > {
    typedef double type;
    static RCPP_CONSTEXPR int RTYPE = REALSXP;
    static inline double NA() { return NA_REAL; }
    static inline RCPP_CONSTEXPR double ZERO() { return 0.0; }
    static inline RCPP_CONSTEXPR double ONE() { return 1.0; }
};

template<>
struct rtype_helper< int > {
    typedef int type;
    static RCPP_CONSTEXPR int RTYPE = INTSXP;
    static inline int NA() { return NA_INTEGER; }
    static inline RCPP_CONSTEXPR int ZERO() { return 0; }
    static inline RCPP_CONSTEXPR int ONE() { return 1; }
};

template< typename T >
struct rtype {
    typedef typename rtype_helper< typename ctype< T >::type >::type type;
    typedef rtype_helper< typename ctype< T >::type > helper_type;
    static RCPP_CONSTEXPR int RTYPE = helper_type::RTYPE;
    static inline T NA() { return helper_type::NA(); }
    static inline RCPP_CONSTEXPR T ZERO() { return helper_type::ZERO(); }
    static inline RCPP_CONSTEXPR T ONE() { return helper_type::ONE(); }
};
{% endhighlight %}

These additional benefits may actually prove more useful than the algorithms
themselves.  Only time will tell.

## Wrapping Up

There are now some simple iterator-based algorithms that can be used with any
structure that supports iterators.  They apply the same semantics as the
analogous `Rcpp::sugar` functions, but give us more flexibility in their
usage.  If you find these to be useful, feel free to request more.
