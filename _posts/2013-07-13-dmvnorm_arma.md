---
title: Faster Multivariate Normal densities with RcppArmadillo and OpenMP
author: Nino Hardt, Dicko Ahmadou
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
The code is based on the latest version of RcppArmadillo (0.3.910.0).

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
we precompute the inverse root of the covariance matrix ahead 
of the main loop over the rows of `X`. 
The loop can easily be parallelized, and the code is easy to read and 
manipulate. For instance, the inverse root can be put inside the main 
loop, if varying covariance matrices are necessary.
The use of `trimatu` allows to exploit the diagonality of the Cholesky
root of the covariance matrix.



{% highlight cpp %}
#include <RcppArmadillo.h>
  
const double log2pi = std::log(2.0 * M_PI);

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::export]]
arma::vec dmvnrm_arma(arma::mat x,  
                      arma::rowvec mean,  
                      arma::mat sigma, 
                      bool logd = false) { 
    int n = x.n_rows;
    int xdim = x.n_cols;
    arma::vec out(n);
    arma::mat rooti = arma::trans(arma::inv(trimatu(arma::chol(sigma))));
    double rootisum = arma::sum(log(rooti.diag()));
    double constants = -(static_cast<double>(xdim)/2.0) * log2pi;
    
    for (int i=0; i < n; i++) {
        arma::vec z = rooti * arma::trans( x.row(i) - mean) ;    
        out(i)      = constants - 0.5 * arma::sum(z%z) + rootisum;     
    }  
      
    if (logd == false) {
        out = exp(out);
    }
    return(out);
}
{% endhighlight %}


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
cores <- parallel::detectCores()
{% endhighlight %}


Only two additional lines are needed to enable multicore processing. 
In this example, a dynamic schedule is used for OpenMP. 
A static schedule might be faster in some cases. However,this is 
left to further experimentation.



{% highlight cpp %}
#include <RcppArmadillo.h>
#include <omp.h>

const double log2pi = std::log(2.0 * M_PI);

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::export]]
arma::vec dmvnrm_arma_mc(arma::mat x,  
                         arma::rowvec mean,  
                         arma::mat sigma, 
                         bool logd = false,
                         int cores = 1) { 
    omp_set_num_threads(cores);
    int n = x.n_rows;
    int xdim = x.n_cols;
    arma::vec out(n);
    arma::mat rooti = arma::trans(arma::inv(trimatu(arma::chol(sigma))));
    double rootisum = arma::sum(log(rooti.diag()));
    double constants = -(xdim/2) * log2pi;
    #pragma omp parallel for schedule(static) 
    for (int i=0; i < n; i++) {
        arma::vec z = rooti * arma::trans( x.row(i) - mean) ;    
        out(i)      = constants - 0.5 * arma::sum(z%z) + rootisum;     
    }  
      
    if (logd==false) {
        out=exp(out);
    }
    return(out);
}
{% endhighlight %}



Likewise, it is easy to translate 'dmvnorm' from the 'mvtnorm' 
package into Rcpp:


{% highlight cpp %}
#include <RcppArmadillo.h>

const double log2pi = std::log(2.0 * M_PI);

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::export]]
arma::vec Mahalanobis(arma::mat x, arma::rowvec center, arma::mat cov) {
    int n = x.n_rows;
    arma::mat x_cen;
    x_cen.copy_size(x);
    for (int i=0; i < n; i++) {
        x_cen.row(i) = x.row(i) - center;
    }
    return sum((x_cen * cov.i()) % x_cen, 1);    
}

// [[Rcpp::export]]
arma::vec dmvnorm_arma(arma::mat x, arma::rowvec mean, arma::mat sigma, bool log = false) { 
    arma::vec distval = Mahalanobis(x,  mean, sigma);
    double logdet = sum(arma::log(arma::eig_sym(sigma)));
    arma::vec logretval = -( (x.n_cols * log2pi + logdet + distval)/2  ) ;
    
    if (log) { 
        return(logretval);
    } else { 
        return(exp(logretval));
    }
}
{% endhighlight %}




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
[1] &quot;Using 8 cores for _mc versions&quot;
</pre>



{% highlight r %}
require(rbenchmark)
{% endhighlight %}



<pre class="output">
Loading required package: rbenchmark
</pre>



{% highlight r %}
benchmark(mvtnorm::dmvnorm(X,means,sigma,log=F), 
          dmvnorm_arma(X,means,sigma,F), 
          dmvnrm_arma(X,means,sigma,F) , 
          dmvnrm_arma_mc(X,means,sigma,F,cores), 
          dMvn(X,means,sigma),
          order="relative", replications=100)[,1:4]
{% endhighlight %}



<pre class="output">
                                        test replications elapsed relative
4  dmvnrm_arma_mc(X, means, sigma, F, cores)          100   9.636    1.000
3            dmvnrm_arma(X, means, sigma, F)          100  18.848    1.956
2           dmvnorm_arma(X, means, sigma, F)          100  23.459    2.435
5                      dMvn(X, means, sigma)          100  29.501    3.062
1 mvtnorm::dmvnorm(X, means, sigma, log = F)          100  44.556    4.624
</pre>




Lastly, we show that the functions yield the same results:


{% highlight r %}
all.equal(mvtnorm::dmvnorm(X,means,sigma,log=FALSE),
          dmvnorm_arma(X,means,sigma,FALSE)[,1],
	  dmvnrm_arma(X,means,sigma,FALSE)[,1],
	  dMvn(X,means,sigma))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



The use of RcppArmadillo brings about a significant increase 
in speed. The addition of OpenMP leads to only little 
additional performance. 

This example also illustrates that Rcpp does not completely
substitute the need to look for faster algorithms. Basing the
code of 'lndMvn' instead of 'dmvnorm' leads to a significantly
faster function.


