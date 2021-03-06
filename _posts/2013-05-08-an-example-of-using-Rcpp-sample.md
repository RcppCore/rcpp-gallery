---
title: An accept-reject sampler using RcppArmadillo&#58;&#58;sample()
author: Jonathan Olmsted
license: GPL (>= 2)
tags: rng
summary: Demonstrates an accept-reject sampler using the RcppArmadillo-based `sample()` implementation
layout: post
src: 2013-05-08-an-example-of-using-Rcpp-sample.Rmd
---

The recently added `RcppArmadillo::sample()` functionality provides the same
algorithm used in R's `sample()` to Rcpp-level code. Because R's own `sample()`
is written in C with minimal work done in R, writing a wrapper around
`RcppArmadillo::sample()` to then call in R won't get you much of a performance
boost. However, if you need to repeatedly call `sample()`, then calling a single
function which performs everything in Rcpp-land (including multiple calls to
`sample()`) before returning to R can produce a noticeable speedup over a purely
R-based solution.

### Accept-Reject Sampler Example

One place where this situation arises is in an accept-reject sampler where the
candidate "draw" is the output of a call to `sample()`. Concretely, let's
suppose we want to sample 20 integers (without replacement) from 1 to 50 such
that the sum of the 20 integers is less than 400. Far fewer than 10% of randomly
drawn samples will meet this constraint.


{% highlight r %}
require(RcppArmadillo)
{% endhighlight %}



<pre class="output">
Loading required package: RcppArmadillo
</pre>



<pre class="output">
Loading required package: Rcpp
</pre>



{% highlight r %}
require(rbenchmark)
{% endhighlight %}



<pre class="output">
Loading required package: rbenchmark
</pre>


The R code is straightforward enough. It has been written to mirror the logic of
the C++ code, although that doesn't come at the cost of much performance.




{% highlight r %}
r_getInts <- function(samples) {
    thresh <- 400
    results <- matrix(0, 20, samples) ;
    cnt <-  0

    while(cnt < samples) {
        candidate = sample(1:50, 20)

        if (sum(candidate) < thresh) {
            results[, cnt + 1] <- candidate
            cnt <- cnt + 1
        }
    }

    return(results)
}
{% endhighlight %}


Although it is a bit longer, the logic of the C++ code is similar.


{% highlight cpp %}
#include <RcppArmadilloExtensions/sample.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export]]
IntegerMatrix cpp_getInts(int samples
                          ) {
    int cnt = 0 ;
    IntegerMatrix results(20, samples) ;
    IntegerVector frame = seq_len(50) ;
    IntegerVector candidate(20) ;
    int thresh = 400 ;
  
    while (cnt < samples) {
        candidate = RcppArmadillo::sample(frame, 
                                          20, 
                                          FALSE, NumericVector::create()
                                          ) ;
        double sum = std::accumulate(candidate.begin(), candidate.end(), 0.0) ;
    
        if (sum < thresh) {
            results(_, cnt) = candidate ;
            cnt++ ;
        }
    }
  
    return results ;
}
{% endhighlight %}


### Performance

The Rcpp code tends to be about 7-9 times faster and this boost increases as the
constraint becomes more complicated (and necessarily more costly in R).


{% highlight r %}
benchmark(r = {set.seed(1); r_getInts(50)},
          cpp = {set.seed(1); cpp_getInts(50)},
          replications = 10,
          order = 'relative',
          columns = c("test", "replications", "relative", "elapsed")
          ) 
{% endhighlight %}



<pre class="output">
  test replications relative elapsed
2  cpp           10     1.00   0.036
1    r           10    11.97   0.431
</pre>




### In the Real World ...

Where might the structure in this problem arise in practice? One set of
instances are those where "space" matters:

- sampling US cities such that no more than two are in any one state
- sampling cellphone towers such that no two are closer than *X* miles apart
- sampling nodes in a graph/network such that no one has more than *K* edges


In these situations, R code to check the acceptance condition will likely be
less efficient relative to the corresponding C++ code and so even larger
speed-ups are realized.

