---
title: Robust Estimators of Location and Scale
author: Ross Bennett
license: GPL (>= 2)
tags: stl benchmark
summary: Functions to compute median and median absolute deviation
layout: post
src: 2013-01-20-robust-estimators.cpp
---



First, the `median_Rcpp` function is defined to compute the median of the
given input vector. It is assumed that the input vector is unsorted, so a 
copy of the input vector is made using `clone` and then [`std::nth_element`](http://en.cppreference.com/w/cpp/algorithm/nth_element) 
is used to access the `nth` sorted element. Since we only care about
accessing one sorted element of the vector for an odd length vector and two
sorted elements for an even length vector, it is faster to use
`std::nth_element` than either `std::sort` or `std::partial_sort`.

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double median_rcpp(NumericVector x) {
   NumericVector y = clone(x);
   int n, half;
   double y1, y2;
   n = y.size();
   half = n / 2;
   if(n % 2 == 1) {
      // median for odd length vector
      std::nth_element(y.begin(), y.begin()+half, y.end());
      return y[half];
   } else {
      // median for even length vector
      std::nth_element(y.begin(), y.begin()+half, y.end());
      y1 = y[half];
      std::nth_element(y.begin(), y.begin()+half-1, y.begin()+half);
      y2 = y[half-1];
      return (y1 + y2) / 2.0;
   }
}
{% endhighlight %}


{% highlight r %}
library(rbenchmark)
set.seed(123)
z <- rnorm(1000000)

median_rcpp(1:10)
{% endhighlight %}



<pre class="output">
[1] 5.5
</pre>



{% highlight r %}
median_rcpp(1:9)
{% endhighlight %}



<pre class="output">
[1] 5
</pre>



{% highlight r %}

# benchmark median_rcpp and median
benchmark(median_rcpp(z), median(z), order="relative")[,1:4]
{% endhighlight %}



<pre class="output">
            test replications elapsed relative
1 median_rcpp(z)          100   1.747    1.000
2      median(z)          100   5.991    3.429
</pre>


Next, the `mad_rcpp` function is defined to compute the median absolute 
deviation. This is a simple one-liner thanks to the sugar function `abs`, 
the vectorized operators, and the `median_rcpp` function defined above. 
Note that a default value is specified for the scale_factor argument.

{% highlight cpp %}
// [[Rcpp::export]]
double mad_rcpp(NumericVector x, double scale_factor = 1.4826) {
   // scale_factor = 1.4826; default for normal distribution consistent with R
   return median_rcpp(abs(x - median_rcpp(x))) * scale_factor;
}
{% endhighlight %}


{% highlight r %}
# benchmark mad_rcpp and mad
benchmark(mad_rcpp(z), mad(z), order="relative")[,1:4]
{% endhighlight %}



<pre class="output">
         test replications elapsed relative
1 mad_rcpp(z)          100   3.678    1.000
2      mad(z)          100  13.443    3.655
</pre>

