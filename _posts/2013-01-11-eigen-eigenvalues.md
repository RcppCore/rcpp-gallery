---
title: Using Eigen for eigenvalues
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: eigen matrix
summary: This example shows how to compute eigenvalues using Eigen
layout: post
src: 2013-01-11-eigen-eigenvalues.cpp
---

[A previous post](../armadillo-eigenvalues) showed how to compute 
eigenvalues using the [Armadillo](http://arma.sf.net) library via RcppArmadillo.

Here, we do the same using [Eigen](http://eigen.tuxfamily.org) and
the RcppEigen package.




{% highlight cpp %}
#include <RcppEigen.h>

// [[Rcpp::depends(RcppEigen)]]

using Eigen::Map;               	// 'maps' rather than copies 
using Eigen::MatrixXd;                  // variable size matrix, double precision
using Eigen::VectorXd;                  // variable size vector, double precision
using Eigen::SelfAdjointEigenSolver;    // one of the eigenvalue solvers

// [[Rcpp::export]]
VectorXd getEigenValues(Map<MatrixXd> M) {
    SelfAdjointEigenSolver<MatrixXd> es(M);
    return es.eigenvalues();
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
[1]  0.3319  1.6856  2.4099 14.2100
</pre>


In comparison, R gets the same results (in reverse order) and also returns the eigenvectors.

{% highlight r %}
eigen(Z)
{% endhighlight %}



<pre class="output">
$values
[1] 14.2100  2.4099  1.6856  0.3319

$vectors
         [,1]     [,2]    [,3]     [,4]
[1,]  0.69988 -0.55799  0.4458 -0.00627
[2,] -0.06833 -0.08433  0.0157  0.99397
[3,]  0.44100 -0.15334 -0.8838  0.03127
[4,]  0.55769  0.81118  0.1413  0.10493
</pre>


Eigen has other _a lot_ of other decompositions, see [its documentation](http://eigen.tuxfamily.org/) 
for more details.
