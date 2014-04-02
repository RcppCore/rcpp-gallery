---
title: Using iterators for sparse vectors and matrices
author: Soren Hojsgaard and Doug Bates
license: GPL (>= 2)
tags: eigen sparse modeling
summary: We illustrate the user of iterators for sparse vectors and matrices and implement a function which determines if a set of nodes in an undirected graph is complete.
layout: post
src: 2014-04-01-sparse-iterators.Rmd
---

## Iterating over a sparse vector

Consider the following vector:

{% highlight r %}
idx1 <- c(2L, 0L, 4L, 0L, 7L)
{% endhighlight %}


A sparse representation of this vector will tell that at entries 1,3,5
(or at entries 0,2,4 if we are 0-based) we will find the values 2,4,7.

Using [Eigen](http://eigen.tuxfamily.org) via
[RcppEigen](http://cran.r-project.org/web/packages/RcppEigen/index.html) we
can obtain the coercion with `.sparseView()`.
We can iterate over all elements (including the zeros) in a sparse
vector as follows:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
typedef Eigen::SparseVector<double> SpVec;

//[[Rcpp::export]]
void vec_loop1 (Eigen::VectorXi idx){
    SpVec sidx = idx.sparseView(); // create sparse vector
    Rcpp::Rcout << "Standard looping over a sparse vector" << std::endl;
    for (int i=0; i<sidx.size();++i){
        Rcpp::Rcout << " i=" << i << " value=" << sidx.coeff( i ) << std::endl;
    }
}
{% endhighlight %}



{% highlight r %}
vec_loop1( idx1 )
{% endhighlight %}



<pre class="output">
Standard looping over a sparse vector
 i=0 value=2
 i=1 value=0
 i=2 value=4
 i=3 value=0
 i=4 value=7
</pre>


To iterate only over the non-zero elements we can do:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
typedef Eigen::SparseVector<double> SpVec;
typedef SpVec::InnerIterator InIterVec;

//[[Rcpp::export]]
void vec_loop2 (Eigen::VectorXi idx){
    SpVec sidx = idx.sparseView();
    Rcpp::Rcout << "Looping over a sparse vector using iterators" << std::endl;
    for (InIterVec i_(sidx); i_; ++i_){
         Rcpp::Rcout << " i=" << i_.index() << " value=" << i_.value() << std::endl;
    }
}
{% endhighlight %}



{% highlight r %}
vec_loop2( idx1 )
{% endhighlight %}



<pre class="output">
Looping over a sparse vector using iterators
 i=0 value=2
 i=2 value=4
 i=4 value=7
</pre>


## Iterating over a sparse matrix


{% highlight r %}
library(Matrix)
{% endhighlight %}



<pre class="output">
Loading required package: methods
</pre>



{% highlight r %}
M1<- new("dgCMatrix"
    , i = c(1L, 2L, 3L, 0L, 2L, 3L, 0L, 1L, 3L, 0L, 
            1L, 2L, 4L, 5L, 3L, 5L, 3L, 4L)
    , p = c(0L, 3L, 6L, 9L, 14L, 16L, 18L)
    , Dim = c(6L, 6L)
    , Dimnames = list(c("a", "b", "c", "d", "e", "f"), 
                      c("a", "b", "c", "d", "e", "f"))
    , x = c(1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3)
    , factors = list()); M1
{% endhighlight %}



<pre class="output">
6 x 6 sparse Matrix of class &quot;dgCMatrix&quot;
  a b c d e f
a . 4 2 5 . .
b 1 . 3 1 . .
c 2 5 . 2 . .
d 3 1 4 . 5 2
e . . . 3 . 3
f . . . 4 1 .
</pre>


To iterate over all values in a column of this matrix we can do:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
using namespace Rcpp;
typedef Eigen::MappedSparseMatrix<double> MSpMat;

// [[Rcpp::export]]
void mat_loop1 (MSpMat X, int j){
    Rcout << "Standard looping over a sparse matrix" << std::endl;
    for (int i=0; i<X.rows(); ++i){
        Rcout << " i,j=" << i << "," << j << " value=" << X.coeff(i,j) << std::endl;
    }
}
{% endhighlight %}



{% highlight r %}
mat_loop1( M1, 1 );
{% endhighlight %}



<pre class="output">
Standard looping over a sparse matrix
 i,j=0,1 value=4
 i,j=1,1 value=0
 i,j=2,1 value=5
 i,j=3,1 value=1
 i,j=4,1 value=0
 i,j=5,1 value=0
</pre>


To iterate over only the non-zero elements in a column we can do:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
using namespace Rcpp;
typedef Eigen::MappedSparseMatrix<double> MSpMat;
typedef MSpMat::InnerIterator InIterMat;
typedef Eigen::SparseVector<double> SpVec;
typedef SpVec::InnerIterator InIterVec;

// [[Rcpp::export]]
void mat_loop2 (MSpMat X, int j){
    Rcout << "Standard looping over a sparse matrix" << std::endl;
    for (InIterMat i_(X, j); i_; ++i_){
        Rcout << " i,j=" << i_.index() << "," << j << " value=" << i_.value() << std::endl;
    }
}
{% endhighlight %}



{% highlight r %}
mat_loop2( M1, 2 );
{% endhighlight %}



<pre class="output">
Standard looping over a sparse matrix
 i,j=0,2 value=2
 i,j=1,2 value=3
 i,j=3,2 value=4
</pre>


# Example from graph theory

A graph with nodes V={1,2,...n} can be reprsented by an adjacency
matrix, say A, with the following semantics: A is n x n. The entry
A(i,j) is non-zero if and only if there is an edge from node i to node
j. If also A(j,i) is non-zero then the edge between i and j is
undirected. Hence if A is symmetric then all edges are undirected, and
the corresponding graph is undirected. In the following we focus on
undirected graphs. If there is an edge between i and j we say that i
and j are neighbours. A subset U of the nodes is complete if all pairs
of nodes in U are neighbours. Here we shall implement a function which
for a sparse matrix representation of and undirected graph will
determine if a given set of nodes is complete.


{% highlight r %}
M1 <- new("dgCMatrix"
    , i = c(1L, 2L, 3L, 0L, 2L, 3L, 0L, 1L, 3L, 0L, 1L, 2L, 4L, 5L, 3L, 5L, 3L, 4L)
    , p = c(0L, 3L, 6L, 9L, 14L, 16L, 18L)
    , Dim = c(6L, 6L)
    , Dimnames = list(c("a", "b", "c", "d", "e", "f"), c("a", "b", "c", "d", "e", "f"))
    , x = c(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
    , factors = list()
); M1
{% endhighlight %}



<pre class="output">
6 x 6 sparse Matrix of class &quot;dgCMatrix&quot;
  a b c d e f
a . 1 1 1 . .
b 1 . 1 1 . .
c 1 1 . 1 . .
d 1 1 1 . 1 1
e . . . 1 . 1
f . . . 1 1 .
</pre>


Define two subsets of nodes

{% highlight r %}
idx1 <- c(2L, 3L)
idx2 <- c(3L, 4L, 5L)
{% endhighlight %}


With an extensive use of sparse matrix and vector iterators we can solve the task as follows:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
using namespace Rcpp;
typedef Eigen::MappedSparseMatrix<double> MSpMat;
typedef MSpMat::InnerIterator InIterMat;
typedef Eigen::SparseVector<double> SpVec;
typedef SpVec::InnerIterator InIterVec;

bool do_is_complete2 (const MSpMat X, SpVec sidx){
    int n = X.cols();
    if (X.rows() != n) throw std::invalid_argument("Sparse matrix X must be square");
    for (InIterVec ii_(sidx); ii_; ++ii_){
        int i0 = ii_.value() - 1;      //Rcpp::Rcout << "i0 = " << i0 << std::endl;
        InIterMat it(X, i0);           // iterator of the i0-column

        for (InIterVec kk_(sidx); kk_; ++kk_){
            int k0 = kk_.value() - 1;    //Rcpp::Rcout << " k0 = " << k0 << ", it.row =";
            if (k0 == i0) continue;
            bool foundit = false;
            for (; it; ++it) {           //Rcpp::Rcout << " " << it.row();
  	        if (it.row() == k0) {
  	           foundit = true;
  	           ++it;
  	           break;
  	        }
  	        if (it.row() > k0) return false;
            }
            if (!foundit) return false;  //Rcpp::Rcout << std::endl;
        }
    }
    return true;
}

//[[Rcpp::export]]
bool is_complete2 (const MSpMat X, const Eigen::VectorXi idx){
    SpVec sidx = idx.sparseView();
    return do_is_complete2( X, sidx );
}
{% endhighlight %}



{% highlight r %}
c( is_complete2( M1, idx1 ), is_complete2( M1, idx2 ) )
{% endhighlight %}



<pre class="output">
[1]  TRUE FALSE
</pre>


For comparison we implement the same function using the .coeff()
method for looking up values in the adjacency matrix directly:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]
#include <RcppEigen.h>
using namespace Rcpp;
typedef Eigen::MappedSparseMatrix<double> MSpMat;
typedef MSpMat::InnerIterator InIterMat;
typedef Eigen::SparseVector<double> SpVec;
typedef SpVec::InnerIterator InIterVec;

bool do_is_complete0 (const MSpMat X, SpVec sidx ){
    int n = X.cols();
    if (X.rows() != n) throw std::invalid_argument("Sparse matrix X must be square");
    int i2, j2;
    for (InIterVec i_(sidx); i_; ++i_){
        i2 = i_.value() - 1;
        for (InIterVec j_(sidx); j_; ++j_){
            j2 = j_.value() - 1 ;
            if (i2>j2)
	        if (X.coeff( i2, j2)==0) return false;
        }
    }
    return true;
}

//[[Rcpp::export]]
bool is_complete0 (const MSpMat X, const Eigen::VectorXi idx){
    SpVec sidx = idx.sparseView();
    return do_is_complete0( X, sidx );
}
{% endhighlight %}



{% highlight r %}
c( is_complete0( M1, idx1 ), is_complete0( M1, idx2 ) )
{% endhighlight %}



<pre class="output">
[1]  TRUE FALSE
</pre>


NOTICE: For large sets U (and hence for large graphs) the first
implementation is considerably faster than the second.
