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
1  run_sum_R(x, 500)          100   49.53    1.000
2 run_sum_R(x, 2500)          100   52.99    1.070
3 run_sum_R(x, 4500)          100   61.07    1.233
4 run_sum_R(x, 6500)          100   65.01    1.313
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
1  run_sum_v1(x, 500)          100   7.635    1.000
2 run_sum_v1(x, 2500)          100  39.827    5.216
3 run_sum_v1(x, 4500)          100  75.232    9.854
4 run_sum_v1(x, 6500)          100 147.537   19.324
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
1  run_sum_v2(x, 500)          100   0.183    1.052
2 run_sum_v2(x, 2500)          100   0.182    1.046
3 run_sum_v2(x, 4500)          100   0.174    1.000
4 run_sum_v2(x, 6500)          100   0.224    1.287
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
1  runSum(x, 500)          100   2.080    1.094
2 runSum(x, 2500)          100   1.976    1.039
3 runSum(x, 4500)          100   1.902    1.000
4 runSum(x, 6500)          100   1.970    1.036
</pre>


The benchmark results of `runSum` are also quite good. The elapsed time is
about a seven tenths of a second and is fairly constant across the
given values of `n`. It should be noted that `runSum` does some extra
work in R such as checking for a valid `n`, non-leading NAs, etc.
and should be considered when comparing the benchmark results of 
`run_sum_v2` to `runSum`.
