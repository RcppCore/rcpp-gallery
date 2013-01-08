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

  sz <- length(x)
  
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
x <- rnorm(10000)

# benchmark run_sum_R for given values of x and n
benchmark( run_sum_R(x, 50), run_sum_R(x, 100),
          run_sum_R(x, 150), run_sum_R(x, 200),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
               test replications elapsed relative
1  run_sum_R(x, 50)          100   3.364    1.007
2 run_sum_R(x, 100)          100   3.339    1.000
3 run_sum_R(x, 150)          100   3.390    1.015
4 run_sum_R(x, 200)          100   3.590    1.075
</pre>


For these benchmarks, I will just focus on the performance of the functions
for a fixed `x` and varying the value of `n`. The results of the benchmark 
of `run_sum_R` show that the elapsed time is fairly constant for the 
given values of `n` (i.e. O(1)).

Now let us consider a running sum function in C++, call it `run_sum_v1`.
One approach is to loop through each element of the given vector
calling std::accumulate to compute the running sum.

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector run_sum_v1(NumericVector x, int n) {
    
    int sz = x.size();

    NumericVector res(sz);
    
    // loop through the vector calling std::accumulate
    for(int i = 0; i < (sz-n+1); i++){
        res[i+n-1] = std::accumulate(x.begin()+i, x.end()-sz+n+i, 0.0);
    }
    
    // pad the first n-1 elements with NA
    std::fill(res.begin(), res.end()-sz+n-1, NA_REAL);
    
	return res;
}
{% endhighlight %}


{% highlight r %}
# benchmark run_sum_v1 for given values of x and n
benchmark( run_sum_v1(x, 50), run_sum_v1(x, 100),
          run_sum_v1(x, 150), run_sum_v1(x, 200),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
                test replications elapsed relative
1  run_sum_v1(x, 50)          100   0.045    1.000
2 run_sum_v1(x, 100)          100   0.088    1.956
3 run_sum_v1(x, 150)          100   0.128    2.844
4 run_sum_v1(x, 200)          100   0.170    3.778
</pre>


Although the elapsed times of `run_sum_v1` are quite fast, note that the
time increases approximately linearly as `n` increases (i.e. O(N)). This 
will become a problem if we use this function with large values of `n`. 

Now let us write another running sum function in C++ that uses
the same algorithm that is used in `run_sum_R`, call it `run_sum_v2`. 

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector run_sum_v2(NumericVector x, int n) {
    
    int sz = x.size();
    
    NumericVector res(sz);
    
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
# benchmark run_sum_v2 for given values of x and n
benchmark( run_sum_v2(x, 50), run_sum_v2(x, 100),
          run_sum_v2(x, 150), run_sum_v2(x, 200),
          order = NULL)[,1:4]
{% endhighlight %}



<pre class="output">
                test replications elapsed relative
1  run_sum_v2(x, 50)          100   0.007        1
2 run_sum_v2(x, 100)          100   0.007        1
3 run_sum_v2(x, 150)          100   0.007        1
4 run_sum_v2(x, 200)          100   0.007        1
</pre>


The benchmark results of `run_sum_v2` are quite fast and much more
favorable than both `run_sum_R` and `run_sum_v1`. The elapsed time is 
approximately constant across the given  values of `n` (i.e O(N)).

Finally, let us benchmark all three functions as well as `runSum` from 
the TTR package for a point of reference using larger values for the 
size of `x` and `n`.

{% highlight r %}
set.seed(42)
y <- rnorm(100000)

# benchmark runSum for given values of x and n
benchmark(    runSum(y, 4500), run_sum_v1(y, 4500),
          run_sum_v2(y, 4500),  run_sum_R(y, 4500),
          order = "relative")[,1:4]
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative
3 run_sum_v2(y, 4500)          100   0.082     1.00
1     runSum(y, 4500)          100   0.889    10.84
4  run_sum_R(y, 4500)          100  33.717   411.18
2 run_sum_v1(y, 4500)          100  37.538   457.78
</pre>


An interesting result of benchmarking with these larger values is 
that `run_sum_R` is faster than `run_sum_v1` for the given values.
This example demonstrates that it is not always the case that C++ code 
is faster than R code. The inefficiency of `run_sum_v1` is due to having 
`std::accumulate` inside the for loop. For a vector of size 100,000 and 
`n = 5000`, `std::accumulate` is called 95,001 times!
 
This is obviously not an "apples-to-apples" comparison because a different
algorithm is used, but the point of the example is to demonstrate the 
importance of the algorithm regardless of the programming language.

It should be noted that `runSum` does some extra
work in R such as checking for a valid `n`, non-leading NAs, etc.
and should be considered when comparing the benchmark results of 
`run_sum_v2` to `runSum`.
