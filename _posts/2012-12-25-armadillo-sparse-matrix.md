---
title: Sparse matrix in Armadillo
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo matrix featured
summary: This example shows how to create a sparse matrix in Armadillo
layout: post
src: 2012-12-25-armadillo-sparse-matrix.cpp
---
The Matrix package in R supports sparse matrices, and we can use
the S4 class in Rcpp to attach the different component row indices,
column pointers and value which can then be used to initialize an
Armadillo sparse matrix.

Let's start with creating a sparse matrix.




{% highlight r %}
suppressMessages(library(Matrix))
i <- c(1,3:8) 
j <- c(2,9,6:10) 
x <- 7 * (1:7)
A <- sparseMatrix(i, j, x = x) 
print(A)
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


The following C++ function access the corresponding slots of the
`sparseMatrix` object, and creates a `sp_mat` Armadillo object.

{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export]]
void convertSparse(S4 mat) {         // slight improvement with two non-nested loops

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
    arma::access::rw(res.row_indices) = arma::memory::acquire_chunked<arma::uword>(i.size() + 1);
    arma::arrayops::copy(arma::access::rwp(res.row_indices), i.begin(), i.size() + 1);
    
    // create space for col_ptrs, and copy 
    arma::access::rw(res.col_ptrs) = arma::memory::acquire<arma::uword>(p.size() + 2);
    arma::arrayops::copy(arma::access::rwp(res.col_ptrs), p.begin(), p.size() + 1);

    // important: set the sentinel as well
    arma::access::rwp(res.col_ptrs)[p.size()+1] = std::numeric_limits<arma::uword>::max();
    
    // set the number of non-zero elements
    arma::access::rw(res.n_nonzero) = x.size();

    Rcout << "SpMat res:\n" << res << std::endl;
}
{% endhighlight %}


Running this example shows the same matrix printed to `stdout` by
Armadillo.

{% highlight r %}
convertSparse(A)
{% endhighlight %}



<pre class="output">
SpMat res:
[matrix size: 8x10; n_nonzero: 7; density: 8.75%]

     (0, 1)          7.0000
     (3, 5)         21.0000
     (4, 6)         28.0000
     (5, 7)         35.0000
     (2, 8)         14.0000
     (6, 8)         42.0000
     (7, 9)         49.0000
</pre>


Support for sparse matrix is currently still limited in Armadillo,
but expected to grow.  Likewise, RcppArmadillo does not yet have
converters such as `as<>()` (though the example above does 
essentially everything that is needed) and `wrap()` converters.
But we expect to add these
eventually --- at which point this example will be much simpler.
