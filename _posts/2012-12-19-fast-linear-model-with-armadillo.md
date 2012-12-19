---
title: Fast Linear Models with Armadillo
author: Dirk Eddelbuettel
license: MIT
tags: modeling armadillo 
summary: A simple implementation of of a fast linear regression using
  the Armadillo library
layout: post
src: 2012-12-19-fast-linear-model-with-armadillo.cpp
---
[Armadillo](http://arma.sourceforge.net/) is a C++ linear algebra library 
aiming towards a good balance between speed and ease of use. Integer, 
floating point and complex numbers are supported, as well as a subset of 
trigonometric and statistics functions. Various matrix decompositions are 
provided through optional integration with LAPACK and ATLAS libraries. A 
delayed evaluation approach is employed (during compile time) to combine 
several operations into one and reduce (or eliminate) the need for 
temporaries. This is accomplished through recursive templates and template 
meta-programming.

Here is a simple implementation of a fast linear regression (note this is 
also provided by the
[RcppArmadillo](http://cran.r-project.org/web/packages/RcppArmadillo/)
package via the `fastLm()` function):

{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]

#include <RcppArmadillo.h>
using namespace Rcpp;

// [[Rcpp::export]]
List fastLm(NumericVector yr, NumericMatrix Xr) {

   int n = Xr.nrow(), k = Xr.ncol();
   
   arma::mat X(Xr.begin(), n, k, false); 
   arma::colvec y(yr.begin(), yr.size(), false);
   
   arma::colvec coef = arma::solve(X, y); 
   arma::colvec resid = y - X*coef; 
   
   double sig2 = arma::as_scalar(arma::trans(resid)*resid/(n-k));
   arma::colvec stderrest = arma::sqrt(
       sig2 * arma::diagvec( arma::inv(arma::trans(X)*X)) );
   
   return List::create(Named("coefficients") = coef,
                       Named("stderr")       = stderrest);
}
{% endhighlight %}

