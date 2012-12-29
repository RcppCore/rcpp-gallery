---
title: Run Functions with Rcpp
author: Ross Bennett
license: GPL (>= 2)
tags: stl
summary: Demonstrates writing functions over a running window
layout: post
src: 2012-12-28-run-functions.cpp
---



Writing running functions in R can be slow because of the loops
involved. The TTR package contains several "run" functions
that are very fast because they call Fortran routines. With Rcpp and
the C++ STL one can easily write run functions to use in R.

{% highlight cpp %}
#include <Rcpp.h>
#include <algorithm>    // for max_element and min_element
#include <numeric>      // for accumulate

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector run_sum(NumericVector x, int n) {
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = std::accumulate(x.begin()+i, x.end()-sz+n+i, 0.0);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
	return res;
}
{% endhighlight %}


{% highlight r %}
 x <- sample(10)
 x
{% endhighlight %}



<pre class="output">
 [1]  5  8  9  1  4 10  3  2  6  7
</pre>



{% highlight r %}
 n <- 3
 run_sum(x, n)
{% endhighlight %}



<pre class="output">
 [1] NA NA 22 18 14 15 17 15 11 15
</pre>


Now that we have the function `run_sum` written, we can easily
use this to write the function `run_mean` just by dividing
`run_sum` by `n`. Another point to note is the syntax
to cast `n` to a double so we do not lose the decimal points
with integer division.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_mean(NumericVector x, int n) {
    return run_sum(x, n) / (double)n;
}
{% endhighlight %}


{% highlight r %}
 run_mean(x, n)
{% endhighlight %}



<pre class="output">
 [1]    NA    NA 7.333 6.000 4.667 5.000 5.667 5.000 3.667 5.000
</pre>


With `min_element` and `max_element` from the algorithm header, one can
also easily write a function that calculates the min and the
max of a range over a running window. Note the `*` to dereference
the iterator to obtain the value. See [Finding the minimum of a vector](http://gallery.rcpp.org/articles/vector-minimum/)
for another example of using `min_element`.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_min(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = *std::min_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}


{% highlight r %}
 run_min(x, n)
{% endhighlight %}



<pre class="output">
 [1] NA NA  5  1  1  1  3  2  2  2
</pre>


{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_max(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < sz; i++){
        res[i+n-1] = *std::max_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}


{% highlight r %}
 run_max(x, n)
{% endhighlight %}



<pre class="output">
 [1] NA NA  9  9  9 10 10 10  6  7
</pre>


This post demonstrates how to incorporate a few useful functions 
from the STL. The STL functions `accumulate`, `min_element`, and 
`max_element` utilize iterators to iterate over a range. The run
functions above demonstrate how one can use a for loop and and 
some math with the iterators to write running functions.
