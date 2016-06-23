---
title: Using Functions from Rmath.h
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: rmath featured
summary: Illustrates how to access statistical distribution functions from
  the standalone R math library (Rmath.h)
layout: post
src: 2012-12-18-using-rmath-functions.cpp
---
R, as a statistical language and environment, has very well written and
tested statistical distribution functions providing probability density,
cumulative distribution, quantiles and random number draws for dozens of
common and not so common distribution functions. This code is used inside
R, and available for use from standalone C or C++ programs via the
standalone R math library which Debian / Ubuntu have as a package `r-mathlib`
(and which can be built from R sources).

The short example below shows this for a simple function taking a vector, 
and returning its pnorm computed using the R `pnorm` function:


{% highlight cpp %}
#include <Rcpp.h>

// [[Rcpp::export]]
Rcpp::NumericVector mypnorm(Rcpp::NumericVector x) {
    
   int n = x.size();
   Rcpp::NumericVector y(n);

   for (int i=0; i<n; i++) 
      y[i] = R::pnorm(x[i], 0.0, 1.0, 1, 0);

   return y;
}
{% endhighlight %}

We can now use the function to compute the probability distribution: 

{% highlight r %}
x <- seq(0, 1, length=1e3)
res <- mypnorm(x)
head(res)
{% endhighlight %}



<pre class="output">
[1] 0.5000000 0.5003993 0.5007987 0.5011980 0.5015974 0.5019967
</pre>

Note that the C++ code, through its use via Rcpp attributes, will be augmented with 
an automatic instantiation of a `RNGScope` object which ensures a proper state
of the R random number generator. 
