---
title: Using the Rcpp sugar function clamp
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: sugar benchmark featured
summary: This post illustrates the sugar function clamp
layout: post
src: 2013-01-07-sugar-function-clamp.cpp
---
Since the 0.10.* release series, Rcpp contains a new sugar function `clamp`
which can be used to limit vectors to both a minimum and maximim value.
[This recent StackOverflow question]() permitted `clamp` to
shine. We retake some of the answers, including the `clamp` entry
by Romain.

We first define the three R versions.



{% highlight r %}
pminpmaxClamp <- function(x, a, b) {
    pmax(a, pmin(x, b) )
}

ifelseClamp <- function(x, a, b) {
    ifelse(x <= a,  a, ifelse(x >= b, b, x))
}

operationsClamp <- function(x, a, b) {
    a + (x-a > 0)*(x-a) - (x-b > 0)*(x-b)
}
{% endhighlight %}


We then define some data, and ensure that these versions all
producing identical results.

{% highlight r %}
set.seed(42)
x <- rnorm(100000)

a <- -1.0
b <- 1.0
stopifnot(all.equal(pminpmaxClamp(x,a,b), ifelseClamp(x,a,b), operationsClamp(x,a,b)))
{% endhighlight %}


Next is the C++ solution: a one-liner thanks to the existing sugar function.

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rcppClamp(NumericVector x, double mi, double ma) {
    return clamp(mi, x, ma);
}
{% endhighlight %}


We can then check and benchmark the new C++ version.

{% highlight r %}
stopifnot(all.equal(pminpmaxClamp(x,a,b), rcppClamp(x,a,b)))

library(rbenchmark)
benchmark(pminpmaxClamp(x, a, b), 
          ifelseClamp(x, a, b), 
          operationsClamp(x, a, b),
          rcppClamp(x, a, b),
          order="relative")[,1:4]
{% endhighlight %}



<pre class="output">
                      test replications elapsed relative
4       rcppClamp(x, a, b)          100   0.134    1.000
3 operationsClamp(x, a, b)          100   0.509    3.799
1   pminpmaxClamp(x, a, b)          100   0.532    3.970
2     ifelseClamp(x, a, b)          100   5.266   39.299
</pre>


We see a decent gain of the Rcpp version even relative to these
vectorised R solutions. Among these, the simplest (based on
`ifelse`) is by far the slowest.  The parallel min/max version is
about as faster as the clever-but-less-readable expression-based
solution.

Real "production" solutions will of course need some more testing
of inputs etc.  However, as an illustration of `clamp` this example
has hopefully been compelling.
