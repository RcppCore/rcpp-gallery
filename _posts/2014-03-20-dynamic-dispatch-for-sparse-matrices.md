---
title: Dynamic dispatch for sparse matrices
author: Fabian Scheipl
license: GPL (>= 2)
tags: sparse
summary: Define a matrix multiplication function for "sparse times dense" and "dense times dense".  
layout: post
src: 2014-03-20-dynamic-dispatch-for-sparse-matrices.Rmd
---

We want to do matrix multiplication for 3 cases:

* dense times dense
* sparse times dense for sparse matrices of class `dgCMatrix` 
* sparse times dense for sparse matrices of class `indMatrix`,

using R's `Matrix` package for sparse matrices in R and 
`RcppArmadillo` for C++ linear algebra:


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
using namespace Rcpp ;

arma::mat matmult_sp(const arma::sp_mat X, const arma::mat Y){
    arma::mat ret = X * Y;
    return ret;
};
arma::mat matmult_dense(const arma::mat X, const arma::mat Y){
    arma::mat ret = X * Y;
    return ret;
};
arma::mat matmult_ind(const SEXP Xr, const arma::mat Y){
    // pre-multiplication with index matrix is a permutation of Y's rows: 
    arma::uvec perm =  as<S4>(Xr).slot("perm");
    arma::mat ret = Y.rows(perm - 1);
    return ret;
};

//[[Rcpp::export]]
arma::mat matmult_cpp(SEXP Xr, const arma::mat Y) {
    if (Rf_isS4(Xr)) {
        if(Rf_inherits(Xr, "dgCMatrix")) {
            return matmult_sp(as<arma::sp_mat>(Xr), Y) ;
        } ;
        if(Rf_inherits(Xr, "indMatrix")) {
            return matmult_ind(Xr, Y) ; 
        } ;
        stop("unknown class of Xr") ;
    } else {
        return matmult_dense(as<arma::mat>(Xr), Y) ;
    } 
}
{% endhighlight %}


**Set up test cases:**

{% highlight r %}
library(Matrix)
{% endhighlight %}



<pre class="output">
Loading required package: methods
</pre>



{% highlight r %}
library(rbenchmark)
set.seed(12211212)
n <- 1000
d <- 50
p <- 30  

X <- matrix(rnorm(n*d), n, d)
X_sp <- as(diag(n)[,1:d], "dgCMatrix")
X_ind <- as(sample(1:d, n, rep=TRUE), "indMatrix")
Y <- matrix(1:(d*p), d, p)
{% endhighlight %}


**Check exception handling:**

{% highlight r %}
matmult_cpp(as(X_ind, "ngTMatrix"), Y)
{% endhighlight %}



<pre class="output">
Error: unknown class of Xr
</pre>


**Dense times dense:**

{% highlight r %}
all.equal(X%*%Y, matmult_cpp(X, Y))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
benchmark(X%*%Y, 
          matmult_cpp(X, Y),
          replications=100)[,1:4]
{% endhighlight %}



<pre class="output">
               test replications elapsed relative
2 matmult_cpp(X, Y)          100   0.086     1.00
1           X %*% Y          100   0.098     1.14
</pre>


**`dgCMatrix` times dense:**

{% highlight r %}
all.equal(as(X_sp%*%Y, "matrix"), matmult_cpp(X_sp, Y),
          check.attributes = FALSE)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
benchmark(X_sp%*%Y, 
          matmult_cpp(X_sp, Y),
          replications=100)[,1:4]
{% endhighlight %}



<pre class="output">
                  test replications elapsed relative
2 matmult_cpp(X_sp, Y)          100   0.009    1.000
1           X_sp %*% Y          100   0.025    2.778
</pre>


**`indMatrix` times dense:**

{% highlight r %}
all.equal(X_ind%*%Y, matmult_cpp(X_ind, Y))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
benchmark(X_ind%*%Y, 
          matmult_cpp(X_ind, Y),
          replications=100)[,1:4]
{% endhighlight %}



<pre class="output">
                   test replications elapsed relative
2 matmult_cpp(X_ind, Y)          100   0.013    1.000
1           X_ind %*% Y          100   0.025    1.923
</pre>

    
Based on [this](http://stackoverflow.com/a/22531129/295025) Q&A on StackOverflow,
thanks again to Kevin Ushey for his helpful comment.
