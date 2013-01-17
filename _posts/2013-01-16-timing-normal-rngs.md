---
title: Timing normal RNGs
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: rng boost c++11 featured
summary: This post compares drawing N(0,1) vectors from R, Boost and C++11
layout: post
src: 2013-01-16-timing-normal-rngs.Rmd
---

In previous articles, we have seen that Rcpp can be particularly
useful for simulations as it executes code at C++ speed. A very
useful feature the API provided by R is the access to the R RNGs
so that simulations at the C++ level can get precisely the same
stream of random numbers as an R application would. 

But sometimes that is not a requirement, and here will look into
drawing normals from R, from the random number generator in Boost
and the new one in C++11.

A first approach is by far the easiest: using Rcpp and its sugar
function which reduces this to a single statement.


{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rcppNormals(int n) {
    RNGScope scope;             // also done by sourceCpp()
    return rnorm(n);
}
{% endhighlight %}


A quick test:


{% highlight r %}
set.seed(42)
rcppNormals(10)
{% endhighlight %}



<pre class="output">
 [1]  1.37096 -0.56470  0.36313  0.63286  0.40427 -0.10612  1.51152
 [8] -0.09466  2.01842 -0.06271
</pre>


Next, the same via Boost. The caveats from the previous two Boost
pieces apply: on some systems you may have to ensure access to the
Boost headers, on some (such as my Linux system) it just works.



{% highlight cpp %}

#include <Rcpp.h>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include <boost/random/normal_distribution.hpp>

using namespace std;
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector boostNormals(int n) {
    
    typedef boost::mt19937 RNGType; 	// select a generator, MT good default
    RNGType rng(123456);			// instantiate and seed

    boost::normal_distribution<> n01(0.0, 1.0);
    boost::variate_generator< RNGType, boost::normal_distribution<> > rngNormal(rng, n01);

    NumericVector V(n);
    for ( int i = 0; i < n; i++ ) {
        V[i] = rngNormal();
    };
  
    return V;
}
{% endhighlight %}


A second test:


{% highlight r %}
boostNormals(10)
{% endhighlight %}



<pre class="output">
 [1]  0.8400  0.8610  2.0907 -0.4437 -0.1029  1.5609  1.3874 -1.0453
 [9] -1.6558  1.6198
</pre>


And now for the same using the random number generator added to
C++11. Here, the same caveats apply as before: we need to enable
the C++11 extensions:


{% highlight r %}
Sys.setenv("PKG_CXXFLAGS"="-std=c++11")
{% endhighlight %}


That way, we can compile this code:


{% highlight cpp %}

#include <Rcpp.h>
#include <random>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector cxx11Normals(int n) {

    std::mt19937 engine(42);
    std::normal_distribution<> normal(0.0, 1.0);

    NumericVector V(n);
    for ( int i = 0; i < n; i++ ) {
        V[i] = normal(engine);
    };

    return V;
}
{% endhighlight %}


And run it:


{% highlight r %}
cxx11Normals(10)
{% endhighlight %}



<pre class="output">
 [1] -0.5502  0.5154  0.4739  1.3685 -0.9168 -0.1241 -2.0110 -0.4928
 [9]  0.3926 -0.9292
</pre>


Lastly, we can compare the runtime for these three in a quick benchmark study.


{% highlight r %}
library(rbenchmark)

n <- 1e5

res <- benchmark(rcppNormals(n),
                 boostNormals(n),
                 cxx11Normals(n),
                 order="relative",
                 replications = 500)
print(res[,1:4])
{% endhighlight %}



<pre class="output">
             test replications elapsed relative
3 cxx11Normals(n)          500   3.997    1.000
1  rcppNormals(n)          500   4.123    1.032
2 boostNormals(n)          500   4.353    1.089
</pre>


In this particularly example, all the RNGs take roughly the same time. It would be 
interesting to see how the Ziggurat algorithm (which is known to produce Normals rather 
fast) would fare.
