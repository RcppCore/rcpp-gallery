---
title: Passing user-supplied C++ functions
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo featured
summary: This example shows how to select user-supplied C++ functions
layout: post
src: 2013-01-21-passing-cpp-function-pointers.cpp
---
Baptiste [asked on StackOverflow about letting users supply C++ functions](http://stackoverflow.com/questions/14428687/rcpparmadillo-pass-user-defined-function)
for use with Armadillo / RcppArmadillo.  This posts helps with an
extended answer. There is nothing specific about Armadillo here,
this would the same way with Eigen, the GSL or any other library a
user wants to support (and provides his or her own `as<>()` and
`wrap()` converters which we already have for Armadillo, Eigen and
the GSL).

To set the stage, let us consider two simple functions of a vector 



{% highlight rcpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

using namespace arma; 
using namespace Rcpp;

vec fun1_cpp(const vec& x) {	// a first function 
    vec y = x + x;
    return (y);
}

vec fun2_cpp(const vec& x) {	// and a second function
    vec y = 10*x;
    return (y);
}
{% endhighlight %}

These are pretty boring and standard functions, and we could simple
switch between them via if/else statements.  Where it gets
interesting is via the `SEXP` wrapping offered by `XPtr` below. 

But before we get there, let us do this one step at a time.

This typedef is important and just says that `funcPtr` will take a
const reference to a vec and return a vector -- just like our two
functions above

{% highlight rcpp %}
typedef vec (*funcPtr)(const vec& x);
{% endhighlight %}

The following function takes a string argument, picks a function and returns it 
wrapped as an external pointer `SEXP`.  We could return this to R as well. 

{% highlight rcpp %}
// [[Rcpp::export]]
XPtr<funcPtr> putFunPtrInXPtr(std::string fstr) {
    if (fstr == "fun1")
        return(XPtr<funcPtr>(new funcPtr(&fun1_cpp)));
    else if (fstr == "fun2")
        return(XPtr<funcPtr>(new funcPtr(&fun2_cpp)));
    else
        return XPtr<funcPtr>(R_NilValue); // runtime error as NULL no XPtr
}
{% endhighlight %}

A simple test of this function follows. First a function using it:

{% highlight rcpp %}
// [[Rcpp::export]]
vec callViaString(const vec x, std::string funname) {
    XPtr<funcPtr> xpfun = putFunPtrInXPtr(funname);
    funcPtr fun = *xpfun;
    vec y = fun(x);
    return (y);
}
{% endhighlight %}

And then a call, showing access to both functions:

{% highlight r %}
callViaString(1:3, "fun1")
{% endhighlight %}



<pre class="output">
     [,1]
[1,]    2
[2,]    4
[3,]    6
</pre>



{% highlight r %}
callViaString(1:3, "fun2")
{% endhighlight %}



<pre class="output">
     [,1]
[1,]   10
[2,]   20
[3,]   30
</pre>

But more interestingly, we can also receive a function pointer via the `SEXP` wrapping:

{% highlight r %}
fun <- putFunPtrInXPtr("fun1")
{% endhighlight %}

And use it in this function which no longer switches:

{% highlight rcpp %}
// [[Rcpp::export]]
vec callViaXPtr(const vec x, SEXP xpsexp) {
    XPtr<funcPtr> xpfun(xpsexp);
    funcPtr fun = *xpfun;
    vec y = fun(x);
    return (y);
}
{% endhighlight %}

As seen here:

{% highlight r %}
callViaXPtr(1:4, fun)
{% endhighlight %}



<pre class="output">
     [,1]
[1,]    2
[2,]    4
[3,]    6
[4,]    8
</pre>

This is a reasonably powerful and generic framework offered by Rcpp
and sitting on top of R's external pointers.
