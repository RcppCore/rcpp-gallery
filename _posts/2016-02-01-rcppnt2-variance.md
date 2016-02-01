---
title: Using RcppNT2 to Compute the Variance
author: Kevin Ushey
license: GPL (>= 2)
tags: simd parallel
summary: Demonstrates how to compute the variance using RcppNT2.
layout: post
src: 2016-02-01-rcppnt2-variance.cpp
---


## Introduction

The [Numerical Template Toolbox (**NT<sup>2</sup>**)](https://github.com/jfalcou/nt2)
collection of header-only C++ libraries that make it
possible to explicitly request the use of SIMD instructions
when possible, while falling back to regular scalar
operations when not. **NT<sup>2</sup>** itself is powered
by [Boost](http://www.boost.org/), alongside two proposed
Boost libraries -- `Boost.Dispatch`, which provides a
mechanism for efficient tag-based dispatch for functions,
and `Boost.SIMD`, which provides a framework for the
implementation of algorithms that take advantage of SIMD
instructions.
[`RcppNT2`](http://rcppcore.github.io/RcppNT2/) wraps
and exposes these libraries for use with `R`.

If you haven't already, read the
[RcppNT2 introduction article]({{ site.baseurl }}/articles/rcppnt2-introduction/)
to get acquainted with the RcppNT2 package.

## Computing the Variance

As you may or may not know, there are a number of algorithms
for [computing the variance](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance),
each making different tradeoffs in algorithmic complexity
versus numerical stability. We'll focus on implementing a
two-pass algorithm, whereby we compute the mean in a first
pass, and later the sum of squares in a second pass.

First, let's look at the R code one might write
to compute the variance.

{% highlight r %}
set.seed(123)
x <- rnorm(16)
myVar <- function(x) {
  sum((x - mean(x))^2) / (length(x) - 1)
}
stopifnot(all.equal(var(x), myVar(x)))
c(var(x), myVar(x))
{% endhighlight %}



<pre class="output">
[1] 0.833939 0.833939
</pre>

We can decompose the operation into a few distinct steps:

1. Compute the mean for our vector 'x',
2. Compute the squared deviations from the mean,
3. Sum the deviations about the mean,
4. Divide the summation by the length minus one.

Naively, we could imagine writing a 'simdTransform()' for 
step 2, and an 'simdReduce()' for step 3. However, this 
is a bit inefficient as the transform would require 
allocating a whole new vector, with the same length as 
our initial vector. When neither 'simdTransform()' nor
'simdReduce()' seem to be a good fit, we can fall back to
'simdFor()'. We can pass a stateful functor to handle
accumulation of the transformed results.

Let's write a class that encapsulates this 'sum of
squares' operation.

{% highlight cpp %}
// [[Rcpp::depends(RcppNT2)]]
#include <RcppNT2.h>
using namespace RcppNT2;

#include <Rcpp.h>
using namespace Rcpp;

class SumOfSquaresAccumulator
{
public:
   
   // Since the 'transform()' operation requires knowledge
   // of the mean, we ensure our class must be constructed
   // with a mean value. We will also hold the final result
   // within the 'result_' variable, and a SIMD 'pack_' to
   // hold the results for SIMD computations.
   explicit SumOfSquaresAccumulator(double mean)
      : mean_(mean), result_(0.0), pack_(0.0)
   {}
   
   // We need to provide two call operators: one to handle
   // SIMD packs, and one to handle scalar values. We do this
   // as we want to accumulate SIMD results in a SIMD data
   // structure, and scalar results in a scalar data object.
   //
   // We _could_ just accumulate all our results in a single
   // 'double', but this is actually less efficient -- it
   // pays to use packed structures when possible.
   void operator()(double data)
   {
      result_ += nt2::sqr(data - mean_);
   }
   
   void operator()(const boost::simd::pack<double>& data)
   {
      pack_ += nt2::sqr(data - mean_);
   }
   
   // We can use 'value()' to extract the result of the 
   // computation, after providing this object to
   // 'simdFor()'.
   double value() const
   {
      return result_ + boost::simd::sum(pack_);
   }
   
private:
   double mean_;
   double result_;
   boost::simd::pack<double> pack_;
};
{% endhighlight %}

Now that we have our accumulator class defined, we can
use it to compute the variance. We'll call our function
'simdVar()', and export it using Rcpp attributes in the
usual way.

{% highlight cpp %}
// [[Rcpp::export]]
double simdVar(NumericVector data)
{
   using namespace RcppNT2;
   
   // First, compute the mean as we'll need that for
   // computation of the sum of squares.
   double total = simdReduce(data.begin(), data.end(), 0.0, functor::plus());
   
   std::size_t n = data.size();
   double mean = total / n;
   
   // Use our accumulator to compute the sum of squares.
   SumOfSquaresAccumulator accumulator(mean);
   simdFor(data.begin(), data.end(), accumulator);
   double ssq = accumulator.value();
   
   // Divide by 'n - 1', and we're done!
   return ssq / (n - 1);
}
{% endhighlight %}

Let's confirm that this works as expected...

{% highlight r %}
x <- as.numeric(1:10)
c(var(x), simdVar(x))
{% endhighlight %}



<pre class="output">
[1] 9.166667 9.166667
</pre>



{% highlight r %}
stopifnot(all.equal(var(x), simdVar(x)))
{% endhighlight %}

And let's benchmark, to see how performance compares.

{% highlight r %}
library(microbenchmark)

# A helper function for printing microbenchmark output
printBm <- function(bm) {
  summary <- summary(bm)
  print(summary[, 1:7], row.names = FALSE)
}

small <- as.numeric(1:16)
stopifnot(all.equal(var(small), simdVar(small)))
bm <- microbenchmark(var(small), simdVar(small))
printBm(bm)
{% endhighlight %}



<pre class="output">
           expr    min      lq     mean median      uq    max
     var(small) 11.784 14.3395 16.37862 15.096 15.7225 40.346
 simdVar(small)  1.506  1.7045  2.06541  1.947  2.1055 10.935
</pre>



{% highlight r %}
large <- rnorm(1E6)
stopifnot(all.equal(var(large), simdVar(large)))
bm <- microbenchmark(var(large), simdVar(large))
printBm(bm)
{% endhighlight %}



<pre class="output">
           expr      min       lq      mean    median       uq      max
     var(large) 3046.597 3194.231 3278.7417 3301.6205 3323.581 3809.090
 simdVar(large)  579.784  594.887  607.0411  607.9845  614.386  712.038
</pre>

As we can see, taking advantage of SIMD instructions
has improved the runtime quite drastically.

However, we should note that this is not an entirely fair
comparison with `R`s implementation of the variance. In
particular, we do not have a nice mechanism for handling 
missing values; if your data does have any `NA` or `NaN`
values, they will simply be propagated (and not
necessarily in the same way that `R` propagates
missingness). If you're interested, a separate example
is provided as part of the RcppNT2 package, and you can
browse the standalone source file
[here](https://github.com/RcppCore/RcppNT2/blob/master/inst/examples/example-na-handling-variance.cpp).

---

This article provides just a taste of how RcppNT2 can be used.
If you're interested in learning more, please check out the
[RcppNT2 website](http://rcppcore.github.io/RcppNT2/).
