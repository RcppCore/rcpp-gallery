---
title: Passing user-supplied C++ functions with RcppXPtrUtils
author: Iñaki Ucar
license: GPL (>= 2)
tags: function
summary: Demonstrates how to build and check user-supplied C++ functions 
  with the RcppXPtrUtils package
layout: post
src: 2017-08-04-passing-cpp-function-pointers-rcppxptrutils.Rmd
---

Sitting on top of R's external pointers, the `RcppXPtr` class provides
a powerful and generic framework for
[Passing user-supplied C++ functions](http://gallery.rcpp.org/articles/passing-cpp-function-pointers/)
to a C++ backend. This technique is exploited in the
[RcppDE](https://cran.r-project.org/package=RcppDE) package, an
efficient C++ based implementation of the
[DEoptim](https://cran.r-project.org/package=DEoptim) package that
accepts optimisation objectives as both R and compiled functions (see
`demo("compiled", "RcppDE")` for further details). This solution has a
couple of issues though:

1. Some repetitive scaffolding is always needed in order to bring the `XPtr` to R space.
2. There is no way of checking whether a user-provided C++ function
   complies with the internal signature supported by the C++ backend,
   which may lead to weird runtime errors.

## Better `XPtr` handling with RcppXPtrUtils

In a nutshell, RcppXPtrUtils provides functions for dealing with these
two issues: namely, `cppXPtr` and `checkXPtr`. As a package author,
you only need to 1) import and re-export `cppXPtr` to compile code and
transparently retrieve an `XPtr`, and 2) use `checkXPtr` to internally
check function signatures.

`cppXPtr` works in the same way as `Rcpp::cppFunction`, but instead of
returning a wrapper to directly call the compiled function from R, it
returns an `XPtr` to be passed to, unwrapped and called from C++. The
returned object is an R's `externalptr` wrapped into a class called
`XPtr` along with additional information about the function signature.


{% highlight r %}
library(RcppXPtrUtils)

ptr <- cppXPtr("double foo(int a, double b) { return a + b; }")
class(ptr)
{% endhighlight %}



<pre class="output">
[1] &quot;XPtr&quot;
</pre>



{% highlight r %}
ptr
{% endhighlight %}



<pre class="output">
'double foo(int a, double b)' &lt;pointer: 0x55e176e4c3f0&gt;
</pre>

The `checkXptr` function checks the object against a given
signature. If the verification fails, it throws an informative error:


{% highlight r %}
checkXPtr(ptr, type="double", args=c("int", "double")) # returns silently
checkXPtr(ptr, "int", c("int", "double"))
{% endhighlight %}



<pre class="output">
Error in checkXPtr(ptr, &quot;int&quot;, c(&quot;int&quot;, &quot;double&quot;)): Bad XPtr signature:
  Wrong return type 'int', should be 'double'.
</pre>



{% highlight r %}
checkXPtr(ptr, "int", c("int"))
{% endhighlight %}



<pre class="output">
Error in checkXPtr(ptr, &quot;int&quot;, c(&quot;int&quot;)): Bad XPtr signature:
  Wrong return type 'int', should be 'double'.
  Wrong number of arguments, should be 2'.
</pre>



{% highlight r %}
checkXPtr(ptr, "int", c("double", "std::string"))
{% endhighlight %}



<pre class="output">
Error in checkXPtr(ptr, &quot;int&quot;, c(&quot;double&quot;, &quot;std::string&quot;)): Bad XPtr signature:
  Wrong return type 'int', should be 'double'.
  Wrong argument type 'double', should be 'int'.
  Wrong argument type 'std::string', should be 'double'.
</pre>

## Complete use case

First, let us define a templated C++ backend that performs some
processing with a user-supplied function and a couple of adapters:


{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

template <typename T>
NumericVector core_processing(T func, double l) {
  double accum = 0;
  for (int i=0; i<1e3; i++)
    accum += sum(as<NumericVector>(func(3, l)));
  return NumericVector(1, accum);
}

// [[Rcpp::export]]
NumericVector execute_r(Function func, double l) {
  return core_processing<Function>(func, l);
}

typedef SEXP (*funcPtr)(int, double);

// [[Rcpp::export]]
NumericVector execute_cpp(SEXP func_, double l) {
  funcPtr func = *XPtr<funcPtr>(func_);
  return core_processing<funcPtr>(func, l);
}
{% endhighlight %}

Note that the user-supplied function takes two arguments: one is also
user-provided and the other is provided by the backend itself. This
core is exposed through the following R function:


{% highlight r %}
execute <- function(func, l) {
  stopifnot(is.numeric(l))
  if (is.function(func))
    execute_r(func, l)
  else {
    checkXPtr(func, "SEXP", c("int", "double"))
    execute_cpp(func, l)
  }
}
{% endhighlight %}

Finally, we can compare the `XPtr` approach with a pure R-based one,
and with a compiled function wrapped in R, as returned by
`Rcpp::cppFunction`:


{% highlight r %}
func_r <- function(n, l) rexp(n, l)
cpp <- "SEXP foo(int n, double l) { return rexp(n, l); }"
func_r_cpp <- Rcpp::cppFunction(cpp)
func_cpp <- cppXPtr(cpp)

microbenchmark::microbenchmark(
  execute(func_r, 1.5),
  execute(func_r_cpp, 1.5),
  execute(func_cpp, 1.5)
)
{% endhighlight %}



<pre class="output">
Unit: microseconds
                     expr       min        lq       mean     median
     execute(func_r, 1.5) 14364.442 14539.147 15269.3730 14679.8990
 execute(func_r_cpp, 1.5) 13738.891 13981.898 14783.3359 14290.0630
   execute(func_cpp, 1.5)   273.006   289.839   377.2792   356.6705
         uq       max neval
 16158.6100 19749.851   100
 15710.1205 18902.110   100
   421.6235  2095.696   100
</pre>
