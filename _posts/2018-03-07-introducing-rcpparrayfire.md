---
title: "Introducing RcppArrayFire"
author: "Ralf Stubner"
license: GPL (>= 2)
mathjax: true
tags: basics gpu
summary: "Introduces RcppArrayFire, an interface to the ArrayFire library."
layout: post
src: 2018-03-02-introducing-rcpparrayfire.Rmd
---

### Introduction

The [RcppArrayFire package](http://www.daqana.org/rcpparrayfire/) provides an interface
from R to and from the [ArrayFire library](http://www.arrayfire.com/), an open source
library that can make use of GPUs and other hardware accelerators via CUDA or OpenCL.

The [official R bindings](https://github.com/arrayfire/arrayfire-r/) expose ArrayFire
data structures as S4 objects in R, which would require a large amount of code to
support all the methods defined in ArrayFire's C/C++ API. RcppArrayFire instead, 
which is derived from [RcppFire](https://github.com/kafku/RcppFire) by Kazuki Fukui,
follows the lead of packages like [RcppArmadillo](http://dirk.eddelbuettel.com/code/rcpp.armadillo.html)
or [RcppEigen](http://dirk.eddelbuettel.com/code/rcpp.eigen.html) to provide seamless
communication between R and ArrayFire at the C++ level.

### Installation

Please note that RcppArrayFire is developed and tested on Linux systems. There is
preliminary support for [Mac OS X](https://github.com/daqana/rcpparrayfire/tree/macos).

In order to use RcppArrayFire you will need the ArrayFire library and header files. 
While ArrayFire has been packaged for Debian, I currently prefer using upstream's
[binary installer](http://arrayfire.org/docs/installing.htm) or 
[building from source](https://github.com/arrayfire/arrayfire).

RcppArrayFire is not on CRAN, but you can install the current version via 
[drat](https://cran.r-project.org/package=drat):


{% highlight r %}
#install.packages("drat")
drat::addRepo("daqana")
install.packages("RcppArrayFire")
{% endhighlight %}

If you have installed ArrayFire in a non-standard directory, you have to use the
configure argument `--with-arrayfire`, e.g.:


{% highlight r %}
install.packages("RcppArrayFire", configure.args = "--with-arrayfire=/opt/arrayfire-3")
{% endhighlight %}

### A first example

Let's look at the classical example of [calculating $$\pi$$](http://gallery.rcpp.org/articles/simulating-pi/)
via simulation. The basic idea is to generate a large number of random points within
the unit square. An approximation for $$\pi$$ can then be calculated from the ratio
of points within the unit circle to the total number of points. A vectorized 
implementation in R might look like this:


{% highlight r %}
piR <- function(N) {
    x <- runif(N)
    y <- runif(N)
    4 * sum(sqrt(x^2 + y^2) < 1.0) / N
}

set.seed(42)
system.time(cat("pi ~= ", piR(10^6), "\n"))
{% endhighlight %}



<pre class="output">
pi ~=  3.13999 
</pre>



<pre class="output">
   user  system elapsed 
  0.102   0.009   0.111 
</pre>

A simple way to use C++ code in R is to use the inline package or `cppFunction()`
from Rcpp, which are both possible with RcppArrayFire. An implementation in C++
using ArrayFire might look like this:


{% highlight r %}
src <- '
double piAF (const int N) {
    array x = randu(N, f32);
    array y = randu(N, f32);
    return 4.0 * sum<float>(sqrt(x*x + y*y) < 1.0) / N;
}'
Rcpp::cppFunction(code = src, depends = "RcppArrayFire", includes = "using namespace af;")

RcppArrayFire::arrayfire_set_seed(42)
cat("pi ~= ", piAF(10^6), "\n") # also used for warm-up 
{% endhighlight %}



<pre class="output">
pi ~=  3.14279 
</pre>



{% highlight r %}
system.time(piAF(10^6))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.000   0.001   0.000 
</pre>

Several things are worth noting:

1. The syntax is almost identical. Besides the need for using types and a
different function name when generating random numbers, the argument `f32` to
`randu` as well as the `float` type catches the eye. These instruct ArrayFire to
use single precision floats, since not all devices support double precision
floating point numbers. If you want and can to use double precision, you have to
specify `f64` and `double`.

1. The results are not the same since ArrayFire uses a different random number
generator.

1. The speed-up can be quite impressive. However, the first invocation of
a function is often not as fast as expected due to the  [just-in-time compilation](http://arrayfire.com/performance-of-arrayfire-jit-code-generation/)
used by ArrayFire. This can be circumvented by using a warm-up call with (normally)
fewer computations.

### Arrays as parameters 

Up to now we have only considered simple types like `double` or `int` as function
parameters and return values.  However, we can also use arrays. Consider the case
of an European put option that was recently handled with [R, Rcpp and RcppArmadillo](http://gallery.rcpp.org/articles/black-scholes-three-ways/).
The Armadillo based function from this post reads:


{% highlight cpp %}
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::plugins(cpp11)]]

using arma::colvec;
using arma::log;
using arma::normcdf;
using std::sqrt;
using std::exp;


// [[Rcpp::export]]
colvec put_option_pricer_arma(colvec s, double k, double r, double y, double t, double sigma) {
  
  colvec d1 = (log(s / k) + (r - y + sigma * sigma / 2.0) * t) / (sigma * sqrt(t));
  colvec d2 = d1 - sigma * sqrt(t);
  
  // Notice the use of % to represent element wise multiplication
  colvec V = normcdf(-d2) * k * exp(-r * t) - s * exp(-y * t) % normcdf(-d1); 
  
  return V;
}
{% endhighlight %}

This function can be applied to a range of spot prices:


{% highlight r %}
put_option_pricer_arma(s = 55:60, 60, .01, .02, 1, .05)
{% endhighlight %}



<pre class="output">
        [,1]
[1,] 5.52021
[2,] 4.58142
[3,] 3.68485
[4,] 2.85517
[5,] 2.11883
[6,] 1.49793
</pre>

Porting this code to RcppArrayFire is straight forward:


{% highlight cpp %}
#include <RcppArrayFire.h>

// [[Rcpp::depends(RcppArrayFire)]]

using af::array;
using af::log;
using af::erfc;
using std::sqrt;
using std::exp;

array normcdf(array x) {
  return erfc(-x / sqrt(2.0)) / 2.0;
}

// [[Rcpp::export]]
array put_option_pricer_af(RcppArrayFire::typed_array<f32> s, double k, double r,
                           double y, double t, double sigma) {
  
  array d1 = (log(s / k) + (r - y + sigma * sigma / 2.0) * t) / (sigma * sqrt(t));
  array d2 = d1 - sigma * sqrt(t);
  
  return normcdf(-d2) * k * exp(-r * t) - s * exp(-y * t) * normcdf(-d1); 
}
{% endhighlight %}

Compared with the implementations in [R, Rcpp and RcppArmadillo](http://gallery.rcpp.org/articles/black-scholes-three-ways/) 
the syntax is again almost the same. One exception is that ArrayFire does not contain
a function for the cumulative normal distribution function. However, the [closely related error function](https://en.wikipedia.org/wiki/Error_function#Cumulative_distribution_function)
is available.

Since an object of type `af::array` can contain different [data types](http://arrayfire.org/docs/defines_8h.htm#a023d8ac325fb14f1712a52fb0940b1d5),
the templated wrapper class `RcppArrayFire::typed_array<>` is used to indicate the
desired data type when converting from R to C++. Again single precision floats are
used with ArrayFire, which leads to differences of the order $$10^{-6}$$ compared to the 
results from [R, Rcpp and RcppArmadillo](http://gallery.rcpp.org/articles/black-scholes-three-ways/):


{% highlight r %}
put_option_pricer_af(s = 55:60, 60, .01, .02, 1, .05)
{% endhighlight %}



<pre class="output">
[1] 5.52021 4.58143 3.68485 2.85516 2.11883 1.49793
</pre>

### Performance

The reason to use hardware accelerators is of course the quest for increased performance.
How does ArrayFire fare in this respect? Using the same
benchmark as in the [R, Rcpp and RcppArmadillo](http://gallery.rcpp.org/articles/black-scholes-three-ways/)
comparison:


{% highlight r %}
s <- matrix(seq(0, 100, by = .0001), ncol = 1)
rbenchmark::benchmark(Arma = put_option_pricer_arma(s, 60, .01, .02, 1, .05),
                      AF = put_option_pricer_af(s, 60, .01, .02, 1, .05),
                      order = "relative", 
                      replications = 100)[,1:4]
{% endhighlight %}



<pre class="output">
  test replications elapsed relative
2   AF          100   0.471    1.000
1 Arma          100   5.923   12.575
</pre>

Here a Nvidia GeForce GT 1030 is used together with ArrayFire's CUDA backend. With
a build-in Intel HD Graphics 520 using the OpenCL backend the ArrayFire solution
is about 6 times faster. Even without a high performance GPU the performance boost
from using ArrayFire can be quite impressive. However, the results change dramatically,
if fewer options are evaluated:


{% highlight r %}
s <- matrix(seq(0, 100, by = 1), ncol = 1)
# use more replications to get run times of more than 10 ms
rbenchmark::benchmark(Arma = put_option_pricer_arma(s, 60, .01, .02, 1, .05),
                      AF = put_option_pricer_af(s, 60, .01, .02, 1, .05),
                      order = "relative", 
                      replications = 1000)[,1:4]
{% endhighlight %}



<pre class="output">
  test replications elapsed relative
1 Arma         1000   0.008    1.000
2   AF         1000   0.123   15.375
</pre>

But is it realistic to process $$10^6$$ options at once? Probably not in the way used
in the benchmark where only the spot price is allowed to vary. However, one can alter
the function to process not only arrays of spot prices but also arrays of strikes,
risk free rates etc.:


{% highlight cpp %}
#include <RcppArrayFire.h>

// [[Rcpp::depends(RcppArrayFire)]]

using af::array;
using af::log;
using af::erfc;
using af::sqrt; // arrayfire function instead of standard function
using af::exp;  // arrayfire function instead of standard function

array normcdf(array x) {
  return erfc(-x / sqrt(2.0)) / 2.0;
}

// [[Rcpp::export]]
array put_option_pricer_af(RcppArrayFire::typed_array<f32> s,
                           RcppArrayFire::typed_array<f32> k, 
                           RcppArrayFire::typed_array<f32> r, 
                           RcppArrayFire::typed_array<f32> y, 
                           RcppArrayFire::typed_array<f32> t, 
                           RcppArrayFire::typed_array<f32> sigma) {
  
  array d1 = (log(s / k) + (r - y + sigma * sigma / 2.0) * t) / (sigma * sqrt(t));
  array d2 = d1 - sigma * sqrt(t);
  
  return normcdf(-d2) * k * exp(-r * t) - s * exp(-y * t) * normcdf(-d1); 
}
{% endhighlight %}

Note that ArrayFire does not recycle elements if arrays with non-matching dimensions
are combined. In this particular case this means that all arrays must have the same
length. One can ensure that by using a data frame for the values:


{% highlight r %}
set.seed(42)
# 1000 * 21 * 3 * 3 * 3 * 3 = 1,701,000 different options
options <- expand.grid(
  s = rnorm(1000, mean = 60, sd = 20),
  k = 50:70,
  r = c(0.01, 0.005, 0.02),
  y = c(0.02, 0.01, 0.04),
  t = c(1, 0.5, 2),
  sigma = c(0.05, 0.025, 0.1)
)
head(within(options,
            p <- put_option_pricer_af(s, k, r, y, t, sigma)))
{% endhighlight %}



<pre class="output">
        s  k    r    y t sigma           p
1 87.4192 50 0.01 0.02 1  0.05 7.44673e-29
2 48.7060 50 0.01 0.02 1  0.05 2.09401e+00
3 67.2626 50 0.01 0.02 1  0.05 2.34556e-09
4 72.6573 50 0.01 0.02 1  0.05 6.84457e-14
5 68.0854 50 0.01 0.02 1  0.05 5.26653e-10
6 57.8775 50 0.01 0.02 1  0.05 2.57750e-03
</pre>

### Conclusion

The ArrayFire library provides a convenient way to use hardware accelerators without
the need to write low-level OpenCL or CUDA code. The C++ syntax is actually quite
similar to properly vectorized R code. The RcppArrayFire package makes this available
to useRs. However, one still has to be careful: using hardware accelerators is not
a "silver bullet" due to the inherent memory transfer overhead.
