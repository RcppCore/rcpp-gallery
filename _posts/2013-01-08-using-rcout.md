---
title: Using Rcout for output synchronised with R
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics featured
summary: This post shows how to use Rcout (and Rcerr) for output
layout: post
src: 2013-01-08-using-rcout.cpp
---

The [Writing R Extensions](http://cran.r-project.org/doc/manuals/R-exts.html) manual, 
which provides the gold standard of documentation as far as
extending R goes, suggests to use `Rprintf` and `REprintf` for
output (from C/C++ code) as these are matched to the usual output and error streams
maintained by R itself.

Also, use of `std::cout` and `std::cerr` (as common in standard C++ code) is flagged when
running `R CMD check` and no longer permitted when uploading to CRAN.

Thanks to an initial patch by Jelmer Ypma, which has since been
reworked and extended, we have devices `Rcout` (for standard
output) and `Rcerr` (for standard error) which intercept output and
redirect it to R.

To illustrate, we create a simple function which prints a value:



{% highlight cpp %}
#include <RcppArmadillo.h>   // as we use RcppArmadillo below
                             // this first example use only Rcpp 

using namespace Rcpp;

// [[Rcpp::export]]
void showValue(double x) {
    Rcout << "The value is " << x << std::endl;
}
{% endhighlight %}


We can use this from R, and output will be properly synchronised:

{% highlight r %}
cat("Before\n")
{% endhighlight %}



<pre class="output">
Before
</pre>



{% highlight r %}
showValue(1.23)
{% endhighlight %}



<pre class="output">
The value is 1.23
</pre>



{% highlight r %}
cat("After\n")
{% endhighlight %}



<pre class="output">
After
</pre>


As of the 0.10.* releases, Rcpp itself still lacks the converter code to
print simple non-scalar data structures---but RcppArmadillo can do
so as Conrad permitted a hool for us to supply the Rcout device as
the default device

{% highlight cpp %}
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
void showMatrix(arma::mat X) {
    Rcout << "Armadillo matrix is" << std::endl << X << std::endl;
}
{% endhighlight %}


{% highlight r %}
M <- matrix(1:9,3,3)
print(M)
{% endhighlight %}



<pre class="output">
     [,1] [,2] [,3]
[1,]    1    4    7
[2,]    2    5    8
[3,]    3    6    9
</pre>



{% highlight r %}
showMatrix(M)
{% endhighlight %}



<pre class="output">
Armadillo matrix is
   1.0000   4.0000   7.0000
   2.0000   5.0000   8.0000
   3.0000   6.0000   9.0000
</pre>


Having output from R and C++ mix effortlessly is a very useful
feature. We hope to over time add more features to output more of
Rcpp basic objects.  Patches are of course always welcome.
