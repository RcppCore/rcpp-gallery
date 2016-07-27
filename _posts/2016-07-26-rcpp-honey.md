---
title: RcppHoney Introduction
author: Daniel C. Dillon
license: GPL (>= 2)
tags: sugar
summary: An overview of interoperable iterator-based containers for Rcpp
layout: post
src: 2016-07-26-rcpp-honey.Rmd
---

## Rationale

In `C++` we often have containers that are not compatible with `R` or `Rcpp`
with data already in them (`std::vector`, `std::set`, etc.).  One would like
to be able to operate on these containers without having to copy them into
`Rcpp` structures like `IntegerVector`.  `RcppHoney` aims to address this
problem by providing operators and functions with `R` semantics that can be
used on any iterator-based container.

## Introduction

`RcppHoney` allows any iterator-based container to be "hooked" in.  Once a
container type is hooked to `RcppHoney`, it is granted operators (+, -, \*, /, etc.)
and a host of other mathematical functions that can be run on it.  It also
becomes interoperable with any other hooked data structure.  This lets us
write expressions that look like `std::vector + Rcpp::IntegerVector +
log(Rcpp::NumericVector)` and get the expected results.

## Implementation

`RcppHoney` has several structures that are hooked in by default.  Currently
they are `std::vector`, `std::set`, and `Rcpp::VectorBase`.  The ability to
hook in custom structures is also provided.

All operators and functions are implemented as
[expression templates](https://en.wikipedia.org/wiki/Expression_templates)
to minimize memory usage and enhance performance.  The goal here is to only
copy the data into an `R` compatible structure when we must (i.e. when we
return it to `R`).  This is achieved through the use of the `RcppHoney::operand`
class.  `RcppHoney::operand` provides an iterable interface to the result
types of operators and functions.

`RcppHoney` currently provides all the basic mathematical operators (+, -, \*, /)
as well as some common
[functions](https://github.com/dcdillon/RcppHoney/blob/master/inst/include/RcppHoney/functions.hpp#L53-L112)
(abs, sin, cos, exp, etc.).  Eventually all of the functionality provided by
`Rcpp::sugar` as well as anything else we can think of will be supported.

Enough about the abstract though...let's see it in action.

## Example

The following example shows how to hook in a custom data structure
(in this case `std::list`) as well as the types of expressions that can be
created once a data structure is hooked in.


{% highlight cpp %}
// [[Rcpp::depends(RcppHoney)]]

#include <RcppCommon.h>
#include <RcppHoneyForward.hpp> // we have to do this because we're going to hook in a non-default structure
#include <list>

// We have to declare our hooks before we include RcppHoney.hpp
namespace RcppHoney {
namespace hooks {

// Hook in all std::list types (could be more specific)
template< typename T, typename A >
traits::true_type is_hooked(const std::list< T, A > &val);

// Tell RcppHoney that NA has meaning in std::list
template< typename T, typename A >
traits::true_type has_na(const std::list< T, A > &val);

// Tell RcppHoney that it needs to create basic (e.g. std::list + std::list) operators
template< typename T, typename A >
RcppHoney::traits::true_type needs_basic_operators(const std::list< T, A > &val);

// Tell RcppHoney that it needs to create scalar (e.g. std::list + int/double) operators
template< typename T, typename A >
RcppHoney::traits::true_type needs_scalar_operators(const std::list< T, A > &val);

// Tell RcppHoney that this set of types is part of the FAMILY_USER + 1 family.
// This is used in conjunction with needs_basic_operators.  If you have
// needs_basic_operators return RcppHoney::traits::false_type, then only types
// that are not part of the same family will have binary operators created
// between them.
template< typename T, typename A >
RcppHoney::traits::int_constant< FAMILY_USER + 1 > family(const std::list< T, A > &val);

} // namespace hooks
} // namespace RcppHoney

#include <RcppHoney.hpp>

// [[Rcpp::export]]
Rcpp::NumericVector example_manually_hooked() {

    // We manually hooked std::list in to RcppHoney so we'll create one
    std::list< int > l;
    l.push_back(1); l.push_back(2); l.push_back(3); l.push_back(4); l.push_back(5);

    // std::vector is already hooked in to RcppHoney in default_hooks.hpp so we'll
    // create one of those too
    std::vector< int > v(l.begin(), l.end());

    // And for good measure, let's create an Rcpp::NumericVector which is also hooked by default
    Rcpp::NumericVector v2(v.begin(), v.end());

    // Now do some weird operations incorporating std::vector, std::list, Rcpp::NumericVector
    // and some RcppHoney functions and return it.  The return value will be equal to the following
    // R snippet:
    //     v <- 1:5
    //     result <- 42 + v + v + log(v) - v - v + sqrt(v) + -v + 42

    // We can store our result in any of RcppHoney::LogicalVector, RcppHoney::IntegerVector, or
    // RcppHoney::NumericVector and simply return it to R.  These classes inherit from their
    // Rcpp counterparts and add a new constructor.  The only copy of the data, in this case, is when
    // we assign our expression to retval.  Since it is then a "native" R type, returning it is a
    // shallow copy.  Alternatively we could write this as:
    //     return Rcpp::wrap(1 + v + RcppHoney::log(v) - v - 1 + RcppHoney::sqrt(v) + -v2);

    RcppHoney::NumericVector retval
        =  42 + l + v + RcppHoney::log(v) - v - l + RcppHoney::sqrt(v) + -v2 + 42;
    return retval;
}
{% endhighlight %}

## Conclusion

`RcppHoney` is a powerful tool for allowing different container types to interoperate
under `Rcpp`.  It can save development time as well as help the user generate faster
and more readable code.

`RcppHoney` is available via [CRAN](http://cran.r-project.org) though as it is still
in an alpha state and changing rapidly, it is recommended that you install it from
source.  Source code is available at
[github.com/dcdillon/RcppHoney](https://github.com/dcdillon/RcppHoney).
