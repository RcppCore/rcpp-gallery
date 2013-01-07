---
title: Performance Benchmark of Running Sum Functions 
author: Ross Bennett
license: GPL (>= 2)
tags: benchmark
summary: Benchmarking the performance of running sum functions
  in R and C++ using Rcpp
layout: post
src: 2013-01-06-run_sum-benchmark.cpp
---



First, let us consider a running sum function in pure R. To get started,
I looked at the source code of the [TTR package](http://cran.r-project.org/web/packages/TTR/index.html) to see the algorithm
used in `runSum`. The `runSum` function uses a Fortran routine to compute
the running/rolling sum of a vector. The `run_sum_R` function below is
my interpretation of that algorithm implemented in R.
Many thanks to the package author, Joshua Ulrich, for pointing out to me
that, in many cases, the algorithm is more critical to performance than 
the language.

{% highlight r %}
run_sum_R <- function(x, n) {
  # x : input vector
  # n : size of window

  # size of input vector
  sz <- length(x)
  
  # initialize the output vector
  ov <- vector(mode = "numeric", length = sz)
  
  # sum the values from the beginning of the vector to n
  ov[n] <- sum(x[1:n])
  
  # loop through the rest of the vector
  for(i in (n+1):sz) {
    ov[i] <- ov[i-1] + x[i] - x[i-n]
  }
  
  # pad the first n-1 values with NA
  ov[1:(n-1)] <- NA
  return(ov)
}

suppressMessages(library(TTR))
library(rbenchmark)

set.seed(123)
x <- rnorm(100000)

stopifnot(all.equal(run_sum_R(x, 500), runSum(x, 500)))

# benchmark run_sum_R for given values of x and n
benchmark(run_sum_R(x, 500),  run_sum_R(x, 2500),
          run_sum_R(x, 4500), run_sum_R(x, 6500),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
                test replications elapsed relative
1  run_sum_R(x, 500)          100   33.21    1.063
2 run_sum_R(x, 2500)          100   32.52    1.041
3 run_sum_R(x, 4500)          100   31.86    1.019
4 run_sum_R(x, 6500)          100   31.26    1.000
</pre>


Now let us consider a running sum function in C++, call it `run_sum_v1`.
One approach is to loop through each element of the given vector
calling std::accumulate to compute the running sum over the window.

{% highlight cpp %}
#include <Rcpp.h>
#include <numeric>      // for accumulate
#include <vector>

using namespace Rcpp;

// [[Rcpp::export]]
std::vector<double> run_sum_v1(std::vector<double> x, int n) {
    // x : input vector
    // n : size of window
    
    // size of the input vector
    int sz = x.size();
    
    // initialize res vector
    std::vector<double> res(sz);
    
    // loop through the vector calling std::accumulate to
    // compute the running sum
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = std::accumulate(x.begin()+i, x.end()-sz+n+i, 0.0);
    }
    
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
	return res;
}
{% endhighlight %}


{% highlight r %}
stopifnot(all.equal(run_sum_v1(x, 500), runSum(x, 500)))

# benchmark run_sum_v1 for given values of x and n
benchmark(run_sum_v1(x, 500),  run_sum_v1(x, 2500),
          run_sum_v1(x, 4500), run_sum_v1(x, 6500),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative
1  run_sum_v1(x, 500)          100   4.183    1.000
2 run_sum_v1(x, 2500)          100  20.598    4.924
3 run_sum_v1(x, 4500)          100  36.360    8.692
4 run_sum_v1(x, 6500)          100  51.459   12.302
</pre>


The benchmark results of `run_sum_v1` are not very impressive. The
time increases fairly linearly as `n` increases.
This is due to having `std::accumulate` inside the for loop.
For a vector of size 100,000 and `n = 5000`, `std::accumulate` is 
called 95,001 times.

An interesting result is that `run_sum_R` is faster than `run_sum_v1` for
`n=4500` and `n=6500` of the benchmark. This example demonstrates that it 
is not always the case that C++ code is faster than R code.

This is obviously not an "apples-to-apples" comparison because a different
algorithm is used, but the point of the example is to demonstrate the 
importance of the algorithm regardless of the programming language.

Now let us write another running sum function in C++ that uses
the same algorithm that is used in `run_sum_R`, call it `run_sum_v2`. 

{% highlight cpp %}
// [[Rcpp::export]]
std::vector<double> run_sum_v2(std::vector<double> x, int n) {
    // x : input vector
    // n : size of window
    
    // size of input vector
    int sz = x.size();
    
    // initialize res vector
    std::vector<double> res(sz);
    
    // sum the values from the beginning of the vector to n 
    res[n-1] = std::accumulate(x.begin(), x.end()-sz+n, 0.0);
    
    // loop through the rest of the vector
    for(int i = n; i < sz; i++) {
       res[i] = res[i-1] + x[i] - x[i-n];
    }
    
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    return res;
}
{% endhighlight %}


{% highlight r %}
stopifnot(all.equal(run_sum_v2(x, 500), runSum(x, 500)))

# benchmark run_sum_v2 for given values of x and n
benchmark(run_sum_v2(x, 500),  run_sum_v2(x, 2500),
          run_sum_v2(x, 4500), run_sum_v2(x, 6500),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative
1  run_sum_v2(x, 500)          100   0.092    1.011
2 run_sum_v2(x, 2500)          100   0.091    1.000
3 run_sum_v2(x, 4500)          100   0.109    1.198
4 run_sum_v2(x, 6500)          100   0.091    1.000
</pre>


The benchmark results of `run_sum_v2` are relatively fast and much more
favorable than both `run_sum_R` and `run_sum_v1`. The elapsed time is 
about a tenth of a second and is fairly constant across the given 
values of `n`.

Finally, let us benchmark `runSum` from the TTR package.

{% highlight r %}
# benchmark runSum for given values of x and n
benchmark(runSum(x, 500),  runSum(x, 2500),
          runSum(x, 4500), runSum(x, 6500),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
             test replications elapsed relative
1  runSum(x, 500)          100   0.703    1.077
2 runSum(x, 2500)          100   0.653    1.000
3 runSum(x, 4500)          100   0.764    1.170
4 runSum(x, 6500)          100   0.740    1.133
</pre>


The benchmark results of `runSum` are also quite good. The elapsed time is
about a seven tenths of a second and is fairly constant across the
given values of `n`. It should be noted that `runSum` does some extra
work in R such as checking for a valid `n`, non-leading NAs, etc.
and should be considered when comparing the benchark results of 
`run_sum_v2` to `runSum`.
