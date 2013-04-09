---
title: Dynamic Wrapping and Recursion with Rcpp
author: Kevin Ushey
license: GPL (>= 2)
tags: basics recursion function
summary: We can use parts of R's API alongside Rcpp to recurse through 
  lists and dynamically wrap objects as needed.
layout: post
src: 2013-04-08-rcpp-wrap-and-recurse.Rmd
---
 
We can leverage small parts of the R's C API in order to
infer the type of objects directly at the run-time of a function call, and use
this information to dynamically wrap objects as needed. We'll also present an
example of recursing through a list.
 
To get a basic familiarity with the main functions exported from R API,
I recommend reading Hadley's guide to R's C internals guide 
[here](https://github.com/hadley/devtools/wiki/C-interface) 
first, as we will be using some of these functions for navigating
native R SEXPs. (Reading it will also give you an appreciation for just how much
work Rcpp does in insulating us from the ugliness of the R API.)

From the R API, we'll be using the `TYPEOF` macro, as well as referencing the
internal R types:
 
* `REALSXP` for numeric vectors,
* `INTSXP` for integer vectors,
* `VECSXP` for lists

We'll start with a simple example: an Rcpp function that takes a list,
loops through it, and:
 
* if we encounter a numeric vector, double each element in it;
* if we encounter an integer vector, add 1 to each element in it
 

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
List do_stuff( List x_ ) {
    List x = clone(x_);
    for( List::iterator it = x.begin(); it != x.end(); ++it ) {
        switch( TYPEOF(*it) ) {
            case REALSXP: {
                NumericVector tmp = as<NumericVector>(*it);
          	tmp = tmp * 2;
		break;    
            }
      	    case INTSXP: {
                if( Rf_isFactor(*it) ) break; // factors have internal type INTSXP too
        	IntegerVector tmp = as<IntegerVector>(*it);
		tmp = tmp + 1;
                break;
      	    }
      	    default: {
                stop("incompatible SEXP encountered; only accepts lists with REALSXPs and INTSXPs");
      	    }
       }
  }  
  return x;
}
{% endhighlight %}


A quick test:
 

{% highlight r %}
dat <- list( 
    1:5, ## integer
    as.numeric(1:5) ## numeric
)
tmp <- do_stuff(dat)
print(tmp)
{% endhighlight %}



<pre class="output">
[[1]]
[1] 2 3 4 5 6

[[2]]
[1]  2  4  6  8 10
</pre>

 
Some notes on the above:
 
1. We clone the list passed through to ensure we work with a copy, rather
than the original list passed in,
2. We switch over the internal R type using `TYPEOF`, and do something 
for the case of numeric vectors (`REALSXP`), and integer vectors (`INTSXP`),
3. After we've figured out what kind of object we have, we can use `Rcpp::as`
to wrap the R object with the appropriate container,
4. Because Rcpp's wrappers point to the internal R structures, any changes made
to them are reflected in the R object wrapped,
5. We use Rcpp sugar to easily add and multiply each element in a vector,
6. We throw an error if a non-numeric / non-integer object is encountered.
One could leave the `default:` switch just to do nothing or fall through,
or handle other `SEXP`s as needed as well.
  
We also check that we fail gracefully when we encounter a non-accepted `SEXP`:


{% highlight r %}
do_stuff( list(new.env()) )
{% endhighlight %}



<pre class="output">
Error: incompatible SEXP encountered; only accepts lists with REALSXPs and
INTSXPs
</pre>

  
However, this only operates on top-level objects within the list. What if your
list contains other lists, and you want to recurse through those lists as well?
 
It's actually quite simple: if the internal R type of the object encountered
is a `VECSXP`, then we just call our recursive function on that element itself!
 

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
List recurse(List x_) {
    List x = clone(x_);
    for( List::iterator it = x.begin(); it != x.end(); ++it ) {
        switch( TYPEOF(*it) ) {
            case VECSXP: {
                *it = recurse(*it);
        	break;
            }
            case REALSXP: {
                NumericVector tmp = as<NumericVector>(*it);
        	tmp = tmp * 2;
            	break;
      	    }
      	    case INTSXP: {
            	if( Rf_isFactor(*it) ) break; // factors have internal type INTSXP too
        	IntegerVector tmp = as<IntegerVector>(*it);
        	tmp = tmp + 1;
        	break;
      	    }
      	    default: {
                stop("incompatible SEXP encountered; only accepts lists containing lists, REALSXPs, and INTSXPs");
      	    }
        }
    }
    return x;
}
{% endhighlight %}


A test case:
 

{% highlight r %}
dat <- list( 
    x=1:5, ## integer
    y=as.numeric(1:5), ## numeric
    z=list( ## another list to recurse into
        zx=10L, ## integer
        zy=20 ## numeric
    )
)
out <- recurse(dat)
print(out)
{% endhighlight %}



<pre class="output">
$x
[1] 2 3 4 5 6

$y
[1]  2  4  6  8 10

$z
$z$zx
[1] 11

$z$zy
[1] 40
</pre>


Note that all we had to do was add a `VECSXP` case in our `switch` statement.
If we see a list, we call the same `recurse` function on that list, and then
re-assign the result of that recursive call. Neat!

Hence, by using `TYPEOF` to query the internal R type of objects pre-wrap, we
can wrap objects as needed into an appropriate container, and then use Rcpp
/ C++ code as necessary to modify them.
