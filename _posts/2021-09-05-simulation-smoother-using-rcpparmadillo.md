---
title: Simulation Smoother using RcppArmadillo
author: Tomasz Woźniak
license: GPL (>= 2)
tags: armadillo  
mathjax: true
summary: Demonstrates three RcppArmadillo implementations of sampling random draws from a multivariate normal distribution specified by a precision matrix of special types.
layout: post
src: 2021-09-05-simulation-smoother-using-rcpparmadillo.Rmd
---



Simulation smoothing involves drawing latent state variables in discrete-time state–space models from their conditional distribution given parameters and data as defined by [McCausland, Miller, & Pelletier (2011)](https://doi.org/10.1016/j.csda.2010.07.009). This tool greatly facilitates Bayesian estimation in the case of a popular family of conditionally linear Gaussian state-space models. This post presents **Rcpp** implementations of a sampling algorithm for a multivariate normal distribution. **RcppArmadillo** is used to  efficiently calculate the Cholesky decomposition and implement the back-substitution to solve linear equations, which can greatly improves computation times when using routines for banded or tridiagonal matrices.

## Precision sampler

Consider the problem of sampling random numbers from a multivariate normal distribution specified by [Chan & Jeliazkov (2009)](https://doi.org/10.1504/IJMMNO.2009.030090) as:
\begin{equation}\label{eq1}
\mathcal{N}_T\left(\Omega^{-1}\alpha, \Omega^{-1}\right)
\end{equation}
where:

- the covariance matrix, denoted by $\Omega^{-1}$, is a $T\times T$ symmetric positive-definite matrix given by the inverse of a precision matrix $\Omega$ that, depending on the autoregressive lag order of the state equation, is band diagonal or tridiagonal and easy to compute, and

- the mean of the distribution, denoted by $\Omega^{-1}\alpha$, is constructed as the product of the inverse of the precision matrix times an easy-to-compute $T$-vector $\alpha$.

Let $L$ be a lower-triangular matrix obtained by the Cholesky decomposition of the precision matrix $\Omega=LL'$. Consequently, the mean of the normal distribution above can be presented as $(LL')^{-1}\alpha = L^{-1\prime}L^{-1}\alpha$. Let a $T$-column vector $\epsilon$ contain independent random draws from the standard normal distribution. A random draw from the target distribution can be computed then by:
\begin{equation}\label{eq2}
L^{-1\prime}L^{-1}\alpha + L^{-1\prime}\epsilon = L^{-1\prime}\left(L^{-1}\alpha + \epsilon\right)
\end{equation}
The formulation of the problem on the right-hand side of the equation above follows the proposition of [McCausland, Miller, & Pelletier (2011)](https://doi.org/10.1016/j.csda.2010.07.009) and leads to the sampling algorithm following the steps:

1. Compute the Cholesky decomposition of matrix, $\Omega = LL'$. Use dedicated computational routines for band or tridiagonal matrices if possible.
2. Sample $T$ independent draws from the standard normal distribution and collect them in the column vector $\epsilon$.
3. Solve the linear equation $La = \alpha$ for $a$ using forward-substitution.
4. Solve the linear equation $L'X = a + \epsilon$ for $X$ using back-substitution.
5. Return $X$ as a draw from the target distribution.

The computational gains from applying the algorithm presented above are twofold. First, taking into account that matrix $\Omega$ is band or tridiagonal makes computing the Cholesky decomposition fast if dedicated routines are applied and leads to a lower-triangular matrix $L$ with as many non-zero subdiagonals as in the precision matrix. Secondly, solving the linear equations using back-substitution bypasses the computation of the inverse of $L$ and takes advantage of the special type of matrix $L$. This strategy greatly reduces the computational time required to obtain the random draws and, therefore, was recommended by [Frühwirth-Schnatter & Kastner (2014)](https://doi.org/10.1016/j.csda.2013.01.002) and [Bitto & Frühwirth-Schnatter (2019)](https://doi.org/10.1016/j.jeconom.2018.11.006) following the developments by [Chan & Jeliazkov (2009)](https://doi.org/10.1504/IJMMNO.2009.030090) and [McCausland, Miller, & Pelletier (2011)](https://doi.org/10.1016/j.csda.2010.07.009). 

## Three implementations in RcppArmadillo

All the presented solutions are designed to sample `n` draws from the target distribution and return them in columns of the output matrix `draw`. The results are required to be reproducible. Therefore, the compatibility with R's `set.seed` is obtained by generating normal random draws using `Rcpp::rnorm` rather than `arma::randn`. All the considered functions take three arguments: the number of requested draws `n`, the `precision` matrix, and the covector `location`. Then they read the dimension of the normal vector `T`, fill matrix `epsilon` with normal random draws, create the `location_matrix` of appropriate dimensions whose columns are filled with `location` using `each_col`, and proceed to the implementation of steps 3 and 4 from the sampling algorithm presented above.

The first function `rmvnorm_arma_inv` is based on the Cholesky decomposition computed by `chol` subsequently declared as an upper-triangular matrix using `trimatu`, and uses function `inv` to compute the inverse. This matrix is then used to compute the draws from the target distribution using matrix multiplication and summation.


{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;
using namespace arma;

// [[Rcpp::export]]
mat rmvnorm_arma_inv (int n, mat precision, vec location){
  
  int T = precision.n_rows;
  
  mat epsilon(T, n);
  for (int i=0; i<n; i++){
    epsilon.col(i) = as<vec>(rnorm(T));
  }
  mat location_matrix(T, n, fill::zeros);
  location_matrix.each_col() += location;
  
  mat precision_chol_inv = trans(inv(trimatu(chol(precision))));
  mat draw    = trans(precision_chol_inv) * (
    precision_chol_inv * location_matrix + epsilon
    );

  return draw;
}
{% endhighlight %}

The second function `rmvnorm_arma_solve` uses `chol` and `trimatu` the same way as the first one. However, this implementation uses `solve` searching for the solutions to the linear equations using back-substitution. Additionally, function `solve` checks if the matrix is band and applies dedicated numerical procedures as explained in the [API Documentation for Armadillo 10.6](http://arma.sourceforge.net/docs.html). This additional check facilitates the computational gains. The development of the two functions considered thus far was somehow inspired by the RcppGallery article by [Hardt, Ahmadou, & Christoffersen (2013-2020)](https://gallery.rcpp.org/articles/dmvnorm_arma/).


{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;
using namespace arma;

// [[Rcpp::export]]
mat rmvnorm_arma_solve(int n, mat precision, vec location){
  
  int T = precision.n_rows;
  
  mat epsilon(T, n);
  for (int i=0; i<n; i++){
    epsilon.col(i) = as<vec>(rnorm(T));
  }
  mat location_matrix(T, n);
  location_matrix.each_col() = location;
  
  mat precision_chol  = trimatu(chol(precision));
  mat draw            = solve(precision_chol, 
                              solve(trans(precision_chol), 
                                    location_matrix) + epsilon);
  
  return draw;
}
{% endhighlight %}

The third function `rmvnorm_arma_stochvol` is based on three utility functions from package **stochvol** by [Kastner (2016)](http://dx.doi.org/10.18637/jss.v069.i05) and [Hosszejni & Kastner](https://arxiv.org/abs/1906.12123). These functions implement dedicated numerical algorithms described in [McCausland, Miller, & Pelletier (2011)](https://doi.org/10.1016/j.csda.2010.07.009) taking advantage of the fact that, in many applications of the state-space models, all the elements on the first sub-diagonal of the precision matrix are given by the same number. In this case, the algorithms for the Cholesky decomposition, forward- and back-substitution simplify granting further computational time gains. The three functions taken from the **stochvol** package namespace are `cholesky_tridiagonal`, `forward_algorithm`, and `backward_algorithm`. The only changes implemented in the functions presented in the listing below consider the definitions of object types that were simplified. The function `rmvnorm_arma_stochvol` exhibits a slightly adjusted structure relative to the first two presented functions for the simulation smoother accommodating the fact that the functions `forward_algorithm`, and `backward_algorithm` accept only a vector as the third argument and not a matrix.


{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;
using namespace arma;

/*
 The three functions below, cholesky_tridiagonal, forward_algorithm, 
 and backward_algorithm were copied from the source code of package 
 stochvol by Georg Kastner that was shared under licence GPL (>= 2). 
 They were modified by the author of the current note on 30 August 2021
 and are shared under the GPL (>= 2) licence as well.
 */
List cholesky_tridiagonal(const vec& omega_diag, const double& omega_offdiag) {
  const int T = omega_diag.n_elem - 1;
  vec chol_diag(T+1);
  vec chol_offdiag(T+1);
  chol_diag[0] = std::sqrt(omega_diag[0]);
  for (int j = 1; j < T+1; j++) {
    chol_offdiag[j-1] = omega_offdiag/chol_diag[j-1];
    chol_diag[j] = std::sqrt(omega_diag[j]-chol_offdiag[j-1]*chol_offdiag[j-1]);
  }
  return List::create(_["chol_diag"]=chol_diag, _["chol_offdiag"]=chol_offdiag);
}

vec forward_algorithm(const vec& chol_diag, const vec& chol_offdiag, const vec& covector) {
  const int T = chol_diag.n_elem - 1;
  vec htmp(T+1);
  htmp[0] = covector[0]/chol_diag[0];
  for (int j = 1; j < T+1; j++) {
    htmp[j] = (covector[j] - chol_offdiag[j-1]*htmp[j-1])/chol_diag[j];
  }
  return htmp;
}

vec backward_algorithm(const vec& chol_diag, const vec& chol_offdiag, const vec& htmp) {
  const int T = chol_diag.size() - 1;
  vec h(T+1);
  h[T] = htmp[T] / chol_diag[T];
  for (int j = T-1; j >= 0; j--) {
    h[j] = (htmp[j] - chol_offdiag[j] * h[j+1]) / chol_diag[j];
  }
  return h;
}

// [[Rcpp::export]]
mat rmvnorm_arma_stochvol(int n, mat precision, vec location){
  
  int     T                 = precision.n_rows;
  vec     precision_diag    = precision.diag();
  double  precision_offdiag = precision(1,0);
  
  List  precision_chol  = cholesky_tridiagonal(precision_diag, precision_offdiag);
  vec   aa              = forward_algorithm(precision_chol["chol_diag"],
                                            precision_chol["chol_offdiag"],
                                                          location);
  mat draw(T, n);
  vec epsilon;
  for (int i=0; i<n; i++){
    epsilon     = rnorm(T);
    draw.col(i) = backward_algorithm(precision_chol["chol_diag"], 
                                     precision_chol["chol_offdiag"],
                                                   aa + epsilon);
  }

  return draw;
}
{% endhighlight %}


## Three implementations in R

In addition to the cpp functions, three R functions are presented for the sake of comparing the computational time of their execution. These functions exhibit a similar structure as the cpp functions with respect to the definition of the arguments, outputs, and the order of implementing the sampling algorithm steps.

The first function `rmvnorm_solve` uses the functionality of R's **base** package and is very similar in its structure to function `rmvnorm_arma_inv`. The Cholesky decomposition is performed using function `chol` and the inversion of a matrix using function `solve`. These functions seem to not check the type of the input matrix. 


{% highlight r %}
rmvnorm_solve = function(n, precision, location){
  T           = dim(precision)[1]
  precision.chol.inv    = solve(t(chol(precision)))
  
  epsilon     = matrix(rnorm(n * T), ncol=n)
  draw        = t(precision.chol.inv) %*% (matrix(rep(precision.chol.inv %*% location, n), ncol=n) + epsilon)
  
  return(draw)
}
{% endhighlight %}

The remaining two functions use **LAPACK** library R wrappers included in package **mgcv** by [Wood (2021)](https://cran.r-project.org/web/packages/mgcv/index.html). This package provides dedicated functions for the Cholesky decomposition for band and tridiagonal matrices. Additionally, these implementations use R's **base** package wrappers for the level-3 **BLAS** routine `dtrsm` for solving linear equations by forward- and back-substitution given by functions `forwardsolve` and `backsolve` respectively. They recognize a special type of the input matrix to be inverted to solve the linear equations.

The function `rmvnorm_bandchol` is written for a band precision matrix and uses function `bandchol`, being the implementation of **LAPACK**'s `dpbtrf` routine, to compute the Cholesky decomposition.


{% highlight r %}
library(mgcv)

rmvnorm_bandchol = function(n, precision, location){
  T           = dim(precision)[1]
  precision.L = t(bandchol(precision))
  
  epsilon     = matrix(rnorm(n * T), ncol=n)
  a           = forwardsolve(precision.L, location)
  draw        = backsolve(t(precision.L), matrix(rep(a, n), ncol=n) + epsilon)
  
  return(draw)
}
{% endhighlight %}

The third function `rmvnorm_trichol` is written for a tridiagonal precision matrix and uses function `trichol` from package **mgcv** to compute the Cholesky decomposition using **LAPACK**'s `dpttrf` routine.


{% highlight r %}
rmvnorm_trichol = function(n, precision, location){
  T           = dim(precision)[1]
  lead.diag   = diag(precision)
  sub.diag    = sdiag(precision, -1)
  
  precision.chol    = trichol(ld = lead.diag, sd=sub.diag)
  precision.L       = diag(precision.chol$ld)
  sdiag(precision.L,-1) = precision.chol$sd
  
  epsilon     = matrix(rnorm(n * T), ncol=n)
  a           = forwardsolve(precision.L, location)
  draw        = backsolve(t(precision.L), matrix(rep(a, n), ncol=n) + epsilon)
  
  return(draw)
}
{% endhighlight %}

The compiled versions of the three R functions were constructed using package **compiler**. However, they improved the computational time of their execution only by a small margin relative to the original functions and, thus, are skipped in the reporting below.

Another package **SparseM** by [Koenker & Ng (2003)](http://dx.doi.org/10.18637/jss.v008.i06) provides useful functions to sample random draws from the target distribution. Its application requires defining the precision matrix as sparse using function `as.matrix.csr`, computation of the Cholesky decomposition using function `chol`, and solving the linear equations using function `backsolve`. The computational gains from using such an implementation are significant relative to the basic R's implementation `rmvnorm_solve`. However, as the package vignette points out, function `chol` returns a solution subjected to a random permutation of rows and columns, which makes the results uncomparable with the reproducible implementations considered here.

## Performance comparison

The functions are compared using routine `microbenchmark` from package **microbenchmark** in an exercise consisting of sampling `n=10` draws from the target distribution with the `location` matrix filled with standard normal random draws, and the `precision` matrix imitating the structure of a precision matrix for a state-space equation given by an autoregressive process of order 1. It implies that this matrix is tridiagonal with all the elements on the first sub-diagonal being equal to the same number. The results are reported for the `precision` and `location` of size `T=250` which is motivated by the number of observations available for quarterly macroeconomic aggregates for many developed countries.


{% highlight r %}
set.seed(12345)
n           = 10
T           = 250
s           = rgamma(1, shape=10, scale=10)
precision   = rgamma(1, shape=10, scale=10) * diag(T) + 2 * s * diag(T)
sdiag(precision, 1)  = -s
sdiag(precision, -1) = -s
location    = as.matrix(rnorm(T))
{% endhighlight %}

Function `microbenchmark` includes the `set.seed` specification and returns the tabular outcome only if the matrices sampled using alternative functions are equal to one another.


{% highlight r %}
library(microbenchmark)

microbenchmark(cpp.inv      = rmvnorm_arma_inv(n, precision, location),
               cpp.solve    = rmvnorm_arma_solve(n, precision, location),
               cpp.stochvol = rmvnorm_arma_stochvol(n, precision, location),
               r.solve      = rmvnorm_solve(n, precision, location),
               r.bandchol   = rmvnorm_bandchol(n, precision, location),
               r.trichol    = rmvnorm_trichol(n, precision, location),
               check  = "equal",
               setup  = set.seed(123)
               )
{% endhighlight %}



<pre class="output">
Unit: microseconds
         expr      min       lq      mean   median        uq       max neval cld
      cpp.inv  790.184  882.677  3582.257  938.960  1678.659  49951.59   100  a 
    cpp.solve  504.267  914.869  1016.789  965.231  1020.201   1711.95   100  a 
 cpp.stochvol  172.530  322.851   401.402  346.279   364.765   4501.92   100  a 
      r.solve 2352.417 2583.490 17877.209 3918.437 17634.294 279008.16   100   b
   r.bandchol 1834.725 4234.104  6738.529 4432.209  5653.852  93943.39   100  a 
    r.trichol  484.612  813.264  4031.655  891.851  1605.639  97691.21   100  a 
</pre>

Unsurprisingly, function `rmvnorm_arma_stochvol` that is custom-made for this example wins the performance comparison. It is twice as fast as another cpp implementation `rmvnorm_arma_solve`, three times faster than the fastest R implementation `rmvnorm_trichol`, and over thirteen times faster than R's `rmvnorm_bandchol`.

Two other unreported runs of this exercise were performed for `T=750` and `T=2500`, the sample sizes corresponding to the number of observations available for monthly macroeconomic aggregates and ten years of daily financial data respectively. The ordering of the functions in terms of their average execution time remained the same while the multipliers increased. For instance, when `T=2500` the function `rmvnorm_arma_stochvol` was over six times faster than both `rmvnorm_arma_solve` and `rmvnorm_trichol` and over seventeen times faster than `rmvnorm_arma_inv` and `rmvnorm_bandchol`.

## Conclusion

One can expect substantial gains in the computation times by writing the functions in **Rcpp** for the simulation smoother. The more fine-tuned the simulation algorithm to the application at hand the more improvements are provided. The computational gains are significant in the context of using the simulation smoother in the Gibbs sampler, an iterative Bayesian estimation procedure in which a single or multiple draws from the multivariate normal distribution are sampled at each iteration, where the number of iterations ranges between several thousand up to even a million.

Tomasz thanks Alex Ballentine, Matthieu Droumaguet, and Gregor Kastner for their useful feedback.
