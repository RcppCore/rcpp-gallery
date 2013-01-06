
{% highlight cpp %}
// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
{% endhighlight %}

---
title: Calling R Functions from C++
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: function featured
summary: This post discusses calling R functions from C++
layout: post
src: 2013-01-05-r-function-from-c++.cpp
---

At its very essence, Rcpp permits easy access to native R objects at the C++ level. R objects can be 
- simple vectors, list or matrices;
- compound data structures created from these; 
- objects of S3, S4 or Reference Class vintage; or
- language objects as for example [environments](../accessing-environments).

Accessing a function object is no different.  And calling a
function can be very useful.  Maybe to pick up parameter
initializations, maybe to access a custom data summary that would
be tedious to recode, or maybe even calling a plotting routine.  We
already have examples for just about all of these use case in the
Rcpp examples or unit tests shipping with the package.

So here were a just providing a simple example of calling a summary
function, namely the Tukey `fivenum()`.

But before we proceed, a warning.  Calling a function is simple and
tempting. It is also slow as there are overheads involved.  And
calling it repeatedly from inside your C++ code, possibly buried
within several loops, is outright silly. This _has to be_ slower
than equivalent C++ code, and even slower than just the R (because
of the marshalling).  Do it when it makes sense, and not simply
because it is available.



{% highlight r %}
set.seed(42)
x <- rnorm(1e5)
fivenum(x)
{% endhighlight %}



<pre class="output">
[1] -4.043276 -0.682384 -0.002066  0.673325  4.328091
</pre>


Now via this C++ code:

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector callFunction(NumericVector x, Function f) {
    NumericVector res = f(x);
    return res;
}
{% endhighlight %}


And unsurprisingly, calling the same function on the same data gets the same result:

{% highlight r %}
callFunction(x, fivenum)
{% endhighlight %}



<pre class="output">
[1] -4.043276 -0.682384 -0.002066  0.673325  4.328091
</pre>

