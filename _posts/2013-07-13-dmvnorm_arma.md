---
title: Faster Multivariate Normal densities with RcppArmadillo and OpenMP
author: Nino Hardt, Dicko Ahmadou, Benjamin Christoffersen
license: GPL (>= 2)
tags: armadillo openmp featured
summary: Fast implementation of Multivariate Normal density using RcppArmadillo and OpenMP.
layout: post
src: 2013-07-13-dmvnorm_arma.Rmd
---

The Multivariate Normal density function is used frequently
for a number of problems. Especially for MCMC problems, fast 
evaluation is important. Multivariate Normal Likelihoods, 
Priors and mixtures of Multivariate Normals require numerous 
evaluations, thus speed of computation is vital. 
We show dramatic increases in speed by using efficient algorithms,
RcppArmadillo, and some extra gain by using OpenMP.
The code is based on the latest version of RcppArmadillo (0.9.800.1.0).

While the `dmvnorm()` function from the `mvtnorm` package is quite popular,
and in an earlier version of this article we demonstrated that an 
Rcpp implementation would lead to faster computation.

Peter Rossi, author of `bayesm`, called our attention to the `bayesm` pure R 
implementation which is much faster than `dmvnorm()`. 
The function `dMvn()` is used internally by the mixture of normals model in 
`bayesm`. It is the matrix-equivalent version of `lndMvn`:



{% highlight r %}
dMvn <- function(X,mu,Sigma) {
    k <- ncol(X)
    rooti <- backsolve(chol(Sigma),diag(k))
    quads <- colSums((crossprod(rooti,(t(X)-mu)))^2)
    return(exp(-(k/2)*log(2*pi) + sum(log(diag(rooti))) - .5*quads))
}
{% endhighlight %}

Translating the vectorized approach into RcppArmadillo, 
we precompute the inverse of the Cholesky decomposition of the covariance 
matrix ahead of the main loop over the rows of `X`. 
The loop can easily be parallelized, and the code is easy to read and 
manipulate. For instance, the inverse Cholesky decomposition can be put 
inside the main loop, if varying covariance matrices are necessary.
It is worth remarking that multiplying with a precomputed inverse of the 
Cholesky decomposition of 
the covariance matrix is faster but less numerically stable compared to 
a backsolve as in the `Mahalanobis` function we will define later.



{% highlight cpp %}
// [[Rcpp::depends("RcppArmadillo")]]
#include <RcppArmadillo.h>
  
static double const log2pi = std::log(2.0 * M_PI);

// [[Rcpp::export]]
arma::vec dmvnrm_arma_old(arma::mat x,  
                          arma::rowvec mean,  
                          arma::mat sigma, 
                          bool logd = false) { 
    using arma::uword;
    uword const n = x.n_rows, 
             xdim = x.n_cols;
    arma::vec out(n);
    arma::mat rooti = arma::trans(arma::inv(trimatu(arma::chol(sigma))));
    double rootisum = arma::sum(log(rooti.diag()));
    double constants = -(double)xdim/2.0 * log2pi;
    
    for (uword i = 0; i < n; i++) {
        arma::vec z = rooti * arma::trans( x.row(i) - mean) ;    
        out(i)      = constants - 0.5 * arma::sum(z%z) + rootisum;     
    }  
      
    if (logd)
      return out;
    return exp(out);
}

// [[Rcpp::export]]
arma::vec dmvnrm_arma(arma::mat const &x,  
                      arma::rowvec const &mean,  
                      arma::mat const &sigma, 
                      bool const logd = false) { 
    using arma::uword;
    uword const n = x.n_rows, 
             xdim = x.n_cols;
    arma::vec out(n);
    arma::mat const rooti = arma::inv(trimatu(arma::chol(sigma)));
    double const rootisum = arma::sum(log(rooti.diag())), 
                constants = -(double)xdim/2.0 * log2pi, 
              other_terms = rootisum + constants;
    
    arma::rowvec z;
    for (uword i = 0; i < n; i++) {
        z      = (x.row(i) - mean) * rooti;    
        out(i) = other_terms - 0.5 * arma::dot(z, z);     
    }  
      
    if (logd)
      return out;
    return exp(out);
}

/* C++ version of the dtrmv BLAS function */
void inplace_tri_mat_mult(arma::rowvec &x, arma::mat const &trimat){
  arma::uword const n = trimat.n_cols;
  
  for(unsigned j = n; j-- > 0;){
    double tmp(0.);
    for(unsigned i = 0; i <= j; ++i)
      tmp += trimat.at(i, j) * x[i];
    x[j] = tmp;
  }
}

// [[Rcpp::export]]
arma::vec dmvnrm_arma_fast(arma::mat const &x,  
                           arma::rowvec const &mean,  
                           arma::mat const &sigma, 
                           bool const logd = false) { 
    using arma::uword;
    uword const n = x.n_rows, 
             xdim = x.n_cols;
    arma::vec out(n);
    arma::mat const rooti = arma::inv(trimatu(arma::chol(sigma)));
    double const rootisum = arma::sum(log(rooti.diag())), 
                constants = -(double)xdim/2.0 * log2pi, 
              other_terms = rootisum + constants;
    
    arma::rowvec z;
    for (uword i = 0; i < n; i++) {
        z = (x.row(i) - mean);
        inplace_tri_mat_mult(z, rooti);
        out(i) = other_terms - 0.5 * arma::dot(z, z);     
    }  
      
    if (logd)
      return out;
    return exp(out);
}
{% endhighlight %}

The use of `trimatu` allows to exploit that of the Cholesky
decomposition the covariance matrix is an upper triangular matrix in the 
inversion. The `dmvnrm_arma_old` is an older version of the function used in
a previous version of this article. The new version differs mainly by

 1. using `const &` for the input parameters. 
 2. declaring `z` outside the loop.  
 3. using `arma::dot` instead of `arma::sum`. 
 4. other minor things.
 
This turns out to be quite important for the computation times. The 
`dmvnrm_arma_fast` makes an inplace vector matrix product and exploits
that the matrix is an upper triangular matrix. One can use the 
[dtrmv](http://www.netlib.org/lapack/explore-html/d7/d15/group__double__blas__level2_ga596c2acd9f81df6608bd5ed97e193897.html#ga596c2acd9f81df6608bd5ed97e193897)
BLAS function instead. It is not available through the Armadillo library 
though.

Additionally, we can make use of the OpenMP library to use multiple 
cores. For the OpenMP implementation, we need to enable OpenMP support. 
One way of doing so is by adding the required compiler and linker 
flags as follows:


{% highlight r %}
Sys.setenv("PKG_CXXFLAGS"="-fopenmp")
Sys.setenv("PKG_LIBS"="-fopenmp")
{% endhighlight %}

Rcpp version 0.10.5 and later will also provide a plugin to set these
variables for us:


{% highlight cpp %}
// [[Rcpp::plugins(openmp)]]
{% endhighlight %}

We also need to set the number of cores to be used before running the
compiled functions. One way is to use `detectCores()` from the `parallel`
package.


{% highlight r %}
cores <- parallel::detectCores(logical = FALSE)
{% endhighlight %}

Only two additional lines are needed to enable multicore processing. 
In this example, a dynamic schedule is used for OpenMP. 
A static schedule might be faster in some cases. However,this is 
left to further experimentation.


{% highlight cpp %}
// [[Rcpp::depends("RcppArmadillo")]]
#include <RcppArmadillo.h>
#include <omp.h>

static double const log2pi = std::log(2.0 * M_PI);

void inplace_tri_mat_mult(arma::rowvec &x, arma::mat const &trimat){
  arma::uword const n = trimat.n_cols;
  
  for(unsigned j = n; j-- > 0;){
    double tmp(0.);
    for(unsigned i = 0; i <= j; ++i)
      tmp += trimat.at(i, j) * x[i];
    x[j] = tmp;
  }
}

// [[Rcpp::export]]
arma::vec dmvnrm_arma_mc(arma::mat const &x,  
                         arma::rowvec const &mean,  
                         arma::mat const &sigma, 
                         bool const logd = false,
                         int const cores = 1) {  
    using arma::uword;
    omp_set_num_threads(cores);
    uword const n = x.n_rows, 
             xdim = x.n_cols;
    arma::vec out(n);
    arma::mat const rooti = arma::inv(trimatu(arma::chol(sigma)));
    double const rootisum = arma::sum(log(rooti.diag())), 
                constants = -(double)xdim/2.0 * log2pi, 
              other_terms = rootisum + constants;
    
    arma::rowvec z;
    #pragma omp parallel for schedule(static) private(z)
    for (uword i = 0; i < n; i++) {
        z = (x.row(i) - mean);
        inplace_tri_mat_mult(z, rooti);   
        out(i) = other_terms - 0.5 * arma::dot(z, z);     
    }  
      
    if (logd)
      return out;
    return exp(out);
}
{% endhighlight %}

Likewise, it is easy to translate `dmvnorm` from the `mvtnorm` 
package into Rcpp:


{% highlight cpp %}
// [[Rcpp::depends("RcppArmadillo")]]
#include <RcppArmadillo.h>

static double const log2pi = std::log(2.0 * M_PI);

arma::vec Mahalanobis(arma::mat const &x, 
                      arma::vec const &center, 
                      arma::mat const &cov) {
    arma::mat x_cen = x.t();
    x_cen.each_col() -= center;
    arma::solve(x_cen, arma::trimatl(chol(cov).t()), x_cen);
    x_cen.for_each( [](arma::mat::elem_type& val) { val = val * val; } );
    return arma::sum(x_cen, 0).t();    
}

// [[Rcpp::export]]
arma::vec dmvnorm_arma(arma::mat const &x, 
                       arma::vec const &mean, 
                       arma::mat const &sigma, 
                       bool const logd = false) { 
    arma::vec const distval = Mahalanobis(x,  mean, sigma);
    double const logdet = sum(arma::log(arma::eig_sym(sigma)));
    arma::vec const logretval = 
      -( (x.n_cols * log2pi + logdet + distval)/2  ) ;
    
    if (logd)
        return logretval;
    return exp(logretval);
}
{% endhighlight %}

We use `each_col`, an appropriate overload of `arma::solve`, and `for_each` 
to do the computations without performing additional allocations after the 
copy at `x_cen = x.t()`. 

Now we simulate some data for benchmarking:


{% highlight r %}
set.seed(123)
sigma <- bayesm::rwishart(10,diag(8))$IW
means <- rnorm(8)
X     <- mvtnorm::rmvnorm(900000, means, sigma)
{% endhighlight %}

And run the benchmark:


{% highlight r %}
print(paste0("Using ",cores," cores for _mc versions"))
{% endhighlight %}



<pre class="output">
[1] &quot;Using 12 cores for _mc versions&quot;
</pre>



{% highlight r %}
require(rbenchmark)
{% endhighlight %}



<pre class="output">
Loading required package: rbenchmark
</pre>



{% highlight r %}
benchmark(
  dmvnorm          = mvtnorm::dmvnorm(X,means,sigma,log=FALSE), 
  dmvnorm_arma     = dmvnorm_arma    (X,means,sigma,FALSE), 
  dmvnrm_arma      = dmvnrm_arma     (X,means,sigma,FALSE),
  dmvnrm_arma_mc   = dmvnrm_arma_mc  (X,means,sigma,FALSE,cores), 
  dmvnrm_arma_old  = dmvnrm_arma_old (X,means,sigma,FALSE), 
  dmvnrm_arma_fast = dmvnrm_arma_fast(X,means,sigma,FALSE),
  dMvn             = dMvn            (X,means,sigma),
  order="relative", replications=100)[,1:4]
{% endhighlight %}



<pre class="output">
              test replications elapsed relative
4   dmvnrm_arma_mc          100   0.931    1.000
6 dmvnrm_arma_fast          100   3.617    3.885
3      dmvnrm_arma          100   5.572    5.985
5  dmvnrm_arma_old          100   7.739    8.313
2     dmvnorm_arma          100   9.219    9.902
7             dMvn          100  12.453   13.376
1          dmvnorm          100  14.326   15.388
</pre>

Lastly, we show that the functions yield the same results:


{% highlight r %}
truth <- mvtnorm::dmvnorm                      (X,means,sigma,log=FALSE)
all(
  isTRUE(all.equal(truth, drop(dmvnorm_arma    (X,means,sigma,    FALSE)))), 
  isTRUE(all.equal(truth, drop(dmvnrm_arma_fast(X,means,sigma,    FALSE)))),
  isTRUE(all.equal(truth, drop(dmvnrm_arma_mc  (X,means,sigma,    FALSE)))),
  isTRUE(all.equal(truth, drop(dmvnrm_arma     (X,means,sigma,    FALSE)))),
  isTRUE(all.equal(truth, drop(dMvn            (X,means,sigma)))))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>
