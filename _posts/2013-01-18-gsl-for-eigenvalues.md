---
title: Using the GSL to compute eigenvalues
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: modeling gsl
summary: This example shows how to call a GSL function using RcppGSL
layout: post
src: 2013-01-18-gsl-for-eigenvalues.cpp
---
Two posts showed how to compute eigenvalues 
[using Armadillo](../armadillo-eigenvalues) and 
[using Eigen](../eigen-eigenvalues/). As we also looked at using the  
[GNU GSL](http://www.gnu.org/software/gsl/), this post will show how to
conpute eigenvalues using GSL.

As mentioned in the [previous GSL post](../gsl-colnorm-example), we
instantiate C language pointers suitable for GSL (here the matrix
`M`). Those *must* be freed manually, as shown before the `return`
statement.  



{% highlight cpp %}
// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>

// [[Rcpp::export]]
Rcpp::NumericVector getEigenValues(Rcpp::NumericMatrix sM) {

    RcppGSL::matrix<double> M(sM); 	// create gsl data structures from SEXP
    int k = M.ncol();
    Rcpp::NumericVector N(k); 		// to store results 

    RcppGSL::vector<double> eigval(k);  // instead of gsl_vector_alloc(k);
    gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(k);
    gsl_eigen_symm (M, eigval, w);
    gsl_eigen_symm_free (w);

    Rcpp::NumericVector res(Rcpp::wrap(eigval));
    //res = eigval;
    //for (int j = 0; j < k; j++) {
    //    N[j] = gsl_vector_get(eigval, j);
    //}
    M.free();               		// important: GSL wrappers use C structure
    eigval.free();

    return res;				// return vector  
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
[1] 14.2100  2.4099  1.6856  0.3319
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

