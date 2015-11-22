---
title: Using GSL functions from R
author: Dirk Eddelbuettel
updated: Nov 21, 2015
license: GPL (>= 2)
tags: modeling gsl featured
summary: This example shows how to call a GSL function using RcppGSL
layout: post
src: 2012-12-22-gsl-colnorm-example.cpp
---
The [GNU GSL](http://www.gnu.org/software/gsl/) is a very
popular--and versatile--library convering many, many scientific
computing topics. It provides a standard C API. This API is
somewhat restrictive for C++ programmer. However, RcppGSL makes it
very easy to pass matrices and vectors in and out.

The following example, based on the code used in the complete (!!)
example package included within RcppGSL, which itself in based on
an example from the GSL documentation, illustrates this by
computing simple vector norm given matrix.

As explained in the package documentation, the RcppGSL clue code
instantiates C language pointers suitable for GSL (here the matrix
`M`). In versions prior to RcppGSL 0.3.0, those *had to* be freed
manually.  Since release 0.3.0, an simple internal mechanism takes
care of this automatically at the end of the score. This form is
more common to C++, and now shown below.  Other aspects of the the
example are straighforward: take a matrix, create a return vector
and compute the chosen norm for each column of the matrix.

This example is also shorter and simpler thanks to Rcpp Attributes.
For illustration, several older approaches are still 
[contained in the source file](https://github.com/eddelbuettel/rcppgsl/blob/master/inst/examples/RcppGSLExample/src/colNorm.cpp).


{% highlight cpp %}
// [[Rcpp::depends(RcppGSL)]]

#include <RcppGSL.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

// [[Rcpp::export]]
Rcpp::NumericVector colNorm(const RcppGSL::Matrix & M) {

    int k = M.ncol();
    Rcpp::NumericVector n(k); 		// to store results 

    for (int j = 0; j < k; j++) {
        RcppGSL::VectorView colview = gsl_matrix_const_column (M, j);
	n[j] = gsl_blas_dnrm2(colview);
    }
    
    return n;				// return vector  
}
{% endhighlight %}

The example function computes a column norm, and returns a
(standard) Rcpp vector of type `NumericVector`.  On input, it takes
a matrix from R which is already instantiated to a proxy object
which maps between R GSL matrices (without making copies).  Here
`RcppGSL::Matrix` is a convenient `typedef` shorthand for
`RcppGSL::matrix<double>`. All the standard GSL types are available
via templating. However, `double` and `int` are the most sensible
in out context as they correspond to R types.  Lastly, also note
that the GSL vector view types explicitly references a `const`
column, this matches the `const &` declaration of the matrix `M`.


A quick illustration, based on 
[Section 8.4.13 of the GSL manual](http://www.gnu.org/software/gsl/manual/html_node/Example-programs-for-matrices.html) (and thanks to R reduced to a one-liner for the data generation) follows.

{% highlight r %}
## create M as a sum of two outer products
M <- outer(sin(0:9), rep(1,10), "*") + outer(rep(1, 10), cos(0:9), "*")
colNorm(M)
{% endhighlight %}



<pre class="output">
 [1] 4.31461 3.12050 2.19316 3.26114 2.53416 2.57281 4.20469 3.65202
 [9] 2.08524 3.07313
</pre>



{% highlight r %}
## same result using just R
apply(M, 2, function(x) sqrt(sum(x^2)))
{% endhighlight %}



<pre class="output">
 [1] 4.31461 3.12050 2.19316 3.26114 2.53416 2.57281 4.20469 3.65202
 [9] 2.08524 3.07313
</pre>
