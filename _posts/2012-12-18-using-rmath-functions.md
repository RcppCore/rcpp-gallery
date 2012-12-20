---
title: Using Functions from Rmath.h
author: Dirk Eddelbuettel
license: MIT
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


We can now use the function to compute the probaility distribution: 

{% highlight r %}
x <- seq(0, 1, length=1e3)
res <- mypnorm(x)
head(res)
{% endhighlight %}



<pre class="output">
[1] 0.5000 0.5004 0.5008 0.5012 0.5016 0.5020
</pre>

