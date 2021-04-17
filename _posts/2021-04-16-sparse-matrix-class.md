---
title: Constructing a Sparse Matrix Class in Rcpp
author: Zach DeBruine and Dirk Eddelbuettel
license: GPL (>= 2)
tags: sparse s4 matrix
summary: Creating a lightweight sparse matrix class in Rcpp
layout: post
src: 2021-04-16-sparse-matrix-class.Rmd
---





### Introduction

It is no secret that sparse matrix operations are faster in C++ than in R. 
[RcppArmadillo](https://github.com/RcppCore/RcppArmadillo) and [RcppEigen](https://github.com/RcppCore/RcppEigen) do a great job copying sparse matrices from R to C++ and back again. 
But note the word "copy". 
Every time RcppArmadillo converts an R sparse matrix to an `arma::SpMat<T>` object, it has to creates a _deep copy_ due to the difference in representation between dense matrices (usually occupying
contiguous chunks of memory) and sparse matrices (which do not precisely because of _sparsity_). 
Similarly, every time RcppEigen converts an R sparse matrix to an `Eigen::SparseMatrix<T>` object, it also creates a deep copy.

The price of instantiating sparse matrix object is measurable in both time and memory. 
But one of the advantages of types such as `Rcpp::NumericVector` is that they simply re-use the underlying memory of the R object by wrapping around the underlying `SEXP` representation and its
contiguous dense memory layout so no copy is created!
Can we create a sparse matrix class using `Rcpp::NumericVector` and `Rcpp::IntegerVector` that uses them similarly as references rather than actual deep-copy of each element?

### Structure of a `dgCMatrix`

We will focus on the `dgCMatrix` type, the most common form of compressed-sparse-column (CSC) matrix in the [Matrix](http://cloud.r-project.org/package=Matrix) package. 
The `dgCMatrix` class is an S4 object with several slots:


{% highlight r %}
library(Matrix)
set.seed(123)
str(rsparsematrix(5, 5, 0.5), vec.len = 12)
{% endhighlight %}



<pre class="output">
Formal class 'dgCMatrix' [package &quot;Matrix&quot;] with 6 slots
  ..@ i       : int [1:12] 2 4 0 3 4 0 3 4 2 3 0 2
  ..@ p       : int [1:6] 0 2 5 8 10 12
  ..@ Dim     : int [1:2] 5 5
  ..@ Dimnames:List of 2
  .. ..$ : NULL
  .. ..$ : NULL
  ..@ x       : num [1:12] 0.5 -1 0.83 -0.056 2.5 0.24 1.7 1.3 0.55 -1.7 -0.78 1.3
  ..@ factors : list()
</pre>

Here we have:

* Slot `i` is an integer vector giving the row indices of the non-zero values of the matrix
* Slot `p` is an integer vector giving the index of the first non-zero value of each column in `i`
* Slot `x` gives the non-zero elements of the matrix corresponding to rows in `i` and columns delineated by `p`

Realize that `i` and `p` are integer vectors, and `x` is a numeric vector (stored as a `double`), corresponding to `Rcpp::IntegerVector` and `Rcpp::NumericVector`.
That means that we can construct a sparse matrix class in C++ using Rcpp vectors!

### A dgCMatrix reference class for Rcpp

A `dgCMatrix` is simply an S4 object containing integer and double vectors, and Rcpp already has implemented exporters and wrappers for S4 objects, integer vectors, and numeric vectors. 
That makes class construction easy:


{% highlight cpp %}
#include <Rcpp.h>

namespace Rcpp {
    class dgCMatrix {
    public:
        IntegerVector i, p, Dim;
        NumericVector x;
        List Dimnames;

        // constructor
        dgCMatrix(S4 mat) {
            i = mat.slot("i");
            p = mat.slot("p");
            x = mat.slot("x");
            Dim = mat.slot("Dim");
            Dimnames = mat.slot("Dimnames");
        };

        // column iterator
        class col_iterator {
          public:
            int index;
            col_iterator(dgCMatrix& g, int ind) : parent(g) { index = ind; }
            bool operator!=(col_iterator x) { return index != x.index; };
            col_iterator& operator++(int) { ++index; return (*this); };
            int row() { return parent.i[index]; };
            int col() { return column; };
            double& value() { return parent.x[index]; };
          private:
            dgCMatrix& parent;
            int column;
        };
        col_iterator begin_col(int j) { return col_iterator(*this, p[j]); };
        col_iterator end_col(int j) { return col_iterator(*this, p[j + 1]); };
        
    };

    template <> dgCMatrix as(SEXP mat) { return dgCMatrix(mat); }

    template <> SEXP wrap(const dgCMatrix& sm) {
        S4 s(std::string("dgCMatrix"));
        s.slot("i") = sm.i;
        s.slot("p") = sm.p;
        s.slot("x") = sm.x;
        s.slot("Dim") = sm.Dim;
        s.slot("Dimnames") = sm.Dimnames;
        return s;
    }
}
{% endhighlight %}

In the above code, first we create a C++ class for a `dgCMatrix` in the Rcpp namespace with public members corresponding to the slots in an R `Matrix::dgCMatrix`.
Second, we add a constructor for the class that receives an S4 R object (which should be a valid `dgCMatrix` object).
Third, we add a simple forward-only sparse column iterator with read/write access for convenient traversal of non-zero elements in the matrix.
Finally, we use `Rcpp::as` and `Rcpp::wrap` for seamless conversion between R and C++ and back again.

### Using the Rcpp::dgCMatrix class

We can now simply pull a `dgCMatrix` into any Rcpp function thanks to our handy class methods and the magic of `Rcpp::as`.


{% highlight cpp %}
//[[Rcpp::export]]
Rcpp::dgCMatrix R_to_Cpp_to_R(Rcpp::dgCMatrix& mat){
    return mat;
}
{% endhighlight %}

And call the following from R:


{% highlight r %}
library(Matrix)
spmat <- abs(rsparsematrix(100, 100, 0.1))
spmat2 <- R_to_Cpp_to_R(spmat)
all.equal(spmat, spmat2)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

Awesome!

### Passing by reference versus value

Rcpp structures such as `NumericVector` are wrappers around the existing R objects. 
This means that if we modify our sparse matrix in C++, it will modify the original R object. 
For instance:


{% highlight cpp %}
//[[Rcpp::export]]
Rcpp::dgCMatrix Rcpp_square(Rcpp::dgCMatrix& mat){
    for (Rcpp::NumericVector::iterator i = mat.x.begin(); i != mat.x.end(); ++i)
        (*i) *= (*i);
    return mat;
}
{% endhighlight %}
used in 


{% highlight r %}
cat("sum before squaring: ", sum(spmat))
{% endhighlight %}



<pre class="output">
sum before squaring:  801.059
</pre>



{% highlight r %}
spmat2 <- Rcpp_square(spmat)
cat("sum of original object after squaring: ", sum(spmat))
{% endhighlight %}



<pre class="output">
sum of original object after squaring:  1007.97
</pre>



{% highlight r %}
cat("sum of assigned object after squaring: ", sum(spmat2))
{% endhighlight %}



<pre class="output">
sum of assigned object after squaring:  1007.97
</pre>

Notice how the original object AND the returned object are identical, yet they don't point to the same memory address (because of _copy on write_):


{% highlight r %}
tracemem(spmat)
{% endhighlight %}



<pre class="output">
[1] &quot;&lt;0x55f9609a3ae0&gt;&quot;
</pre>



{% highlight r %}
tracemem(spmat2)
{% endhighlight %}



<pre class="output">
[1] &quot;&lt;0x55f960f449f0&gt;&quot;
</pre>

You might further inspect memory addresses within the structure using `.Internal(inspect())` and indeed, you will see the memory addresses are not the same.
What this all means is that we can simply call the function in R and modify the object implicitly without an assignment operator.


{% highlight r %}
set.seed(123)
spmat <- abs(rsparsematrix(100, 100, 0.1))
sum_before <- sum(spmat)
Rcpp_square(spmat)
sum_after <- sum(spmat)
{% endhighlight %}


<pre class="output">
sum_before =  793.861
</pre>



<pre class="output">
sum_after  =  970.174
</pre>

This principle of course applies to other Rcpp classes such as `NumericVector` as well. 
But especially when working with very large sparse matrices, it is useful to avoid deep copies and pass by reference only.
If you do need to operate on a copy of your matrix in C++, there is no reason to be using an Rcpp container when you can be using RcppArmadillo or RcppEigen. 
These libraries are extremely capable and well-documented---and generally give you access to specific sparse Matrix algorithms.

### Sparse iterator class

One of the best ways to read and/or write non-zero values in a sparse matrix is with an iterator. 
A basic column forward iterator with read/write access was presented in the declaration of our sparse matrix class. 
We can use this iterator in a very Armadillo-esque manner to compute things like column sums:


{% highlight cpp %}
//[[Rcpp::export]]
Rcpp::NumericVector Rcpp_colSums(Rcpp::dgCMatrix& mat){
    int n_col = mat.p.size() - 1;
    Rcpp::NumericVector sums(n_col);
    for (int col = 0; col < n_col; col++)
       for (Rcpp::dgCMatrix::col_iterator it = mat.begin_col(col); it != mat.end_col(col); it++)
           sums[col] += it.value();
    return sums;
}
{% endhighlight %}

On the R end:


{% highlight r %}
sums <- Rcpp_colSums(spmat)
all.equal(Matrix::colSums(spmat), sums)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

Great---but is it faster???


{% highlight r %}
library(microbenchmark)
microbenchmark(Rcpp_colSums(spmat), Matrix::colSums(spmat))
{% endhighlight %}



<pre class="output">
Unit: microseconds
                   expr    min      lq     mean  median      uq    max neval cld
    Rcpp_colSums(spmat)  2.510  2.7885  3.32225  3.1355  3.2685 21.842   100  a 
 Matrix::colSums(spmat) 11.068 11.5380 13.26691 11.7775 12.0525 79.324   100   b
</pre>

### Extending the class

There are many ways we can extend `Rcpp::dgCMatrix`. 
For example, we can use `const_iterator`, `row_iterator` and iterators that traverse all values in addition to col_iterator. 
We can also add support for subview copies, dense copies of columns and rows, basic element-wise operations, cross-product, etc. 

We have implemented and documented these methods, and hopefully there will be a Rcpp-extending CRAN package in the not-too-distant future that allows you to seamlessly interface with the dgCMatrix
class. 
For now, see the [Github repo for RcppSparse](https://github.com/zdebruine/RcppSparse) for the in-progress package.
