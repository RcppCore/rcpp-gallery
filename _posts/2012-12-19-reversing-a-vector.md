---
title: Reversing a Vector
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl
summary: Compares reversing a numeric vector the R C API and Rcpp 
  with an STL algorithm.
layout: post
src: 2012-12-19-reversing-a-vector.cpp
---



To start with and for purposes of comparison, we reverse a numeric
vector using the R C API (note that this example was taken from 
Jeff Ryan's esotericR package):

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

SEXP rev (SEXP x) {
  SEXP res;
  int i, r, P=0;
  PROTECT(res = Rf_allocVector(REALSXP, Rf_length(x))); P++;
  for(i=::Rf_length(x), r=0; i>0; i--, r++) {
     REAL(res)[r] = REAL(x)[i-1];
  }
  ::Rf_copyMostAttrib(x, res);
  UNPROTECT(P);
  return res;
}
{% endhighlight %}


Here's the same operation implemented using Rcpp and calling the 
`std::reverse` function from the C++ standard library:

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector rcppRev(NumericVector x) {
   NumericVector revX = clone<NumericVector>(x);
   std::reverse(revX.begin(), revX.end());
   ::Rf_copyMostAttrib(x, revX); 
   return revX;
} 
{% endhighlight %}


Here's an illustration of calling our `rcppRev` function from R:

{% highlight r %}
obj <- structure(seq(0, 1, 0.1), obligatory="hello, world!")
obj
{% endhighlight %}



<pre class="output">
 [1] 0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
attr(,"obligatory")
[1] "hello, world!"
</pre>



{% highlight r %}
rcppRev(obj)
{% endhighlight %}



<pre class="output">
 [1] 1.0 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1 0.0
attr(,"obligatory")
[1] "hello, world!"
</pre>


Both the `obj` variable and the new copy contain the desired data attribute,
the new copy is reversed, the original is untouched. All in four lines of
C++ not requiring explicit memory managment or easy to get wrong array
manipulations.
