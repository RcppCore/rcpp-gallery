---
title: A first example of using Boost
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics boost
summary: This post shows how to use some functionality from a Boost library
layout: post
src: 2013-01-14-a-first-boost-example.cpp
---
[Boost](http://www.boost.org) is, to quote the quote by Sutter and Alexandrescu 
which adornes the [Boost](http://www.boost.org) website, _...one of the most highly 
regarded and expertly designed C++ library projects in the world_.

The impact of [Boost](http://www.boost.org) on C++ cannot be overstated. 
[Boost](http://www.boost.org) is, at its core, a collection of thoroughly designed and 
peer-reviewed libraries. Some of these have been included into the new C++11 standard (see
our [intro post on C++11](../first-steps-with-C++11)) as for example lambda functions which
we illustrated in [another post on C++11](../simple-lambda-func-c++11).

[Boost](http://www.boost.org) is mostly implemented using
templates. That means headers files only, and compile-time -- but not linking. Which is perfect
for example posts like these.

Many, many [Boost](http://www.boost.org) libraries are useful, and we could fill a series of 
posts.  Here, as an introduction, we going to use two simple functions from the 
[Boost.Math](http://www.boost.org/doc/libs/1_52_0/libs/math/doc/html/index.html)
library to compute greatest common denominator and least common multiple.

I should note that I write this post on a machine with [Boost](http://www.boost.org) headers 
in a standard system location. <em>So stuff just works.</em> If you have to install Boost from source, 
and into a non-standard location, you may need to add a <code>-I</code> flag, not unlike how added 
the C++11 flag in [this post](../first-steps-with-C++11) .




{% highlight cpp %}
#include <Rcpp.h>
#include <boost/math/common_factor.hpp>  // one file, automatically found here

using namespace Rcpp;
 
// [[Rcpp::export]]
int computeGCD(int a, int b) {
    return boost::math::gcd(a, b);
}

// [[Rcpp::export]]
int computeLCM(int a, int b) {
    return boost::math::lcm(a, b);
}
{% endhighlight %}


We can test these:


{% highlight r %}
a <- 6
b <- 15
cat( c(computeGCD(a,b), computeLCM(a,b)), "\n")
{% endhighlight %}



<pre class="output">
3 30 
</pre>



{% highlight r %}

a <- 96
b <- 484
cat( c(computeGCD(a,b), computeLCM(a,b)), "\n")
{% endhighlight %}



<pre class="output">
4 11616 
</pre>


Benchmark


{% highlight r %}
library(rbenchmark)
library(numbers)

a <- 962
b <- 4842

benchmark(r1 = c(computeGCD(a,b), computeLCM(a,b)),
          r2 = c(GCD(a,b), LCM(a,b)),
          replications = 1000)
{% endhighlight %}



<pre class="output">
  test replications elapsed relative user.self sys.self user.child
1   r1         1000   0.010      1.0     0.009    0.001          0
2   r2         1000   0.057      5.7     0.058    0.000          0
  sys.child
1         0
2         0
</pre>

