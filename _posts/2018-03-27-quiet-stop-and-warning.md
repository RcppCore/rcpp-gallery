---
title: Suppressing Call Stack Info in Rcpp-Generated Errors and Warnings
author: Michael Weylandt
license: GPL (>= 2)
tags: basics c++11
date: 2018-03-27
summary: We show how to achieve the equivalent of `stop(..., call.=FALSE)` or
         `warning(..., call. = FALSE) in `Rcpp` code. 
layout: post
src: 2018-03-27-quiet-stop-and-warning.Rmd
---

### Introduction

`Rcpp` has an elegant [mechanism of exception handling](http://gallery.rcpp.org/articles/intro-to-exceptions/)
whereby `C++` exceptions are automatically translated to errors in `R`. For most
projects, the `Rcpp::stop` wrapper (in conjunction with the `BEGIN_RCPP` and 
`END_RCPP` macros automatically inserted by 
[`RcppAttributes`](https://cran.r-project.org/web/packages/Rcpp/vignettes/Rcpp-attributes.pdf))
is sufficient and easy to use, providing an `Rcpp` equivalent of `base::stop`. 

By default, it captures the call stack and attaches it to the exception
in `R`, giving informative error messages: 


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

//[[Rcpp::export]]         
NumericVector add1(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        stop("x and y are not the same length!");
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add1(1:5, 1:3)
{% endhighlight %}



<pre class="output">
Error in add1(1:5, 1:3): x and y are not the same length!
</pre>

This matches the default behavior of `base::stop()` which captures the call info. 

For complex calling patterns (*e.g.*, creating an argument list and calling the
`Rcpp` function with `do.call`), the resulting error messages are less helpful: 


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

// [[Rcpp::export]]
NumericVector internal_function_name(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        stop("x and y are not the same length!");
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add2 <- function(x, y){
    if(!is.numeric(x)){
        x <- as.numeric(x)
    }

    do.call(internal_function_name, list(x, y))
}

add2(1:5, 1:3)
{% endhighlight %}



<pre class="output">
Error in (function (x, y) : x and y are not the same length!
</pre>

If the internal error were being generated in `R` code, we might choose to use
the `call.=FALSE` argument to `base::stop` to suppress the unhelpful `(function (x, y)`
part of the error message, but we don't (immediately) have a corresponding option
in `Rcpp`. In this gallery post, we show how to suppress the call-stack capture
of `Rcpp::stop` to give cleaner error messages. 

### Error Messages

The key functionality was added to `Rcpp` by [Jim Hester](https://github.com/jimhester) in 
[Rcpp Pull Request #663](https://github.com/RcppCore/Rcpp/pull/663/files). 
To generate an `R`-level exception without a call stack, we pass an optional
`false` flag to `Rcpp::exception`. For example, 


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

// [[Rcpp::export]]
NumericVector internal_function_name2(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        throw Rcpp::exception("x and y are not the same length!", false);
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add3 <- function(x, y){
    if(!is.numeric(x)){
        x <- as.numeric(x)
    }

    do.call(internal_function_name2, list(x, y))
}

add3(1:5, 1:3)
{% endhighlight %}



<pre class="output">
Error: x and y are not the same length!
</pre>

This can't capture the `R` level call stack, but it is at least cleaner than
the error message from the previous example. 

Note that here, as elsewhere in `C++`, we need to handle exceptions using a 
`try/catch` structure, but we do not add it explicitly because
[`RcppAttributes`](https://cran.r-project.org/web/packages/Rcpp/vignettes/Rcpp-attributes.pdf)
automatically handles this for us. 

### Warnings

Similar to `Rcpp::stop`, `Rcpp` also provides a `warning` function to generate 
`R` level warnings. It has the same call-stack capture behavior as `stop`. 

For the direct call case:


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

//[[Rcpp::export]]         
NumericVector add4(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        warning("x and y are not the same length!");
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add4(1:5, 1:3)
{% endhighlight %}



<pre class="output">
Warning in add4(1:5, 1:3): x and y are not the same length!
</pre>



<pre class="output">
[1] 2 4 6 4 5
</pre>

For the indirect call case: 


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

// [[Rcpp::export]]
NumericVector internal_function_name3(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        warning("x and y are not the same length!");
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add5 <- function(x, y){
    if(!is.numeric(x)){
        x <- as.numeric(x)
    }

    do.call(internal_function_name3, list(x, y))
}

add5(1:5, 1:3)
{% endhighlight %}



<pre class="output">
Warning in (function (x, y) : x and y are not the same length!
</pre>



<pre class="output">
[1] 2 4 6 4 5
</pre>

If we want to suppress the call stack info in this warning, we have to drop
down to the `C`-level `R` API. In particular, we use the `Rf_warningcall` function,
which takes the call as the first argument. By passing a `NULL`, we suppress the call: 


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

// [[Rcpp::export]]
NumericVector internal_function_name5(NumericVector x, NumericVector y){
    if(x.size() != y.size()){
        Rf_warningcall(R_NilValue, "x and y are not the same length!");
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add6 <- function(x, y){
    if(!is.numeric(x)){
        x <- as.numeric(x)
    }

    do.call(internal_function_name5, list(x, y))
}

add6(1:5, 1)
{% endhighlight %}



<pre class="output">
Warning: x and y are not the same length!
</pre>



<pre class="output">
[1] 2 2 3 4 5
</pre>

## A C++11 Implementation

The above methods work, but they are not as clean as their `Rcpp::stop` and
`Rcpp::warning` counterparts. We can take advantage of `C++11` to provide 
similar functionality for our call-free versions. 

Basing our implementation on the [`C++11` implementation](https://github.com/RcppCore/Rcpp/blob/master/inst/include/Rcpp/exceptions/cpp11/exceptions.h)
of `Rcpp::stop` and `Rcpp::warning` we can define our own
`stopNoCall` and `warningNoCall`


{% highlight cpp %}
#include "Rcpp.h"
using namespace Rcpp; 

// [[Rcpp::plugins(cpp11)]]
template <typename... Args>
inline void warningNoCall(const char* fmt, Args&&... args ) {
    Rf_warningcall(R_NilValue, tfm::format(fmt, std::forward<Args>(args)... ).c_str());
}

template <typename... Args>
inline void NORET stopNoCall(const char* fmt, Args&&... args) {
    throw Rcpp::exception(tfm::format(fmt, std::forward<Args>(args)... ).c_str(), false);
}

// [[Rcpp::export]]
NumericVector internal_function_name6(NumericVector x, NumericVector y, bool warn){
    if(x.size() != y.size()){
        if(warn){
            warningNoCall("x and y are not the same length!");  
        } else {
            stopNoCall("x and y are not the same length!");
        }
        
    }
    return x + y; 
}
{% endhighlight %}


{% highlight r %}
add7 <- function(x, y, warn=TRUE){
    if(!is.numeric(x)){
        x <- as.numeric(x)
    }

    do.call(internal_function_name6, list(x, y, warn))
}
{% endhighlight %}


{% highlight r %}
add7(1:5, 1:3, warn=TRUE)
{% endhighlight %}



<pre class="output">
Warning: x and y are not the same length!
</pre>



<pre class="output">
[1] 2 4 6 4 5
</pre>


{% highlight r %}
add7(1:5, 1:3, warn=FALSE)
{% endhighlight %}



<pre class="output">
Error: x and y are not the same length!
</pre>

Note that we used `C++11` variadic templates here -- if we wanted to do something
similar in `C++98,` we could use essentially the same pattern, but would need to
implement each case individually. 
