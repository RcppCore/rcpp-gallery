---
title: "Introduction to RcppNT2"
author: "Kevin Ushey"
license: GPL (>= 2)
tags: simd parallel
summary: Introduction to RcppNT2
layout: post
src: 2016-02-01-rcppnt2-introduction.Rmd
---

Modern CPU processors are built with new, extended
instruction sets that optimize for certain operations. A
class of these allow for vectorized operations, called
Single Instruction / Multiple Data
([SIMD](https://en.wikipedia.org/wiki/SIMD)) instructions. 
Although modern compilers will use these instructions when
possible, they are often unable to reason about whether or
not a particular block of code can be executed using SIMD
instructions.

The [Numerical Template Toolbox (**NT<sup>2</sup>**)](https://github.com/jfalcou/nt2)
is a collection of header-only C++ libraries that make it 
possible to explicitly request the use of SIMD instructions 
when possible, while falling back to regular scalar
operations when not. **NT<sup>2</sup>** itself is powered
by [Boost](http://www.boost.org/), alongside two proposed
Boost libraries -- `Boost.Dispatch`, which provides a
mechanism for efficient tag-based dispatch for functions,
and `Boost.SIMD`, which provides a framework for the
implementation of algorithms that take advantage of SIMD
instructions. [RcppNT2](http://rcppcore.github.io/RcppNT2/)
wraps and exposes these libraries for use with `R`.

The primary abstraction that `Boost.SIMD` uses under the 
hood is the `boost::simd::pack<>` data structure. This item 
represents a small, contiguous, pack of integral objects 
(e.g. `double`s), and comes with a host of functions that 
facilitate the use of SIMD operations on those objects when 
possible. Although you don't need to know the details to use
the high-level functionality provided by `Boost.SIMD`, it's
useful for understanding what happens behind the scenes.

Here's a quick example of how we might compute the sum of 
elements in a vector, using **NT<sup>2</sup>**.


{% highlight cpp %}
// [[Rcpp::depends(RcppNT2)]]
#include <RcppNT2.h>
using namespace RcppNT2;

#include <Rcpp.h>
using namespace Rcpp;

// Define a functor -- a C++ class which defines a templated
// 'function call' operator -- to perform the addition of 
// two pieces of data.
struct add_two {
  template <typename T>
  T operator()(const T& lhs, const T& rhs) {
    return lhs + rhs;
  }
};

// [[Rcpp::export]]
double simd_sum(NumericVector x) {
  // Pass the functor to 'simdReduce()'. This is an
  // algorithm provided by RcppNT2, which makes it
  // easy to apply nt2-style functor definitions
  // across a range of data.
  return simdReduce(x.begin(), x.end(), 0.0, add_two());
}
{% endhighlight %}

Behind the scenes, `simdReduce()` takes care of iteration 
over the provided sequence, and ensures that we use optimized SIMD 
instructions over packs of numbers when possible, and scalar
instructions when not. By passing a templated functor, 
`simdReduce()` can automatically choose the correct template
specialization depending on whether it's working with a pack
or not. In other words, two template specializations will be
generated in this case: one with `T = double`, and another
with `T = boost::simd::pack<double>`.

Let's confirm that this produces the correct output, and run
a small benchmark.


{% highlight r %}
# helper function for printing microbenchmark output
printBm <- function(bm) {
  summary <- summary(bm)
  print(summary[, 1:7], row.names = FALSE)
}

# generate some data
data <- rnorm(1024 * 1000)

# verify that it produces the correct sum
all.equal(simd_sum(data), sum(data))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
# compare results
library(microbenchmark)
bm <- microbenchmark(sum(data), simd_sum(data))
printBm(bm)
{% endhighlight %}



<pre class="output">
           expr     min       lq      mean    median       uq      max
      sum(data) 894.451 943.4145 1033.5598 1020.5000 1071.327 1429.533
 simd_sum(data) 280.585 293.6315  316.6797  307.8795  314.429  574.050
</pre>

We get a noticable gain by taking advantage of SIMD 
instructions here. However, it's worth noting that we don't
handle `NA` and `NaN` with the same granularity as `R`.

## Learning More

This article provides just a taste of how RcppNT2 can be used.
If you're interested in learning more, please check out the
[RcppNT2 website](http://rcppcore.github.io/RcppNT2/).
