---
title: Using STL Iterators
author: Hadley Wickham
license: GPL (>= 2)
tags: stl
summary: Demonstrates the basics of using STL iterators to traverse
  Rcpp vectors.
layout: post
src: 2012-12-15-using-stl-iterators.cpp
---



Iterators are used extensively in the STL: many functions either accept or 
return iterators. They are the next step up from basic loops, abstracting 
away the details of the underlying data structure. Iterators have three main 
operators: they can be advanced with `++`, dereferenced (to get the value
they refer to) with `*` and compared using `==`. For example we could
write a simple sum function using iterators:

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double calculateSum(NumericVector x) {
  double total = 0;
  for(NumericVector::iterator it = x.begin(); it != x.end(); ++it) {
    total += *it;
  }
  return total;
}
{% endhighlight %}


Some differences to highlight about using iterators rather than 
contentional for loop indexing:

- We start at `x.begin()` and loop until we get to `x.end()`. 
- Instead of indexing into x, we use the dereference operator to get
its current value: `*it`.
- Notice the type of the iterator: `NumericVector::iterator`. Each 
vector type has its own iterator type: `LogicalVector::iterator`, 
`CharacterVector::iterator` etc.

Iterators also allow us to use the C++ equivalents of the apply family of 
functions. For example, we could again rewrite sum to use the
`std::accumulate` function, which takes an starting and ending iterator and
adds all the values in between. To use accumulate we need to include the
`<numeric>` header.

{% highlight cpp %}
#include <numeric>

// [[Rcpp::export]]
double sum4(NumericVector x) {
  return std::accumulate(x.begin(), x.end(), 0.0);
}
{% endhighlight %}


The third argument to accumulate gives the initial value: it's particularly
important because this also determines the data type that accumulate uses
(here we use 0.0 and not 0 so that accumulate uses a double, not an int.).
