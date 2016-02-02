/**
 * @title SIMD Map-Reduction with RcppNT2
 * @author Kevin Ushey
 * @license GPL (>= 2)
 * @tags simd parallel
 * @summary Demonstrates how a class of map-reduction algorithms
 *    can be expressed easily and efficiently using RcppNT2.
 */

/**
 * ## Introduction
 *
 * The [Numerical Template Toolbox (**NT<sup>2</sup>**)](https://github.com/jfalcou/nt2)
 * collection of header-only C++ libraries that make it
 * possible to explicitly request the use of SIMD instructions
 * when possible, while falling back to regular scalar
 * operations when not. **NT<sup>2</sup>** itself is powered
 * by [Boost](http://www.boost.org/), alongside two proposed
 * Boost libraries -- `Boost.Dispatch`, which provides a
 * mechanism for efficient tag-based dispatch for functions,
 * and `Boost.SIMD`, which provides a framework for the
 * implementation of algorithms that take advantage of SIMD
 * instructions.
 * [`RcppNT2`](http://rcppcore.github.io/RcppNT2/) wraps
 * and exposes these libraries for use with `R`.
 *
 * If you haven't already, read the
 * [RcppNT2 introduction article]({{ site.baseurl }}/articles/rcppnt2-introduction/)
 * to get acquainted with the RcppNT2 package.
 */

/**
 * ## Map Reduce
 *
 * [MapReduce](https://en.wikipedia.org/wiki/MapReduce) is
 * the (infamous) buzzword that describes a class of
 * problems that can be solved by splitting an an algorithm
 * into a map (transform) step, and a reduction step. Although
 * this scheme is typically adopted to help solve problems
 * 'at scale' (e.g., with a large number of communicating
 * machines), it is also a useful abstraction for many
 * problems in the SIMD universe.
 *
 * Take, for example, the dot product. This can be expressed
 * in R code simply as:
 *
 * ```r
 * sum(lhs * rhs)
 * ```
 *
 * Here, we 'map' our vectors by multiplying them together
 * element-wise, and 'reduce' the result through summation.
 * Of course, behind the scenes, R is doing a bit more than
 * it has to -- it's computing a new vector, `lhs * rhs`,
 * which is of the same length as `lhs`, and then collapsing
 * (reducing) that vector by adding each element up. It
 * would be great if we could skip that large temporary
 * vector allocation. RcppNT2 provides a function,
 * `simdMapReduce()`, that makes expressing these kinds of
 * problems very easy.
 *
 * To make use of `simdMapReduce()`, you need to write a
 * class that provides a number of templated methods:
 *
 * - `U init()` --- returns the initial (scalar) data state.
 *
 * - `T map(const T&... ts)` --- transforms the values
 *
 * - `T combine(const T& lhs, const T& rhs)` --- describes how results should be combined
 *
 * - `U reduce(const T& t)` --- describes how a SIMD pack should be reduced
 *
 * You'll notice that we play a little fast-and-loose with
 * the terms, but it should still be relatively clear what
 * each method accomplishes. With this infrastructure, the
 * dot product could be implemented like so:
 */

// [[Rcpp::depends(RcppNT2)]]
#include <RcppNT2.h>
using namespace RcppNT2;

template <typename V>
class DotProductMapReducer
{
public:

  // The initial value for our accumulation. In this case, for
  // a 'plus' reduction, we start at 0.
  V init() { return V{}; }

  // 'map' describes the transformation -- here, multiplying
  // two elements together.
  template <typename T>
  T map(const T& lhs, const T& rhs)
  {
    return lhs * rhs;
  }

  // 'combine' describes how 'map'ped values should be combined.
  // Note that the return type matches the input type -- this
  // allows this function to be specialized both for the pack
  // data structure, as well as scalars, when appropriate.
  template <typename T>
  T combine(const T& lhs, const T& rhs)
  {
    return lhs + rhs;
  }

  // 'reduce' is our bridge from SIMD land to scalar land.
  // We collapse our SIMD vector into a scalar value by
  // summing the elements.
  template <typename T>
  auto reduce(const T& t) -> decltype(nt2::sum(t))
  {
    return nt2::sum(t);
  }
};

/**
 * If you can ignore the C++ templates, it should hopefully
 * be fairly clear what's going on here. We transform
 * elements by multiplying them together, and we combine +
 * reduce by adding them up. (Unfortunately, although the
 * `combine()` and `reduce()` functions are effectively
 * doing the same thing, they need to be expressed separately,
 * as the `reduce()` function is effectively our bridge from
 * SIMD land to scalar land).
 *
 * Now, let's show how our map-reducer can be called.
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double simdDot(NumericVector x, NumericVector y)
{
  return variadic::simdMapReduce(DotProductMapReducer<double>(), x, y);
}

/**
 * Let's also export a version that accepts IntegerVector,
 * just to show that our class is generic enough to accept
 * other integral types as well.
 */

// [[Rcpp::export]]
int simdDotInt(IntegerVector x, IntegerVector y)
{
  return variadic::simdMapReduce(DotProductMapReducer<int>(), x, y);
}

/**
 * And let's execute it from R, just to convince ourselves that it works.
 */

/*** R
set.seed(123)

# numeric version
x <- runif(16)
y <- runif(16)
stopifnot(all.equal(sum(x * y), simdDot(x, y)))

# int version
x <- 1:16
y <- 1:16
stopifnot(all.equal(sum(x * y), simdDotInt(x, y)))
*/

/**
 * Great! Of course, a large number of problems can be
 * expressed with a 'plus', or 'sum' reduction, so RcppNT2
 * also provides a helper for that, so that you only need to
 * implement the 'map' step. We can do this by writing a class
 * that inherits from the `PlusReducer` class:
 */

template <typename V>
class DotProductMapReducerV2 : public PlusReducer<V>
{
public:
  template <typename T>
  T map(const T& lhs, const T& rhs)
  {
    return lhs * rhs;
  }
};

// [[Rcpp::export]]
double simdDotV2(NumericVector x, NumericVector y)
{
  return variadic::simdMapReduce(DotProductMapReducerV2<double>(), x, y);
}

/**
 * And, let's convince ourselves it works:
 */

/*** R
stopifnot(all.equal(simdDot(x, y), simdDotV2(x, y)))
*/

/**
 * And let's use a quick microbenchmark to see if we've truly
 * gained anything here:
 */

/*** R
# benchmarking the numeric case
n1 <- runif(1E6)
n2 <- runif(1E6)

library(rbenchmark)
benchmark(sum(n1 * n2),
          n1 %*% n2,
          simdDot(n1, n2),
          simdDotV2(n1, n2))[, 1:4]

# benchmarking the integer case
i1 <- rpois(1E6, 1)
i2 <- rpois(1E6, 1)
benchmark(sum(i1 * i2),
          i1 %*% i2,
          simdDotInt(i1, i2))[, 1:4]
*/

/**
 * You might be surprised how profound the speed
 * improvements accrued by using SIMD instructions are. How
 * does this happen?
 *
 * Behind the scenes, `simdMapReduce()` is handling a number
 * of things for us:
 *
 * 1. Iteration over the sequences used SIMD packs when
 *    possible, and scalars when not,
 *
 * 2. Optimized SIMD instructions are used to transform and
 *    combine packs of values,
 *
 * 3. Intermediate results are held in a SIMD register, rather
 *    than materializing a whole vector,
 *
 * 4. The SIMD register and scalar buffer are not combined
 *    until the very final step.
 *
 * In the 'double' case, we can pack 2 values into a SIMD
 * pack; in the 'int' case, we can pack 4 values (assuming
 * 32bit 'int' and 128bit SSE registers, which is the common
 * case on Intel processors at the time of this post).
 * Assuming that it takes the number of clock cycles to
 * execute a SIMD instruction as it does for the scalar
 * equivalent, this should translate into ~2x and ~4x
 * speedups -- and that's not even accounting for gains in
 * efficient register use, cache efficiency, and the ability
 * to avoid the large temporary allocation! That said, we
 * are playing it a little fast and loose in the 'int' case:
 * with larger numbers, we could easily overflow; depending
 * on the type of data expected it may be more appropriate
 * to accumulate values into a different data type.
 *
 * In short -- if you're implementing an algorithm, or part
 * of an algorithm, that can be expressed as:
 *
 * - `sum(<transformation of variables>)`
 *
 * then `simdMapReduce()` is worth looking at.
 *
 * ---
 *
 * This article provides just a taste of how RcppNT2 can be used.
 * If you're interested in learning more, please check out the
 * [RcppNT2 website](http://rcppcore.github.io/RcppNT2/).
 */
