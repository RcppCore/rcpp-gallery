---
title: Armadillo eigenvalues
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo matrix featured
summary: This example shows how to compute eigenvalues easily
layout: post
src: 2013-01-04-armadillo-eigenvalues.cpp
---

Today a (slightly confused) question on 
[StackOverflow](http://stackoverflow.com/questions/14164972/eigenvalues-calculations-in-c-within-r-codes/14165455) 
wondered how to access R's facilities for eigenvalues calculations
from C code.

For this, we need to step back and consider how this is done. In
fact, R farms the calculation out to the BLAS. One could possibly
access R's functions---but would then have to wrestle with the data
input/output issues which make Rcpp shine in comparison.  Also,
Rcpp gets us access to Armadillo (via the RcppArmadillo) package
and Armadillo's main focus are exactly the linear algebra
calculations and decompositions.  

And with facilities that were added to Rcpp in the 0.10.* release
series, this effectively becomes a one-liner of code! (Nitpickers
will note that there are also one include statement, two attributes
declarations and the function name itself.)


{% highlight rcpp %}
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
arma::vec getEigenValues(arma::mat M) {
    return arma::eig_sym(M);
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
          [,1]
[1,]  0.331887
[2,]  1.685588
[3,]  2.409920
[4,] 14.210011
</pre>

In comparison, R gets the same results (in reverse order) and also returns the eigenvectors.

{% highlight r %}
eigen(Z)
{% endhighlight %}



<pre class="output">
eigen() decomposition
$values
[1] 14.210011  2.409920  1.685588  0.331887

$vectors
           [,1]       [,2]       [,3]        [,4]
[1,]  0.6998802  0.5579950  0.4458363 -0.00626959
[2,] -0.0683337  0.0843295  0.0157046  0.99396801
[3,]  0.4409966  0.1533407 -0.8837594  0.03127147
[4,]  0.5576919 -0.8111823  0.1412537  0.10493036
</pre>

Armadillo has other eigenvector computations too, see [its documentation](http://arma.sourceforge.net/docs.html#eig_sym).
