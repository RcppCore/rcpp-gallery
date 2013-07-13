---
title: Faster Multivariate Normal densities with RcppArmadillo and openmp
author: Nino Hardt, Dicko Ahmadou
license: GPL (>= 2)
tags: armadillo openmp
summary: Fast implementation of Multivariate Normal density using RcppArmadillo and openmp.
layout: post
src: 2013-07-13-dmvnorm_arma.Rmd
---

The Multivariate Normal density function is used frequently
in a number of problems. Especially for MCMC problems, fast 
evaluation is important. Multivariate Normal Likelihoods, 
Priors and mixtures of Multivariate Normals require numerous 
evaluations, thus speed of computation is vital. 
We show a twofold increase in speed by using RcppArmadillo, 
and some extra gain by using openmp.

This project is based on the following [StackOverflow post](http://stackoverflow.com/questions/17617618/dmvnorm-mvn-density-rcpparmadillo-implementation-slower-than-r-package-includi). 

Loading necessary packages:

{% highlight r %}
libs <- c("mvtnorm","RcppArmadillo","rbenchmark","bayesm","parallel")
if (sum(!(libs %in% .packages(all.available = TRUE))) > 0) {
    install.packages(libs[!(libs %in% .packages(all.available = TRUE))])
}
{% endhighlight %}



<pre class="output">
Installing packages into '/usr/local/lib/R/site-library' (as 'lib' is
unspecified)
</pre>



<pre class="output">
Error: trying to use CRAN without setting a mirror
</pre>



{% highlight r %}

for (i in 1:length(libs)) {
    library(libs[i],character.only = TRUE,quietly=TRUE)
}
{% endhighlight %}



<pre class="output">
Error: there is no package called 'mvtnorm'
</pre>



First, we show the RcppArmadillo implementation without openmp.

{% highlight cpp %}
#include <RcppArmadillo.h>

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::export]]
arma::vec Mahalanobis(arma::mat x, arma::rowvec center, arma::mat cov){
    int n = x.n_rows;
    arma::mat x_cen;
    x_cen.copy_size(x);
    for (int i=0; i < n; i++) {
        x_cen.row(i) = x.row(i) - center;
    }
    return sum((x_cen * cov.i()) % x_cen, 1);    
}

// [[Rcpp::export]]
arma::vec dmvnorm_arma ( arma::mat x,  arma::rowvec mean,  arma::mat sigma, bool log = false){ 
    arma::vec distval = Mahalanobis(x,  mean, sigma);
    double logdet = sum(arma::log(arma::eig_sym(sigma)));
    double log2pi = std::log(2.0 * M_PI);
    arma::vec logretval = -( (x.n_cols * log2pi + logdet + distval)/2  ) ;
    
    if(log){ 
      return(logretval);
    }else { 
      return(exp(logretval));
    }
}
{% endhighlight %}


Now we simulate some data for benchmarking:

{% highlight r %}
#simulate data
    set.seed(3242352532)
{% endhighlight %}



<pre class="output">
Warning: NAs introduced by coercion
</pre>



<pre class="output">
Error: supplied seed is not a valid integer
</pre>



{% highlight r %}
    sigma = rwishart(10,diag(4))$IW
{% endhighlight %}



<pre class="output">
Error: could not find function &quot;rwishart&quot;
</pre>



{% highlight r %}
    means = rnorm(4)
    X     = rmvnorm(500000, means, sigma)
{% endhighlight %}



<pre class="output">
Error: could not find function &quot;rmvnorm&quot;
</pre>


And run benchmark:

{% highlight r %}
#benchmark
benchmark( mvtnorm::dmvnorm(X,means,sigma), 
           dmvnorm_arma(X,means,sigma) , order="relative", replications=50    )
{% endhighlight %}



<pre class="output">
Error: could not find function &quot;benchmark&quot;
</pre>



For the openmp implementation, we need to run the following 
commands under windows:

{% highlight r %}
if(Sys.info()['sysname']=="Windows"){
    Sys.setenv("PKG_CXXFLAGS"="-fopenmp")
    Sys.setenv("PKG_LIBS"="-fopenmp")
}
{% endhighlight %}


The source code only changes slightly. I have chosen a dynamic 
schedule for openmp, although a static schedule might be faster 
in some cases. This is left to further experimentation.

{% highlight cpp %}
#include <RcppArmadillo.h>
#include <Rcpp.h>
#include <omp.h>

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::export]]
arma::vec Mahalanobis_mc(arma::mat x, arma::rowvec center, arma::mat cov, int cores=1){
    omp_set_num_threads( cores );
    int n = x.n_rows;
    arma::mat x_cen;
    x_cen.copy_size(x);
    #pragma omp parallel for schedule(dynamic)   
        for (int i=0; i < n; i++) {
            x_cen.row(i) = x.row(i) - center;
        }
    return sum((x_cen * cov.i()) % x_cen, 1);    
}

// [[Rcpp::export]]
arma::vec dmvnorm_arma_mc ( arma::mat x,  arma::rowvec mean,  arma::mat sigma, bool log = false, int cores=1){ 
    arma::vec distval = Mahalanobis_mc(x,  mean, sigma, cores);
    double logdet = sum(arma::log(arma::eig_sym(sigma)));
    double log2pi = std::log(2.0 * M_PI);
    arma::vec logretval = -( (x.n_cols * log2pi + logdet + distval)/2  ) ;
    
    if(log){ 
        return(logretval);
    }else { 
        return(exp(logretval));
    }
}
{% endhighlight %}



We need to set the number of cores to be used. 

{% highlight r %}
    cores=detectCores()
{% endhighlight %}



<pre class="output">
Error: could not find function &quot;detectCores&quot;
</pre>



Now we are ready to benchmark again. The speed gain of 
the openmp version is not big, but noticable. 


{% highlight r %}
benchmark( mvtnorm::dmvnorm(X,means,sigma), 
           dmvnorm_arma(X,means,sigma) ,
           dmvnorm_arma_mc(X,means,sigma, cores), order="relative", replications=50    )
{% endhighlight %}



<pre class="output">
Error: could not find function &quot;benchmark&quot;
</pre>


The use of RcppArmadillo brings about a significant increase 
in speed. The addition of openmp leads to only little 
additional performance. The largest share of the increase 
in speed is due to faster computation of the Mahalanobis distance, 
which is used to compute the Multivariate Normal density.
