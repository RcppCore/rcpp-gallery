---
title: Using a GSL function from R
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: modeling gsl featured
summary: This example shows how to call a GSL function using RcppGSL
layout: post
src: 2012-12-22-gsl-colnorm-example.cpp
---
The [GNU GSL](http://www.gnu.org/software/gsl/) is very popular--and 
versatile--library convering many, many scientific computing topics. It 
provides a standard C API which is somewhat restrictive. However, RcppGSL
makes it easy to pass matrices and vectors in and out.

The following example, based on the code used in the complete (!!)
example package included within RcppGSL, which itself in based on
an example from the GSL documentation, illustrates this by
computing simple vector norm given matrix.

As explained in the package documentation, the RcppGSL clue code
instantiates C language pointers suitable for GSL (here the matrix
`M`). Those *must* be free manually, as shown before the `return`
statement.  Otherwise the example is straighforward: take a matrix,
create a return vector and compute the chosen norm for each column
of the matrix.  

This example is also shorter and simpler thanks to Rcpp Attributes.



{% highlight cpp %}
// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

// [[Rcpp::export]]
Rcpp::NumericVector colNorm(Rcpp::NumericMatrix sM) {

    RcppGSL::matrix<double> M(sM); 	// create gsl data structures from SEXP
    int k = M.ncol();
    Rcpp::NumericVector n(k); 		// to store results 

    for (int j = 0; j < k; j++) {
        RcppGSL::vector_view<double> colview = gsl_matrix_column (M, j);
	n[j] = gsl_blas_dnrm2(colview);
    }
    M.free() ;                          // important as GSL wrappers use C structure
    return n;				// return vector  
}
{% endhighlight %}


A quick illustration, based on Section 8.4.13 of the GSL manual follows.

{% highlight r %}
## create M as a sum of two outer products
M <- outer(sin(0:9), rep(1,10), "*") + outer(rep(1, 10), cos(0:9), "*")
colNorm(M)
{% endhighlight %}



<pre class="output">
 [1] 4.315 3.121 2.193 3.261 2.534 2.573 4.205 3.652 2.085 3.073
</pre>



{% highlight r %}
## same result using just R
apply(M, 2, function(x) sqrt(sum(x^2)))
{% endhighlight %}



<pre class="output">
 [1] 4.315 3.121 2.193 3.261 2.534 2.573 4.205 3.652 2.085 3.073
</pre>

