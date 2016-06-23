---
title: Finding the minimum of a vector
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl featured
summary: Demonstrates how STL's min_element can be used.
layout: post
src: 2012-12-21-vector-minimum.cpp
---
This example was motivated by
[http://stackoverflow.com/questions/5158219/find-minimum-of-vector-in-rcpp](http://stackoverflow.com/questions/5158219/find-minimum-of-vector-in-rcpp)
and addresses how to find the minumum value and its position index in a
vector.



{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double vecmin(NumericVector x) {
  // Rcpp supports STL-style iterators
  NumericVector::iterator it = std::min_element(x.begin(), x.end());
  // we want the value so dereference 
  return *it;
}
{% endhighlight %}

We can also use the iterator to compute the position, simply by
taking the offset to the vector beginning.

{% highlight cpp %}
// [[Rcpp::export]]
int vecminInd(NumericVector x) {
  // Rcpp supports STL-style iterators
  NumericVector::iterator it = std::min_element(x.begin(), x.end());
  // we want the value so dereference 
  return it - x.begin();
}
{% endhighlight %}

A quick illustration follows. Note that we pad the position by one to adjust for the 0-based versus 1-based indexing between C++ and R.

{% highlight r %}
set.seed(5)
x <- sample(1:100, 10)  # ten out 100
x
{% endhighlight %}



<pre class="output">
 [1] 21 68 90 28 11 67 50 76 88 96
</pre>



{% highlight r %}
cat("Min is ", vecmin(x), " and at position ", vecminInd(x)+1, "\n")
{% endhighlight %}



<pre class="output">
Min is  11  and at position  5 
</pre>

Of course, we subsequently added `min` and `which_min` as sugar
functions, but this example still illustrated how useful the STL
algoritms can be.
