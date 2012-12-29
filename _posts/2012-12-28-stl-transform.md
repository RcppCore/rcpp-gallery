
{% highlight cpp %}
// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
{% endhighlight %}

---
title: STL Transform
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl
summary: This example shows how to use the STL's transform function
layout: post
src: 2012-12-28-stl-transform.cpp
---
The STL transform function can be used to pass a single function over
a vector. Here we use a simple function `square()`.



{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

inline double square(double x) { return x*x ; }

// [[Rcpp::export]]
std::vector<double> transformEx(const std::vector<double>& x) {
    std::vector<double> y(x.size());
    std::transform(x.begin(), x.end(), y.begin(), square);
    return y;
}
{% endhighlight %}


{% highlight r %}
  x <- c(1,2,3,4)
  transformEx(x)
{% endhighlight %}



<pre class="output">
[1]  1  4  9 16
</pre>


A second variant combines two input vectors.


{% highlight cpp %}
inline double squaredNorm(double x, double y) { return sqrt(x*x + y*y); }

// [[Rcpp::export]]
NumericVector transformEx2(NumericVector x, NumericVector y) {
    NumericVector z(x.size());
    std::transform(x.begin(), x.end(), y.begin(), z.begin(), squaredNorm);
    return z;
}
{% endhighlight %}


{% highlight r %}
  x <- c(1,2,3,4)
  y <- c(2,2,3,3)
  transformEx2(x,y)
{% endhighlight %}



<pre class="output">
[1] 2.236 2.828 4.243 5.000
</pre>

