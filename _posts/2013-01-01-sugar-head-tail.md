---
title: Sugar Functions head and tail
author: Ross Bennett
license: GPL (>= 2)
tags: sugar stl
summary: Illustrate use of sugar functions head and tail
layout: post
src: 2013-01-01-sugar-head-tail.cpp
---



The R functions `head` and `tail` return the first (last) n elements
of the input vector.  With Rcpp sugar, the functions `head` and `tail`
work the same way as they do in R.
 
Here we use `std::sort` from the STL and then `tail` to return the top
n items (items with the highest values) of the input vector.

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector top_n(NumericVector y, int n){
    NumericVector x = clone(y);
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return tail(x, n);
}
{% endhighlight %}


A simple illustration:

{% highlight r %}
set.seed(42)
x <- rnorm(10)
x
{% endhighlight %}



<pre class="output">
 [1]  1.37096 -0.56470  0.36313  0.63286  0.40427 -0.10612  1.51152
 [8] -0.09466  2.01842 -0.06271
</pre>



{% highlight r %}
top_n(x, 3)
{% endhighlight %}



<pre class="output">
[1] 1.371 1.512 2.018
</pre>


Here we use `std::sort` from the STL and then `head` to return the bottom
n items (items with the lowest values) of the input vector.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector bottom_n(NumericVector y, int n){
    NumericVector x = clone(y);
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return head(x, n);
}
{% endhighlight %}


{% highlight r %}
bottom_n(x, 3)
{% endhighlight %}



<pre class="output">
[1] -0.56470 -0.10612 -0.09466
</pre>

