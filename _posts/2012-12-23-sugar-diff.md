---
title: Using Sugar Function diff()
author: Ross Bennett
license: GPL (>= 2)
tags: sugar
summary: Illustrates the use of sugar function diff()
layout: post
src: 2012-12-23-sugar-diff.cpp
---



The sugar function diff() computes the difference of consecutive elements
(i.e. lag = 1) of the input vector. Note that the size of the vector returned
is one less than the input vector. The sugar function diff() works the same
way as the diff() function in base R, except the lag is not specified.

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector diff_sug(NumericVector x){
   return diff(x);
}
{% endhighlight %}


One can use the diff() function to compute one period simple returns of stock
prices.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector ret_simple(NumericVector x) {
   NumericVector vec_diff = diff(x);
   NumericVector res(x.size());
   // pad the front with an NA
   res[0] = NA_REAL;
   for(int i = 1; i < res.size(); i++) {
      res[i] = vec_diff[i-1] / x[i-1];
   }
   return res;
}
{% endhighlight %}


{% highlight r %}
 x <- rnorm(10)
 # Close prices of S&P 500
 y <- c(1418.55, 1427.84, 1428.48, 1419.45, 1413.58, 
        1430.36, 1446.79, 1435.81, 1443.69, 1430.15)
 diff_sug(x)
{% endhighlight %}



<pre class="output">
[1]  1.78889 -1.48820  0.05878  1.58578 -1.25415 -1.72598  0.57821  0.24119
[9]  1.66974
</pre>



{% highlight r %}
 # base R function
 diff(x)
{% endhighlight %}



<pre class="output">
[1]  1.78889 -1.48820  0.05878  1.58578 -1.25415 -1.72598  0.57821  0.24119
[9]  1.66974
</pre>



{% highlight r %}
 ret_simple(y)
{% endhighlight %}



<pre class="output">
 [1]         NA  0.0065489  0.0004482 -0.0063214 -0.0041354  0.0118706
 [7]  0.0114866 -0.0075892  0.0054882 -0.0093787
</pre>

