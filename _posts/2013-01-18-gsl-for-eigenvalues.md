---
title: Using the GSL to compute eigenvalues
author: Dirk Eddelbuettel
updated: Nov 23, 2015
license: GPL (>= 2)
tags: modeling gsl
summary: This example shows how to call a GSL function using RcppGSL
layout: post
src: 2013-01-18-gsl-for-eigenvalues.cpp
---
Two previous posts showed how to compute eigenvalues 
[using Armadillo](../armadillo-eigenvalues) and 
[using Eigen](../eigen-eigenvalues/). As we have also looked at using the  
[GNU GSL](http://www.gnu.org/software/gsl/), this post will show how to
conpute eigenvalues using GSL.

As mentioned in the [previous GSL post](../gsl-colnorm-example), we
instantiate C language pointers suitable for GSL (here the matrix
`M`). Prior to release 0.3.0 of RcppGSL, these *had to be freed manually*. 
However, since release 0.3.0 since is now taken care of via the standard
C++ mechanism of destructors.


{% highlight cpp %}
// Tell Rcpp to rely on the RcppGSL package to find GSL library and headers
// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>

// [[Rcpp::export]]
RcppGSL::Vector getEigenValues(RcppGSL::Matrix & M) {
    int k = M.ncol();

    RcppGSL::Vector ev(k);  	// instead of gsl_vector_alloc(k);
    gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(k);
    gsl_eigen_symm(M, ev, w);
    gsl_eigen_symm_free (w);

    return ev;				// return results vector  
}
{% endhighlight %}

We can illustrate this easily via a random sample matrix.

{% highlight r %}
set.seed(42)
X <- matrix(rnorm(4*4), 4, 4)
Z <- X %*% t(X)

getEigenValues(Z)
{% endhighlight %}



<pre class="output">
[1] 14.210011  2.409920  1.685588  0.331887
</pre>

In comparison, R gets the same results (in reverse order) and also returns the eigenvectors.

{% highlight r %}
eigen(Z)
{% endhighlight %}



<pre class="output">
$values
[1] 14.210011  2.409920  1.685588  0.331887

$vectors
           [,1]       [,2]       [,3]        [,4]
[1,]  0.6998802 -0.5579950  0.4458363 -0.00626959
[2,] -0.0683337 -0.0843295  0.0157046  0.99396801
[3,]  0.4409966 -0.1533407 -0.8837594  0.03127147
[4,]  0.5576919  0.8111823  0.1412537  0.10493036
</pre>
