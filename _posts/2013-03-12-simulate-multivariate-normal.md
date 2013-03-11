---
title: Generating a multivariate gaussian distribution using RcppArmadillo
author: Ahmadou Dicko
license: GPL (>= 2)
tags: matrix armadillo random number
summary: Demonstrate how to sample from a multivariate gaussian using a Cholesky decomposition
---

There are many ways to simulate a multivariate gaussian distribution assuming that you can simulate from independent univariate normal distributions. 
One of the most popular method is based on the [Cholesky decomposition][1].
Let's see how `Rcpp` and `Armadillo` perform on this task.



```cpp
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]

using namespace arma;
using namespace Rcpp;

mat mvrnormArma(int n, vec mu, mat sigma) {
   int ncols = sigma.n_cols;
   mat Y = randn(n, ncols);
   return repmat(mu, 1, n).t() + Y * chol(sigma);
}
```



The easiest way to perform a Cholesky distribution in R is to use  the ``chol`` function in `R` which interface some fast `LAPACK` routines.



```r
### naive implementation in R
mvrnormR <- function(n, mu, sigma) {
    ncols <- ncol(sigma)
    mu <- rep(mu, each = n)  ## not obliged to use a matrix (recycling)
    mu + matrix(rnorm(n * ncols), ncol = ncols) %*% chol(sigma)
}
```


We will also use ``MASS:mvrnorm`` which implemented it differently


```r
require(MASS)
### Covariance matrix and mean vector
sigma <- matrix(c(1, 0.9, -0.3, 0.9, 1, -0.4, -0.3, -0.4, 1), ncol = 3)
mu <- c(10, 5, -3)

require(MASS)
### checking variance
set.seed(123)
cor(mvrnormR(100, mu, sigma))
```

```
##         [,1]    [,2]    [,3]
## [1,]  1.0000  0.8851 -0.3830
## [2,]  0.8851  1.0000 -0.4675
## [3,] -0.3830 -0.4675  1.0000
```

```r
cor(MASS::mvrnorm(100, mu, sigma))
```

```
##         [,1]    [,2]    [,3]
## [1,]  1.0000  0.9106 -0.3016
## [2,]  0.9106  1.0000 -0.4599
## [3,] -0.3016 -0.4599  1.0000
```

```r
cor(mvrnormArma(100, mu, sigma))
```

```
##         [,1]    [,2]    [,3]
## [1,]  1.0000  0.9098 -0.3174
## [2,]  0.9098  1.0000 -0.4194
## [3,] -0.3174 -0.4194  1.0000
```

```r

## checking means
colMeans(mvrnormR(100, mu, sigma))
```

```
## [1]  9.850  4.911 -2.902
```

```r
colMeans(MASS::mvrnorm(100, mu, sigma))
```

```
## [1] 10.051  5.046 -2.914
```

```r
colMeans(mvrnormArma(100, mu, sigma))
```

```
## [1] 10.210  5.211 -3.095
```


Now, let's benchmark the different versions


```r
require(rbenchmark)
```

```
## Loading required package: rbenchmark
```

```r
benchmark(mvrnormR(10000, mu, sigma), 
 	      MASS::mvrnorm(10000, mu, sigma), 
          mvrnormArma(10000, mu, sigma), 
	      columns = c("test", "replications", "relative", "elapsed"), 
          order = "elapsed")
```

```
##                              test replications relative elapsed
## 3   mvrnormArma(10000, mu, sigma)          100    1.000   0.592
## 1      mvrnormR(10000, mu, sigma)          100    2.054   1.216
## 2 MASS::mvrnorm(10000, mu, sigma)          100    2.262   1.339
```


The ``RcppArmadillo`` function outperforms the MASS implementation and the naive R code, but more surprisinugly ``mvrnormR`` is slightly faster than ``mvrnorm`` in this benchmark.

To be fair, while digging into the ``MASS::mvrnorm`` code it appears that there are few code sanity checks ( such as the positive definiteness  of `Sigma` ).



[1]: http://en.wikipedia.org/wiki/Cholesky_decomposition
