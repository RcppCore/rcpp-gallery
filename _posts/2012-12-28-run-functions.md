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
involved. The TTR package contains several run functions
that are very fast because they call Fortran and C routines. With Rcpp and
the C++ STL one can easily write run functions to use in R.

{% highlight cpp %}
#include <Rcpp.h>
#include <numeric>      // for accumulate

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector run_sum(NumericVector x, int n) {
    int sz = x.size();
    NumericVector res(sz);

    res[n-1] = std::accumulate(x.begin(), x.end()-sz+n, 0.0);

    for(int i = n; i < sz; i++) {
       res[i] = res[i-1] + x[i] - x[i-n];
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}

{% highlight r %}
suppressMessages(library(TTR))

# use a small sample set for demonstration purposes
set.seed(123)
x <- rnorm(10)
x
{% endhighlight %}



<pre class="output">
 [1] -0.5604756 -0.2301775  1.5587083  0.0705084  0.1292877  1.7150650
 [7]  0.4609162 -1.2650612 -0.6868529 -0.4456620
</pre>



{% highlight r %}
n <- 4

# Check to make sure that the result of run_sum matches
# runSum from the TTR package
stopifnot(all.equal(run_sum(x, n), runSum(x, n)))

run_sum(x, n)
{% endhighlight %}



<pre class="output">
 [1]        NA        NA        NA  0.838564  1.528327  3.473569  2.375777
 [8]  1.040208  0.224067 -1.936660
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
# Check to make sure that the result of run_mean matches
# runMean from the TTR package
stopifnot(all.equal(run_mean(x, n), runMean(x, n)))

run_mean(x, n)
{% endhighlight %}



<pre class="output">
 [1]         NA         NA         NA  0.2096409  0.3820817  0.8683924
 [7]  0.5939443  0.2600519  0.0560168 -0.4841650
</pre>

With `min_element` and `max_element` from the algorithm header, one can
also easily write a function that calculates the min and the
max of a range over a running window. Note the `*` to dereference
the iterator to obtain the value. See [Finding the minimum of a vector](https://gallery.rcpp.org/articles/vector-minimum/)
for another example of using `min_element`.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_min(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = *std::min_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}

{% highlight r %}
# Check to make sure that the result of run_min matches
# runMin from the TTR package
stopifnot(all.equal(run_min(x, n), runMin(x, n)))

run_min(x, n)
{% endhighlight %}



<pre class="output">
 [1]         NA         NA         NA -0.5604756 -0.2301775  0.0705084
 [7]  0.0705084 -1.2650612 -1.2650612 -1.2650612
</pre>

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_max(NumericVector x, int n){
    int sz = x.size();
    NumericVector res(sz);
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = *std::max_element(x.begin() + i, x.end() - sz + n + i);
    }
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}

{% highlight r %}
# Check to make sure that the result of run_max matches
# runMax from the TTR package
stopifnot(all.equal(run_max(x, n), runMax(x, n)))

run_max(x, n)
{% endhighlight %}



<pre class="output">
 [1]       NA       NA       NA 1.558708 1.558708 1.715065 1.715065
 [8] 1.715065 1.715065 0.460916
</pre>

This post demonstrates how to incorporate a few useful functions
from the STL, `accumulate`, `min_element`, and
`max_element`, to write 'run' functions with Rcpp.
