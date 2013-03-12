---
title: Generating a multivariate gaussian distribution using RcppArmadillo
author: Ahmadou Dicko
license: GPL (>= 2)
tags: matrix armadillo random number
summary: Demonstrate how to sample from a multivariate gaussian using a Cholesky decomposition
layout: post
src: 2013-03-12-simulate-multivariate-normal.Rmd
---

There are many ways to simulate a multivariate gaussian distribution assuming that you can simulate from independent univariate normal distributions. 
One of the most popular method is based on the [Cholesky decomposition][1].
Let's see how `Rcpp` and `Armadillo` perform on this task.



{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;

// [[Rcpp::export]]
arma::mat mvrnormArma(int n, arma::vec mu, arma::mat sigma) {
   int ncols = sigma.n_cols;
   arma::mat Y = arma::randn(n, ncols);
   return arma::repmat(mu, 1, n).t() + Y * arma::chol(sigma);
}
{% endhighlight %}



The easiest way to perform a Cholesky distribution in R is to use  the ``chol`` function in `R` which interface some fast `LAPACK` routines.



{% highlight r %}
### naive implementation in R
mvrnormR <- function(n, mu, sigma) {
    ncols <- ncol(sigma)
    mu <- rep(mu, each = n) ## not obliged to use a matrix (recycling)
    mu + matrix(rnorm(n * ncols), ncol = ncols) %*% chol(sigma)
}
{% endhighlight %}


We will also use ``MASS:mvrnorm`` which implemented it differently


{% highlight r %}
require(MASS)
### Covariance matrix and mean vector
sigma <- matrix(c(1, 0.9, -0.3, 0.9, 1, -0.4, -0.3, -0.4, 1), ncol = 3)
mu <- c(10, 5, -3)

require(MASS)
### checking variance
set.seed(123)
cor(mvrnormR(100, mu,  sigma))
{% endhighlight %}



<pre class="output">
        [,1]    [,2]    [,3]
[1,]  1.0000  0.8851 -0.3830
[2,]  0.8851  1.0000 -0.4675
[3,] -0.3830 -0.4675  1.0000
</pre>



{% highlight r %}
cor(MASS::mvrnorm(100, mu, sigma))
{% endhighlight %}



<pre class="output">
        [,1]    [,2]    [,3]
[1,]  1.0000  0.9106 -0.3016
[2,]  0.9106  1.0000 -0.4599
[3,] -0.3016 -0.4599  1.0000
</pre>



{% highlight r %}
cor(mvrnormArma(100, mu, sigma))
{% endhighlight %}



<pre class="output">
        [,1]    [,2]    [,3]
[1,]  1.0000  0.9103 -0.4405
[2,]  0.9103  1.0000 -0.4880
[3,] -0.4405 -0.4880  1.0000
</pre>



{% highlight r %}

## checking means
colMeans(mvrnormR(100, mu, sigma))
{% endhighlight %}



<pre class="output">
[1]  9.850  4.911 -2.902
</pre>



{% highlight r %}
colMeans(MASS::mvrnorm(100, mu, sigma))
{% endhighlight %}



<pre class="output">
[1] 10.051  5.046 -2.914
</pre>



{% highlight r %}
colMeans(mvrnormArma(100, mu, sigma))
{% endhighlight %}



<pre class="output">
[1]  9.987  5.091 -2.907
</pre>


Now, let's benchmark the different versions


{% highlight r %}
require(rbenchmark)
{% endhighlight %}



<pre class="output">
Loading required package: rbenchmark
</pre>



{% highlight r %}
benchmark(mvrnormR(1e4, mu, sigma),
          MASS::mvrnorm(1e4, mu, sigma),
          mvrnormArma(1e4, mu, sigma),
          columns = c('test', 'replications', 'relative', 'elapsed'),
          order = 'elapsed')
{% endhighlight %}



<pre class="output">
                             test replications relative elapsed
3   mvrnormArma(10000, mu, sigma)          100    1.000   0.651
1      mvrnormR(10000, mu, sigma)          100    2.197   1.430
2 MASS::mvrnorm(10000, mu, sigma)          100    3.281   2.136
</pre>


The ``RcppArmadillo`` function outperforms the MASS implementation and the naive R code, but more surprisinugly ``mvrnormR`` is slightly faster than ``mvrnorm`` in this benchmark.

To be fair, while digging into the ``MASS::mvrnorm`` code it appears that there are few code sanity checks ( such as the positive definiteness  of `Sigma` ).



[1]: http://en.wikipedia.org/wiki/Cholesky_decomposition
