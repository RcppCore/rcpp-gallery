---
title: Transforming a Matrix with STL Algorithms
author: Romain Francois 
license: GPL (>= 2)
tags: matrix stl featured
summary: Demonstrates transforming a matrix passed to a function 
  using std::transform.
layout: post
src: 2012-12-14-transforming-a-matrix.cpp
---
Here we take the square root of each item of a matrix and return a 
new matrix with the tranformed values. We do this by using
`std::transform` to call the `sqrt` function on each element of
the matrix:



{% highlight cpp %}
#include <Rcpp.h>
#include <cmath>

using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix matrixSqrt(NumericMatrix orig) {

  // allocate the matrix we will return
  NumericMatrix mat(orig.nrow(), orig.ncol());
  
  // transform it 
  std::transform(orig.begin(), orig.end(), mat.begin(), ::sqrt);
  
  // return the new matrix
  return mat;
}
{% endhighlight %}


Here we call the function from R:

{% highlight r %}
m <- matrix(c(1,2,3, 11,12,13), nrow = 2, ncol=3)
matrixSqrt(m)
{% endhighlight %}



<pre class="output">
      [,1]  [,2]  [,3]
[1,] 1.000 1.732 3.464
[2,] 1.414 3.317 3.606
</pre>

