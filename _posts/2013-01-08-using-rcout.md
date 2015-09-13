---
title: Using Rcout for output synchronised with R
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics featured
summary: This post shows how to use Rcout (and Rcerr) for output
layout: post
src: 2013-01-08-using-rcout.cpp
---

The [Writing R Extensions](http://cran.r-project.org/doc/manuals/R-exts.html)
manual, which provides the gold standard of documentation as far as
extending R goes, strongly suggests to use `Rprintf` and `REprintf`
for output (from C/C++ code). The key reason is that these are
matched to the usual output and error streams maintained by R
itself.

In fact, use of `std::cout` and `std::cerr` (as common in standard
C++ code) is flagged when running `R CMD check` and no longer
permitted when uploading to CRAN.

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

During the 0.10.* abd 0.11.* releases, Rcpp itself still lacked the
converter code to "pretty-print simple non-scalar data
structures. But there always were alternatives. First,
RcppArmadillo can do so via its `operator<<()` as the
(Rcpp)Armadillo output is automatically redirected to R output
stream.  See below for recent alternatives from Rcpp itself.

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
Rcpp basic objects. 

Alternatively, starting with R version 0.11.5, we now have function
`print()` which can print any `SEXP` object -- by calling the internal R
function `Rf_PrintValue()`.

A simple illustration follow. We first define helper function.

{% highlight cpp %}
// [[Rcpp::export]]
void callPrint(RObject x) { 
    Rcpp::print(x);             // will work on any SEXP object
}
{% endhighlight %}

A few examples calls follow below.

{% highlight r %}
callPrint(1:3)             # print a simple vector
{% endhighlight %}



<pre class="output">
[1] 1 2 3
</pre>



{% highlight r %}
callPrint(LETTERS[1:3])    # or characters
{% endhighlight %}



<pre class="output">
[1] &quot;A&quot; &quot;B&quot; &quot;C&quot;
</pre>



{% highlight r %}
callPrint(matrix(1:9,3))   # or a matrix
{% endhighlight %}



<pre class="output">
     [,1] [,2] [,3]
[1,]    1    4    7
[2,]    2    5    8
[3,]    3    6    9
</pre>



{% highlight r %}
callPrint(globalenv())    # or an environment object
{% endhighlight %}



<pre class="output">
&lt;environment: R_GlobalEnv&gt;
</pre>

Starting with version 0.12.1 of Rcpp, the `operator<<()` is also suppported for
vector and matrix types. See below for some examples.

{% highlight cpp %}
// [[Rcpp::export]]
void useOperatorOnVector(NumericVector x) { 
    Rcpp::Rcout << x << std::endl;
}

// [[Rcpp::export]]
void useOperatorOnMatrix(NumericMatrix x) { 
    Rcpp::Rcout << x << std::endl;
}
{% endhighlight %}

{% highlight r %}
v <- seq(0.0, 10.0, by=2.5)
useOperatorOnVector(v)
{% endhighlight %}



<pre class="output">
0 2.5 5 7.5 10
</pre>



{% highlight r %}
M <- matrix(seq(1.0, 16.0, by=1.0), 4, 4)
useOperatorOnMatrix(M)
{% endhighlight %}



<pre class="output">
      1       5       9      13
      2       6      10      14
      3       7      11      15
      4       8      12      16
</pre>
