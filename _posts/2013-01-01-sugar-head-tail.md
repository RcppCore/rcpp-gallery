---
title: Sugar Functions head and tail
author: Ross Bennett
license: GPL (>= 2)
tags: sugar stl
summary: Illustrate use of sugar functions head and tail
layout: post
src: 2013-01-01-sugar-head-tail.cpp
---



The sugar function `head` returns the first n elements of the input vector.
The sugar function `tail` returns the last n elements of the input vector.
With Rcpp sugar, the functions `head` and `tail` work the same way
as they do in R.

Here we use `std::sort` from the STL and then `tail` to return the top
n items (items with the highest values) of the input vector.

{% highlight cpp %}
#include <algorithm>	// for sort
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector top_n(NumericVector x, int n){
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return tail(x, n);
}
{% endhighlight %}


{% highlight r %}
x <- rnorm(10)
x
{% endhighlight %}



<pre class="output">
 [1] -0.23018  1.55871  0.07051  0.12929  1.71506  0.46092 -1.26506
 [8] -0.68685 -0.44566  1.22408
</pre>



{% highlight r %}
top_n(x, 3)
{% endhighlight %}



<pre class="output">
[1] 1.224 1.559 1.715
</pre>


Here we use `std::sort` from the STL and then `head` to return the bottom
n items (items with the lowest values) of the input vector.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector bottom_n(NumericVector x, int n){
    std::sort(x.begin(), x.end());	// sort x in ascending order
    return head(x, n);
}
{% endhighlight %}


{% highlight r %}
bottom_n(x, 3)
{% endhighlight %}



<pre class="output">
[1] -1.2651 -0.6869 -0.4457
</pre>

