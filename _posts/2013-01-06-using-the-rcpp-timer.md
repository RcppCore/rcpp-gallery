---
title: Using the Rcpp Timer
author: Dirk Eddelbuettel
updated: Dec 30, 2016
license: GPL (>= 2)
tags: benchmark rng featured
summary: This post shows how to use the Timer class in Rcpp
layout: post
src: 2013-01-06-using-the-rcpp-timer.cpp
---

Sine the 0.10.2 release, Rcpp contains an internal class `Timer`
which can be used for fine-grained benchmarking.  Romain motivated
`Timer` in a 
[post to the mailing list](http://article.gmane.org/gmane.comp.lang.r.rcpp/4525) 
where `Timer` is used to measure the different components of the costs of
random number generation.

A slightly modified version of that example follows below.


{% highlight cpp %}
#include <Rcpp.h>
#include <Rcpp/Benchmark/Timer.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector useTimer() {
    int n = 1000000;

    // start the timer
    Timer timer;
    timer.step("start");        // record the starting point

    for (int i=0; i<n; i++) {
        GetRNGstate();
        PutRNGstate();
    }
    timer.step("get/put");      // record the first step

    for (int i=0; i<n; i++) {
        GetRNGstate();
        rnorm(10, 0.0, 1.0);
        PutRNGstate();
    }
    timer.step("g/p+rnorm()");  // record the second step

    for (int i=0; i<n; i++) {
        // empty loop
    }
    timer.step("empty loop");   // record the final step

    NumericVector res(timer);   // 
    for (int i=0; i<res.size(); i++) {
        res[i] = res[i] / n;
    }
    return res;
}
{% endhighlight %}

We get the following result, each expressing the cost per iteration in nanoseconds, 
both cumulative (default) and incrementally (by taking differences).

{% highlight r %}
res <- useTimer()
res          # normal results: cumulative
{% endhighlight %}



<pre class="output">
      start     get/put g/p+rnorm()  empty loop 
   0.000156 1571.057133 3920.082384 3920.085223 
</pre>



{% highlight r %}
diff(res)    # simple difference
{% endhighlight %}



<pre class="output">
    get/put g/p+rnorm()  empty loop 
1571.056977 2349.025251    0.002839 
</pre>

The interesting revelation is that *repeatedly* calling
`GetRNGstate()` and `PutRNGstate()` can amount to about 60% of the
cost of RNG draws.  Luckily, we usually only have to call these
helper functions once per subroutine called from R (rather than
repeatedly as shown here) so this is not really a permanent cost to
bear when running simulations with R. 

It also show the usefulness of a fine-grained timer at the code level.
