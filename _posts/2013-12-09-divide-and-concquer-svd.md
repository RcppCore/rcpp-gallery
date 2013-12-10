---
title: Performance of the divide-and-conquer SVD algorithm
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo matrix
summary: This example illustrates the performance gain from divide-and-conquer SVD
layout: post
src: 2013-12-09-divide-and-concquer-svd.cpp
---
The ubiquitous [LAPACK](http://www.netlib.org/lapack) library provides several 
implementations for the singular-value decomposition (SVD). We will illustrate 
possible speed gains from using the divide-and-conquer method by comparing it
to the base case.



{% highlight cpp %}
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
arma::vec baseSVD(const arma::mat & X) {
    arma::mat U, V;
    arma::vec S;
    arma::svd(U, S, V, X, "standard");
    return S;
}

// [[Rcpp::export]]
arma::vec dcSVD(const arma::mat & X) {
    arma::mat U, V;
    arma::vec S;
    arma::svd(U, S, V, X, "dc");
    return S;
}
{% endhighlight %}


Having the two implementations, which differ only in the `method`
argument (added recently in Armadillo 3.930), we are ready to do a
simple timing comparison.

{% highlight r %}
library(microbenchmark)
set.seed(42)
X <- matrix(rnorm(16e4), 4e2, 4e2)
microbenchmark(baseSVD(X), dcSVD(X))
{% endhighlight %}



<pre class="output">
Unit: milliseconds
       expr   min    lq median    uq   max neval
 baseSVD(X) 421.2 422.6  424.2 426.2 442.1   100
   dcSVD(X) 111.0 111.5  111.9 113.6 126.1   100
</pre>


The speed gain is noticeable which a ratio of about 3.9 at the
median. However, we can also look at complex-valued matrices.

{% highlight cpp %}
// [[Rcpp::export]]
arma::vec cxBaseSVD(const arma::cx_mat & X) {
    arma::cx_mat U, V;
    arma::vec S;
    arma::svd(U, S, V, X, "standard");
    return S;
}

// [[Rcpp::export]]
arma::vec cxDcSVD(const arma::cx_mat & X) {
    arma::cx_mat U, V;
    arma::vec S;
    arma::svd(U, S, V, X, "dc");
    return S;
}
{% endhighlight %}


{% highlight r %}
A <- matrix(rnorm(16e4), 4e2, 4e2)
B <- matrix(rnorm(16e4), 4e2, 4e2)
X <- A + 1i * B
microbenchmark(cxBaseSVD(X), cxDcSVD(X))
{% endhighlight %}



<pre class="output">
Unit: milliseconds
         expr    min     lq median     uq    max neval
 cxBaseSVD(X) 1248.7 1253.7 1257.5 1262.3 1311.7   100
   cxDcSVD(X)  259.2  259.8  260.5  263.2  327.9   100
</pre>


Here the difference is even more pronounced at about 4.8. However,
it is precisely this complex-value divide-and-conquer method which
is missing in R's own Lapack. So R built with the default
configuration can not currently take advantage of the
complex-valued divide-and-conquer algorithm. Only builds which use
an external Lapack library (as for example the Debian and Ubuntu
builds) can. Let's hope that R will add this functionality to its
next release R 3.1.0. <em>Update: And the underlying `zgesdd`
routine has now been added to the upcoming R 3.1.0 release. Nice.</em>
