---
title: STL random_shuffle for permutations
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl featured
summary: Using the STL's random_shuffle function
layout: post
src: 2012-12-30-stl-random-shuffle.cpp
---
The STL also contains random sampling and shuffling algorithms.
We start by looking at `random_shuffle`.

There are two forms. The first uses an internal RNG with its own
seed; the second form allows for a function object conformant to
the STL's requirements (essentially, given `N` produce a uniform
draw greater or equal to zero and less than `N`).  This is useful
for us as it lets us tie this to the same RNG which R uses.



{% highlight cpp %}
#include <Rcpp.h>

// wrapper around R's RNG such that we get a uniform distribution over
// [0,n) as required by the STL algorithm
inline int randWrapper(const int n) { return floor(unif_rand()*n); }

// [[Rcpp::export]]
Rcpp::NumericVector randomShuffle(Rcpp::NumericVector a) {
    // already added by sourceCpp(), but needed standalone
    Rcpp::RNGScope scope;             

    // clone a into b to leave a alone
    Rcpp::NumericVector b = Rcpp::clone(a);

    std::random_shuffle(b.begin(), b.end(), randWrapper);

    return b;
}
{% endhighlight %}


We can illustrate this on a simple example or two:

{% highlight r %}
a <- 1:8
set.seed(42)
randomShuffle(a)
{% endhighlight %}



<pre class="output">
[1] 1 4 3 7 5 8 6 2
</pre>



{% highlight r %}
set.seed(42)
randomShuffle(a)
{% endhighlight %}



<pre class="output">
[1] 1 4 3 7 5 8 6 2
</pre>


By tieing the STL implementation of the random permutation to the
RNG from R, we are able to compute reproducible permutations, fast
and from C++.
