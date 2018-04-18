---
title: "Performance considerations with sparse matrices in Armadillo"
author: "Hong Ooi"
license: GPL (>= 2)
tags: sparse armadillo matrix
date: 2018-04-18
summary: We discuss performance considerations when working with Armadillo sparse matrices.
layout: post
src: 2018-04-18-armadillo-sparse-matrix-performance.Rmd
---

### Introduction



The Armadillo library provides a great way to manipulate sparse matrices in C++. However, the
performance characteristics of dealing with sparse matrices may be surprising if one is only
familiar with dense matrices. This is a collection of observations on getting best performance with
sparse matrices in Armadillo.

All the timings in this article were generated using Armadillo version 8.500. This version adds a
number of substantial optimisations for sparse matrix operations, in some cases speeding things up
by as much as two orders of magnitude.


### General considerations: sparsity, row vs column access

Perhaps the most important thing to note is that the efficiency of sparse algorithms can depend
strongly on the _level of sparsity_ in the data. If your matrices and vectors are very sparse (most
elements equal to zero), you will often see better performance even if the nominal sizes of those
matrices remain the same. This isn't specific to C++ or Armadillo; it applies to sparse algorithms
in general, including the code used in the Matrix package for R. By contrast, algorithms for working
with dense matrices usually aren't affected by sparsity.

Similarly, the pattern of accesssing elements, whether by rows or by columns, can have a significant
impact on performance. This is due to caching, which modern CPUs use to speed up memory access:
accessing elements that are already in the cache is much faster than retrieving them from main
memory. If one iterates or loops over the elements of a matrix in Armadillo, one should try to
iterate over _columns_ first, then rows, to maximise the benefits of caching. This applies to both
dense and sparse matrices. (Technically, at least for dense matrices, whether to iterate over rows
or columns first depends on how the data is stored in memory. Both R and Armadillo store matrices in
[column-major order](https://en.wikipedia.org/wiki/Row-_and_column-major_order), meaning that
elements in the same column are contiguous in memory. Sparse matrices are more complex but the
advice to iterate by columns is basically the same; see below.)


### Matrix multiplication

We start with a simple concrete example: multiplying two matrices together. In R, this can be done
using the `%*%` operator which (via the Matrix package) is able to handle any combination of sparse
and dense inputs. However, let us assume we want to do the multiplication in Armadillo: for example
if the inputs are from other C++ functions, or if we want more precise control of the output.

In Armadillo, the `*` operator multiplies two matrices together, and this also works for any
combination of sparse and dense inputs. However, the speed of the operation can vary tremendously,
depending on which of those inputs is sparse. To see this, let us define a few simple functions:


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
arma::sp_mat mult_sp_sp_to_sp(const arma::sp_mat& a, const arma::sp_mat& b) {
    // sparse x sparse -> sparse
    arma::sp_mat result(a * b);
    return result;
}

// [[Rcpp::export]]
arma::sp_mat mult_sp_den_to_sp(const arma::sp_mat& a, const arma::mat& b) {
    // sparse x dense -> sparse
    arma::sp_mat result(a * b);
    return result;
}

// [[Rcpp::export]]
arma::sp_mat mult_den_sp_to_sp(const arma::mat& a, const arma::sp_mat& b) {
    // dense x sparse -> sparse
    arma::sp_mat result(a * b);
    return result;
}
{% endhighlight %}

The outputs of these functions are all the same, but they take different types of inputs: either two
sparse matrices, or a sparse and a dense matrix, or a dense and a sparse matrix (the order
matters). Let us call them on some randomly generated data:


{% highlight r %}
library(Matrix)
set.seed(98765)
n <- 5e3
# 5000 x 5000 matrices, 99% sparse
a <- rsparsematrix(n, n, 0.01, rand.x=function(n) rpois(n, 1) + 1)
b <- rsparsematrix(n, n, 0.01, rand.x=function(n) rpois(n, 1) + 1)

a_den <- as.matrix(a)
b_den <- as.matrix(b)

system.time(m0 <- a %*% b)
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.221   0.089   0.310 
</pre>



{% highlight r %}
system.time(m1 <- mult_sp_sp_to_sp(a, b))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.368   0.063   0.431 
</pre>



{% highlight r %}
system.time(m2 <- mult_sp_den_to_sp(a, b_den))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
 15.511   0.089  15.600 
</pre>



{% highlight r %}
system.time(m3 <- mult_den_sp_to_sp(a_den, b))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.910   0.092   1.002 
</pre>



{% highlight r %}
all(identical(m0, m1), identical(m0, m2), identical(m0, m3))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

While all the times are within an order of magnitude of each other, multiplying a dense and a sparse
matrix takes about twice as long as multiplying two sparse matrices together, and multiplying a
sparse and dense matrix takes about three times as long. (The sparse-dense multiply is actually one
area where Armadillo 8.500 makes massive gains over previous versions. This operation used to take
much longer due to using an unoptimised multiplication algorithm.)

Let us see if we can help the performance of the mixed-type functions by creating a temporary sparse
copy of the dense input. This forces Armadillo to use the sparse-sparse version of matrix multiply,
which as seen above is much more efficient. For example, here is the result of tweaking the
dense-sparse multiply. Creating the sparse copy does take some extra time and memory, but not enough
to affect the result.


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
arma::sp_mat mult_sp_den_to_sp2(const arma::sp_mat& a, const arma::mat& b) {
    // sparse x dense -> sparse
    // copy dense to sparse, then multiply
    arma::sp_mat temp(b);
    arma::sp_mat result(a * temp);
    return result;
}
{% endhighlight %}


{% highlight r %}
system.time(m4 <- mult_sp_den_to_sp2(a, b_den))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.442   0.040   0.483 
</pre>



{% highlight r %}
identical(m0, m4)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

This shows that mixing sparse and dense inputs can have significant effects on the efficiency of
your code. To avoid unexpected slowdowns, consider sticking to either sparse or dense classes for
all your objects. If one decides to mix them, it is worth remembering to test and profile the code.


### Row vs column access

Consider another simple computation: multiply the elements of a matrix by their row number, then sum
them up. (The multiply by row number is to make it not _completely_ trivial.) That is, we want to
obtain:

$$\sum_{i=1}^n{\sum_{j=1}^n{ ix_{ij} }}$$

Armadillo lets us extract individual rows and columns from a matrix, using the `.row()` and `.col()`
member functions. We can use row extraction to do this computation:


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
arma::sp_mat row_slice(const arma::sp_mat& x, const int n) {
    return x.row(n - 1);
}
{% endhighlight %}


{% highlight r %}
system.time({
    result <- sapply(1:nrow(a),
        function(i) i * sum(row_slice(a, i)))
    print(sum(result))
})
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  1.873   0.004   1.878 
</pre>

For a large matrix, this takes a not-insignificant amount of time, even on a fast machine. To speed
it up, we will make use of the fact that Armadillo uses the _compressed sparse column_ (CSC) format
for storing sparse matrices.  This means that the data for a matrix is stored as three vectors: the
nonzero elements; the row indices of these elements (ordered by column); and a vector of offsets for
the first row index in each column. Since the vector of row indices is ordered by column, and we
have the starting offsets for each column, it turns out that extracting a column slice is actually
very fast. We only need to find the offset for that column, and then pull out the elements and row
indices up to the next column offset. On the other hand, extracting a row is much more work; we have
to search through the indices to find those matching the desired row.

We can put this knowledge to use on our problem. Row slicing a matrix is the same as transposing it
and then slicing by columns, so let us define a new function to return a column slice. (Transposing
the matrix takes only a tiny fraction of a second.)


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
arma::sp_mat col_slice(const arma::sp_mat& x, const int n) {
    return x.col(n - 1);
}
{% endhighlight %}


{% highlight r %}
a_t <- t(a)
system.time({
    result <- sapply(1:nrow(a_t),
        function(i) i * sum(col_slice(a_t, i)))
    print(sum(result))
})
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  0.818   0.000   0.817 
</pre>

The time taken has come down by quite a substantial margin. This reflects the ease of obtaining
column slices for sparse matrices.


### The R--C++ interface

We can take the previous example further. Each time R calls a C++ function that takes a sparse
matrix as input, it makes a copy of the data. Similarly, when the C++ function returns, its sparse
outputs are copied into R objects. When the function itself is very simple---as it is here---all
this copying and memory shuffling can be a significant proportion of the time taken.

Rather than calling `sapply` in R to iterate over rows, let us do the same entirely in C++:


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
double sum_by_row(const arma::sp_mat& x) {
    double result = 0;
    for (size_t i = 0; i < x.n_rows; i++) {
        arma::sp_mat row(x.row(i));
        for (arma::sp_mat::iterator j = row.begin(); j != row.end(); ++j) {
            result += *j * (i + 1);
        }
    }
    return result;
}
{% endhighlight %}


{% highlight r %}
system.time(print(sum_by_row(a)))
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  0.926   0.000   0.926 
</pre>

This is again a large improvement. But what if we do the same with column slicing?


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
double sum_by_col(const arma::sp_mat& x) {
    double result = 0;
    for (size_t i = 0; i < x.n_cols; i++) {
        arma::sp_mat col(x.col(i));
        for (arma::sp_mat::iterator j = col.begin(); j != col.end(); ++j) {
            result += *j * (i + 1);
        }
    }
    return result;
}
{% endhighlight %}


{% highlight r %}
system.time(print(sum_by_col(a_t)))
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  0.006   0.000   0.006 
</pre>

Now the time is less than a tenth of a second, which is faster than the original code by roughly
three orders of magnitude. The moral to the story is: rather than constantly switching between C++
and R, you should try to stay in one environment for as long as possible. If your code involves a
loop with a C++ call inside (including functional maps like `lapply` and friends), consider writing
the loop entirely in C++ and combine the results into a single object to return to R.

(It should be noted that this interface tax is less onerous for built-in Rcpp classes such as
`NumericVector` or `NumericMatrix`, which do not require making copies of the data. Sparse data
types are different, and in particular Armadillo's sparse classes do not provide constructors that
can directly use auxiliary memory.)


## Iterators vs elementwise access

Rather than taking explicit slices of the data, let us try using good old-fashioned loops over the
matrix elements. This is easily coded up in Armadillo, and it turns out to be quite efficient,
relatively speaking. It is not as fast as using column slicing, but much better than row slicing.


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
double sum_by_element(const arma::sp_mat& x) {
    double result = 0;
    // loop over columns, then rows: see comments at the start of this article
    for (size_t j = 0; j < x.n_cols; j++) {
        for (size_t i = 0; i < x.n_rows; i++) {
            result += x(i, j) * (i + 1);
        }
    }
    return result;
}
{% endhighlight %}


{% highlight r %}
system.time(print(sum_by_element(a)))
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  0.388   0.000   0.388 
</pre>

However, we can still do better. In Armadillo, the iterators for sparse matrix classes iterate only
over the nonzero elements. Let us compare this to our naive double loop:


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

// [[Rcpp::export]]
double sum_by_iterator(const arma::sp_mat& x) {
    double result = 0;
    for (arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i) {
        result += *i * (i.row() + 1);
    }
    return result;
}
{% endhighlight %}


{% highlight r %}
system.time(print(sum_by_iterator(a)))
{% endhighlight %}



<pre class="output">
[1] 1248361320
</pre>



<pre class="output">
   user  system elapsed 
  0.002   0.000   0.002 
</pre>

This is the best time achieved so far, to the extent that `system.time` might have difficulty
capturing it. The timings are now so low that we should use the microbenchmark package to get
accurate measurements:


{% highlight r %}
library(microbenchmark)
microbenchmark(col=sum_by_col(a_t), 
               elem=sum_by_element(a), 
               iter=sum_by_iterator(a),
               times=20)
{% endhighlight %}



<pre class="output">
Unit: milliseconds
 expr       min        lq      mean    median        uq       max neval
  col   5.02286   5.17710   5.34210   5.33575   5.39444   6.02304    20
 elem 389.33550 393.98013 402.67589 403.13064 411.42497 420.33654    20
 iter   1.01472   1.07613   1.19713   1.18075   1.22931   1.74711    20
</pre>

Thus, using iterators represents a greater than three-order-of-magnitude speedup over the original
row-slicing code.
