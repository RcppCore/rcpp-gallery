---
title: Creating as and wrap for sparse matrices
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo matrix sparse
summary: This post shows an example of simple as and wrap methods
layout: post
src: 2013-08-05-as-and-wrap-for-sparse-matrices.cpp
---
An [earlier article discussed sparse matrix conversion](http://gallery.rcpp.org/articles/armadillo-sparse-matrix)
but stopped short of showing how to create custom `as<>()` and `wrap()` methods
or functions.  This post starts to close this gap.

We will again look at sparse matrices from the 
[Matrix](http://cran.r-project.org/package=Matrix) package for R, as well as 
the `SpMat` class from [Armadillo](http://arma.sf.net).  
At least for now we will limit outselves to the
case of `double` element types. These uses the `sp_mat` typedef which will be
our basic type for sparse matrices at the C++ level.

At the time of writing, this code had just been added to the SVN repo `RcppArmadillo`
as an extension header file `spmat.h`. Further integration is planned, but no concrete 
steps are planned just yet.

First, we look at the `as` method.



{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]

#include <RcppArmadillo.h>

namespace Rcpp {
    
    // converts an SEXP object from R which was created as a sparse
    // matrix via the Matrix package) into an Armadillo sp_mat matrix
    //
    // TODO: template'ize to allow for types other than double, though
    //       realistically this is all we need
    template <> arma::sp_mat as(SEXP sx) {
        S4 mat(sx);  
        IntegerVector dims = mat.slot("Dim");
        arma::urowvec i = Rcpp::as<arma::urowvec>(mat.slot("i"));
        arma::urowvec p = Rcpp::as<arma::urowvec>(mat.slot("p"));     
        arma::vec x     = Rcpp::as<arma::vec>(mat.slot("x"));
        
        int nrow = dims[0], ncol = dims[1];
        arma::sp_mat res(nrow, ncol);

        // create space for values, and copy
        arma::access::rw(res.values) = arma::memory::acquire_chunked<double>(x.size() + 1);
        arma::arrayops::copy(arma::access::rwp(res.values), x.begin(), x.size() + 1);

        // create space for row_indices, and copy 
        arma::access::rw(res.row_indices) = 
            arma::memory::acquire_chunked<arma::uword>(i.size() + 1);
        arma::arrayops::copy(arma::access::rwp(res.row_indices), i.begin(), i.size() + 1);
    
        // create space for col_ptrs, and copy 
        arma::access::rw(res.col_ptrs) = arma::memory::acquire<arma::uword>(p.size() + 2);
        arma::arrayops::copy(arma::access::rwp(res.col_ptrs), p.begin(), p.size() + 1);

        // important: set the sentinel as well
        arma::access::rwp(res.col_ptrs)[p.size()+1] = std::numeric_limits<arma::uword>::max();
    
        // set the number of non-zero elements
        arma::access::rw(res.n_nonzero) = x.size();

        return res;
    }

}
{% endhighlight %}


Next, we look at the corresponding `wrap()` method.

{% highlight cpp %}
namespace Rcpp {

    // convert an Armadillo sp_mat into a corresponding R sparse matrix
    // we copy to STL vectors as the Matrix package expects vectors whereas the
    // default wrap in Armadillo returns matrix with one row (or col) 
    SEXP wrap(arma::sp_mat sm) {

        IntegerVector dim(2);
        dim[0] = sm.n_rows; 
        dim[1] = sm.n_cols;

        arma::vec  x(sm.n_nonzero);        // create space for values, and copy
        arma::arrayops::copy(x.begin(), sm.values, sm.n_nonzero);
        std::vector<double> vx = arma::conv_to< std::vector< double > >::from(x);

        arma::urowvec i(sm.n_nonzero);	// create space for row_indices, and copy & cast
        arma::arrayops::copy(i.begin(), sm.row_indices, sm.n_nonzero);
        std::vector<int> vi = arma::conv_to< std::vector< int > >::from(i);
 
        arma::urowvec p(sm.n_cols+1);	// create space for col_ptrs, and copy 
        arma::arrayops::copy(p.begin(), sm.col_ptrs, sm.n_cols+1);
        // do not copy sentinel for returning R
        std::vector<int> vp = arma::conv_to< std::vector< int > >::from(p);

        S4 s("dgCMatrix");
        s.slot("i")   = vi;
        s.slot("p")   = vp;
        s.slot("x")   = vx;
        s.slot("Dim") = dim;
        return s;
    }

}
{% endhighlight %}


We can now illustrate this with a simple example.

{% highlight cpp %}
// [[Rcpp::export]]
arma::sp_mat doubleSparseMatrix(arma::sp_mat m) {
    Rcpp::Rcout << m << std::endl;  // use the i/o from Armadillo
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
suppressMessages(library(Matrix))
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

