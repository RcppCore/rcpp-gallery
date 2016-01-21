/**
 * @title Using RcppNT2 to Compute the Sum
 * @author Kevin Ushey
 * @license GPL (>= 2)
 * @tags simd parallel
 * @summary Demonstrates how to compute the sum using RcppNT2.
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
 * ## Computing the Sum
 * 
 * First, let's review how we might use `std::accumulate()`
 * to sum a vector of numbers. We explicitly pass in the
 * `std::plus<double>()` functor, just to make it clear that
 * the `std::accumulate()` algorithm expects a binary
 * functor when accumulating values.
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double vectorSum(NumericVector x) {
   return std::accumulate(x.begin(), x.end(), 0.0, std::plus<double>());
}

/**
 * Now, let's rewrite this to take advantage of RcppNT2.
 * There are two main steps required to take advantage of
 * RcppNT2 at a high level:
 * 
 *   1. Write a functor, with a templated call operator,
 *      with the implementation written in a
 *      '`Boost.SIMD`-aware' way;
 * 
 *   2. Provide the functor as an argument to the
 *      appropriate SIMD algorithm.
 * 
 * Let's follow these steps in implementing our SIMD sum.
 */

// [[Rcpp::depends(RcppNT2)]]
#include <RcppNT2.h>
using namespace RcppNT2;

struct simd_plus {
   template <typename T>
   T operator()(const T& lhs, const T& rhs) {
      return lhs + rhs;
   }
};

// [[Rcpp::export]]
double vectorSumSimd(NumericVector x) {
   return simdReduce(x.begin(), x.end(), 0.0, simd_plus());
}

/**
 * As you can see, it's quite simple to take advantage of
 * `Boost.SIMD`. For very simple operations such as this,
 * RcppNT2 provides a number of pre-defined functors,
 * which can be accessed in the `RcppParallel::functor`
 * namespace. The following is an equivalent way of defining
 * the above function:
 */

// [[Rcpp::export]]
double vectorSumSimdV2(NumericVector x) {
   return simdReduce(x.begin(), x.end(), 0.0, functor::plus());
}

/**
 * Behind the scenes of `simdReduce()`, `Boost.SIMD` will 
 * apply your templated functor to 'packs' of values when
 * appropriate, and scalar values when not. In other words,
 * there are effectively two kinds of template
 * specializations being generated behind the scenes: one
 * with `T = double`, and one with `T =
 * boost::simd::pack<double>`. The use of the packed
 * representation is what allows `Boost.SIMD` to ensure 
 * vectorized instructions are used and generated.
 * `Boost.SIMD` provides a host of functions and operator
 * overloads that ensure that optimized instructions are
 * used when possible over a packed object, while falling 
 * back to 'default' operations for scalar values when not.
 *
 * Now, let's compare the performance of these two
 * implementations.
 */

/*** R
library(microbenchmark)

# helper function for printing microbenchmark output
printBm <- function(bm) {
  summary <- summary(bm)
  print(summary[, 1:7], row.names = FALSE)
}

# allocate a large vector
set.seed(123)
v <- rnorm(1E6)

# ensure they compute the same values
stopifnot(all.equal(vectorSum(v), vectorSumSimd(v)))

# compare performance
bm <- microbenchmark(vectorSum(v), vectorSumSimd(v))
printBm(bm)
*/

/**
 * Perhaps surprisingly, the RcppNT2 solution is much
 * faster -- the gains are similar to what we might have
 * seen when computing the sum in parallel. However, we're
 * still just using a single core; we're just taking
 * advantage of vectorized instructions provided by the CPU.
 * In this particular case, on Intel CPUs, `Boost.SIMD` will
 * ensure that we are using the `addpd` instruction, which
 * is documented in the Intel Software Developer's Manual 
 * [[PDF](http://www.intel.com/Assets/en_US/PDF/manual/253666.pdf)].
 * 
 * Note that, for the naive serial sum, the compiler would
 * likely generate similarly efficient code when the
 * `-ffast-math` optimization flag is set. By default, the
 * compiler is somewhat 'pessimistic' about the set of 
 * optimizations it can perform around floating point
 * arithmetic. This is because it must respect the
 * [IEEE floating point standard](https://en.wikipedia.org/wiki/IEEE_floating_point),
 * and this means respecting the fact that, for example,
 * floating point operations are not assocative:
 */

/*** R
((0.1 + 0.2) + 0.3) - (0.1 + (0.2 + 0.3))
*/

/**
 * Surprisingly, the above computation does not evaluate to
 * zero!
 * 
 * In practice, you're likely safe to take advantage of the
 * `-ffast-math` optimizations, or `Boost.SIMD`, in your own
 * work. However, be sure to test and verify!
 * 
 * ---
 * 
 * This article provides just a taste of how RcppNT2 can be used.
 * If you're interested in learning more, please check out the
 * [RcppNT2 website](http://rcppcore.github.io/RcppNT2/).
 */