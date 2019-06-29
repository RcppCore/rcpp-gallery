---
title: Creating as and wrap for sparse matrices
author: Dirk Eddelbuettel and Binxiang Ni
updated: Oct 18, 2018
license: GPL (>= 2)
tags: armadillo matrix sparse
summary: This post shows an example of simple as and wrap methods
layout: post
src: 2013-08-05-as-and-wrap-for-sparse-matrices.cpp
---
An earlier article discussed
[sparse matrix conversion](https://gallery.rcpp.org/articles/armadillo-sparse-matrix)
but stopped short of showing how to create custom `as<>()` and `wrap()` methods
or functions.  This post started to close this gap.

We will again look at sparse matrices from the
[Matrix](http://cran.r-project.org/package=Matrix) package for R, as well as
the `SpMat` class from [Armadillo](http://arma.sf.net).
At least for now we will limit outselves to the
case of `double` element types. These uses the `sp_mat` typedef which will be
our basic type for sparse matrices at the C++ level.

_Nota bene: At the time of the update of the post, very similar
code (by Romain) has just been added to the SVN repo for
`RcppArmadillo`; it should appear in the next regular CRAN
release. Because we cannot redefine method with the same signature,
we renamed the code presented here `as_<>()` and `wrap_()`. Of
course, for conversion not already present in the package, names
without underscores should be used instead. My thanks to Romain for
improving over the initial versions I wrote in the first version of
this post._

_Nota bene 2: As of `RcppArmadillo` release 0.3.920.1 (based on `Armadillo` 3.920.1)
there is also a new constructor taking vectors `rowind`, `colptr` and `values`._

_Nota bene 3:
Since `RcppArmadillo` release 0.7.960.1.1, a fulle eleven types of sparse matrices on
the R side are supported. You can pass these directly to RcppArmadillo, and each will be
converted to an `arma::sp_mat` by a corresponding function. More details are in
the [sparse Matrix vignette](https://cran.r-project.org/web/packages/RcppArmadillo/vignettes/RcppArmadillo-sparseMatrix.pdf)._

First, we look at the `as` method.


{% highlight cpp %}
 #include <RcppArmadillo.h>
 // [[Rcpp::depends(RcppArmadillo)]]

 using namespace Rcpp;

 namespace Rcpp {

     // converts an SEXP object from R which was created as a sparse
     // matrix via the Matrix package) into an Armadillo sp_mat matrix
     //
     // NB: called as_() here as a similar method is already in the
     //     RcppArmadillo sources
     //

     // The following as_() only applies to dgCMatrix.
     // For other types of sparse matrix conversion,
     // you might want to check the source code of RcppArmadillo.
     template <typename T> arma::SpMat<T> as_(SEXP sx) {

       // Rcpp representation of template type
       const int RTYPE = Rcpp::traits::r_sexptype_traits<T>::rtype;

       // instantiate S4 object with the sparse matrix passed in
       S4 mat(sx);
       IntegerVector dims = mat.slot("Dim");
       IntegerVector i = mat.slot("i");
       IntegerVector p = mat.slot("p");
       Vector<RTYPE> x = mat.slot("x");

       // create sp_mat object of appropriate size
       arma::SpMat<T> res(dims[0], dims[1]);

       // In order to access the internal arrays of the SpMat class
       res.sync();

       // Making space for the elements
       res.mem_resize(static_cast<unsigned>(x.size()));

       // Copying elements
       std::copy(i.begin(), i.end(), arma::access::rwp(res.row_indices));
       std::copy(p.begin(), p.end(), arma::access::rwp(res.col_ptrs));
       std::copy(x.begin(), x.end(), arma::access::rwp(res.values));

       return res;
     }

 }
{% endhighlight %}

If you are interested in the as<>() for more types of sparse matrix,
you might want to take a look at the [source code](https://github.com/RcppCore/RcppArmadillo/blob/42e8b5b619771f9076c246de652eff4ad1e9c66a/inst/include/RcppArmadilloAs.h#L96)

Next, we look at the corresponding `wrap()` method.
Here the `sp_mat` Armadillo object is wrapped into a `dgCMatrix`.

{% highlight cpp %}
 namespace Rcpp {

     // convert an Armadillo sp_mat into a corresponding R sparse matrix
     // we copy to STL vectors as the Matrix package expects vectors whereas the
     // default wrap in Armadillo returns matrix with one row (or col)
     //
     // NB: called wrap_() here as a similar method is already in the
     //     RcppArmadillo sources
     //
     template <typename T> SEXP wrap_(const arma::SpMat<T>& sm) {
       const int  RTYPE = Rcpp::traits::r_sexptype_traits<T>::rtype;

       // In order to update internal state of SpMat object
       sm.sync();
       IntegerVector dim = IntegerVector::create( sm.n_rows, sm.n_cols );

       // copy the data into R objects
       Vector<RTYPE> x(sm.values, sm.values + sm.n_nonzero ) ;
       IntegerVector i(sm.row_indices, sm.row_indices + sm.n_nonzero);
       IntegerVector p(sm.col_ptrs, sm.col_ptrs + sm.n_cols+1 ) ;

       std::string klass = "dgCMatrix";

       S4 s(klass);
       s.slot("i")   = i;
       s.slot("p")   = p;
       s.slot("x")   = x;
       s.slot("Dim") = dim;
       return s;
     }

 }
{% endhighlight %}

We can now illustrate this with a simple example. _Note that the
compiler will use the methods `as<>()` and `wrap()` from the package
rather than the ones depicted here. However, the ones shown here compile as
well and are functionally identical._

{% highlight cpp %}
// [[Rcpp::export]]
arma::sp_mat doubleSparseMatrix(arma::sp_mat m) {
    Rcpp::Rcout << m << std::endl;  // use the i/o code from Armadillo
    arma::sp_mat n = 2*m;
    return n;
}
{% endhighlight %}

First, we create a sparse matrix. We then the function we just showed to
to a minimal (and boring) transformation: we double the values of the matrix.
The key really in the seamless passage of matrix `A` from R down to the C++
code where it is accessed as `m`, and the return of the new matrix `n` which
becomes `B` at the R level.

{% highlight r %}
suppressMessages({
  library(methods)
  library(Matrix)
})
i <- c(1,3:8)              # row indices
j <- c(2,9,6:10)           # col indices
x <- 7 * (1:7)             # values
A <- sparseMatrix(i, j, x = x)
A
{% endhighlight %}



<pre class="output">
8 x 10 sparse Matrix of class &quot;dgCMatrix&quot;
                             
[1,] . 7 . . .  .  .  .  .  .
[2,] . . . . .  .  .  .  .  .
[3,] . . . . .  .  .  . 14  .
[4,] . . . . . 21  .  .  .  .
[5,] . . . . .  . 28  .  .  .
[6,] . . . . .  .  . 35  .  .
[7,] . . . . .  .  .  . 42  .
[8,] . . . . .  .  .  .  . 49
</pre>



{% highlight r %}
B <- doubleSparseMatrix(A) # this will print A from C++
{% endhighlight %}



<pre class="output">
[matrix size: 8x10; n_nonzero: 7; density: 8.75%]

     (0, 1)          7.0000
     (3, 5)         21.0000
     (4, 6)         28.0000
     (5, 7)         35.0000
     (2, 8)         14.0000
     (6, 8)         42.0000
     (7, 9)         49.0000
</pre>



{% highlight r %}
B
{% endhighlight %}



<pre class="output">
8 x 10 sparse Matrix of class &quot;dgCMatrix&quot;
                              
[1,] . 14 . . .  .  .  .  .  .
[2,] .  . . . .  .  .  .  .  .
[3,] .  . . . .  .  .  . 28  .
[4,] .  . . . . 42  .  .  .  .
[5,] .  . . . .  . 56  .  .  .
[6,] .  . . . .  .  . 70  .  .
[7,] .  . . . .  .  .  . 84  .
[8,] .  . . . .  .  .  .  . 98
</pre>



{% highlight r %}
identical(2*A, B)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>
