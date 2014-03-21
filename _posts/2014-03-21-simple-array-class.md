---
title: A simple array class with specialized linear algebra routines
author: Fabian Scheipl
license: GPL (>= 2)
tags: modeling armadillo
summary: Define a simple array class for implementing Currie/Durban/Eilers "generalized linear array models".  
layout: post
src: 2014-03-21-simple-array-class.Rmd
---

[Currie, Durban and Eilers](http://www.macs.hw.ac.uk/~iain/research/RSSB.2006.pdf) write:

> Data with an array structure are common in statistics, and the design or
> regression matrix for analysis of such data can often be written as a
> Kronecker product. Factorial designs, contingency tables and smoothing of
> data on multidimensional grids are three such general classes of data and
> models. In such a setting, we develop an arithmetic of arrays which allows
> us to define the expectation of the data array as a sequence of nested
> matrix operations on a coefficient array. We show how this arithmetic leads
> to low storage, high speed computation in the scoring algorithm of the
> generalized linear model.

For example, they show that if a design matrix `X` has the Kronecker
structure `X = kronecker(Xd, ..., X2, X1)` with `X<i>` a partial model matrix
with `n<i>` rows and `c<i>` columns, linear functions `X%*%theta` of `X` and
a coefficient vector `theta` can be efficiently computed based only on the
partial model matrices where the entries in the vector `X%*%theta` (with
`nd*...*n2*n1` entries) are the same as the entries in the array with
dimension `c(n1, n2, ..., nd)` returned by `RH(Xd, ... , RH(X2, RH(X1,
Theta))...)`.  
`Theta` is an array with dimensions `c(c1, c2, ..., cd)` containing `theta`
and `RH(X, A)` -- the "rotated H-transform" -- is an operation generalizing
transposed pre-multiplication `t(X %*% A)` of a matrix `A` by a matrix `X` to
the case of higher dimensional array-valued `A`.  

The code below implements a simple array class for numeric arrays and the
rotated H-transform in `RcppArmadillo` and compares the performance to both
the naive straight forward matrix multiplication based on the full model
matrix and an `R`-implementation of `RH()`: 


{% highlight cpp %}
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
using namespace Rcpp ;

/*
******************************************************************************
Offset and Array classes based on code by Romain Francois copied from
http://comments.gmane.org/gmane.comp.lang.r.rcpp/5932 on 2014-01-07.
******************************************************************************
*/

class Offset{
private:
     IntegerVector dim ;

public:
     Offset( IntegerVector dim ) : dim(dim) {}

     int operator()( IntegerVector ind ){
         int ret = ind[0] ;
         int offset = 1 ;
         for(int d=1; d < dim.size(); d++) {
             offset = offset * dim[d-1] ; 
             ret = ret + ind[d] * offset ;
         }
         return ret ;
     } ;
     
     IntegerVector getDims() const {
         return(dim) ;
     };

} ;

class Array : public NumericVector {
private:
     // NumericVector value;
     Offset dims ;

public:
    //Rcpp:as
    Array( SEXP x) : NumericVector(x), 
                     dims( (IntegerVector)((RObject)x).attr("dim") ) {}
    
    Array( NumericVector x,  Offset d ): NumericVector(x), 
                                                dims(d) {}
    
    Array( Dimension d ): NumericVector( d ), dims( d ) {}

    IntegerVector getDims() const {
        return(dims.getDims());
    };
     
    NumericVector getValue()  const {
        return(*((NumericVector*)(this)));
    };
     
     inline double& operator()( IntegerVector ind) {
         int vecind = dims(ind);
         NumericVector value = this->getValue();  
         return value(vecind);
     } ;
     
     // change dims without changing order of elements (!= aperm)
     void resize(IntegerVector newdim) {
         int n = std::accumulate((this->getDims()).begin(), (this->getDims()).end(), 1, 
                                 std::multiplies<int>());
         int nnew = std::accumulate(newdim.begin(), newdim.end(), 1, 
                                 std::multiplies<int>());
         if(n != nnew)  stop("old and new old dimensions don't match.");
         this->dims = Offset(newdim);
     } ;
     
} ;

namespace Rcpp {
    // wrap(): converter from Array to an R array
    template <> SEXP wrap(const Array& A) {
        IntegerVector dims = A.getDims();
        //Dimension dims = A.getDims();
        Vector<REALSXP> x = A;
        x.attr( "dim" ) = wrap(dims);
        return x; 
    }
}

// [[Rcpp::export]]
Array rotate(Array A){
    /* 
    Re-dimension an array from dim to c(dim[-1], dim[1]).
    Example: for a 2*3*4 array, indices 1:24 are shuffled into  
    1 3 5 ... 21 23 2 4 6 ... 20 22 24  
    i.e., a sequence of length prod(dims[-1])=12 from 1 to prod(dims)=24 
    ("baseseq") repeated twice ("space" = dims[1]) and shifted by 1 each time.
    */
     
    IntegerVector dims = A.getDims() ;
    int ndims = dims.size() ;
    int space = dims[0] ;
    int length = std::accumulate(dims.begin(),dims.end(), 1, 
                                 std::multiplies<int>()) / space ;
    IntegerVector baseseq = (seq_len(length) - 1) * space ;
    
    NumericVector old = A.getValue() ; 
    NumericVector ret(space*length) ; 
    
    for(int r=0; r < space; r++) {
        for(int j=0; j < length; j++){
            ret[ r * length + j ] = old[ baseseq[j] + r ] ;
        } ;
    } ;
    
    IntegerVector newdim(ndims) ;
    for(int d=0; d < ndims; d++){
        newdim(d) = dims[d+1] ;
    } ; 
    newdim[ndims-1] = dims[0] ;  
    
   Array rA = Array(ret, Offset(newdim)) ;
   return rA;
}

// [[Rcpp::export]]
Array RH(const arma::mat& X, Array A){
    /* 
    Rotated H-transform of Array A by matrix X.
    H-transform generalizes premultiplication of A by X to array-valued A.
    For A with dimensions (c1, c2, ..., cd) and X with dim=(n, c1),
    H(X, A) is array(X*Aflat, dim=c(n, c2, ..., cd)), where Aflat is 
    array(A, c(c1, c2*c3*..*cd).
    */
    IntegerVector olddims = A.getDims() ;
    int n = A.getDims()[0] ;
    int d = std::accumulate(A.getDims().begin(), A.getDims().end(), 1, 
                                 std::multiplies<int>()) / n ;
    
    arma::mat Amod((A.getValue()).begin(), n, d, false) ;
    arma::vec tmp = vectorise(X * Amod);
     
    IntegerVector newdims = clone(A.getDims());
    newdims[0] = X.n_rows; 
    
    Array ret  = rotate(Array(as<NumericVector>(wrap(tmp)), Offset(newdims)));
    
    return ret ; 
}
{% endhighlight %}


**Set up test case:**

Let's look at a 3-dimensional example where `X = X3 %x% X2 %x% X1` and
each `X<i>` is a B-spline basis over `seq(0, 1, len=n<i>)`:


{% highlight r %}
library(splines)
set.seed(11212)

n1 <- 30; n2 <- 40; n3 <- 50
c1 <- 5; c2 <- 10; c3 <- 15
n <- n1*n2*n3
c <- c1*c2*c3

X1 <- bs(seq(0, 1, len=n1), df=c1)
X2 <- bs(seq(0, 1, len=n2), df=c2)
X3 <- bs(seq(0, 1, len=n3), df=c3)
X <- X3 %x% X2 %x% X1

theta_vec <- runif(c)
Theta <- array(theta_vec, dim=c(c1, c2, c3))

RH_r <- function(X, A){
    ## H-transform:
    A_flat <- array(A, dim=c(dim(A)[1], prod(dim(A)[-1])))
    ret <- array(X %*% A_flat, dim=c(nrow(X), dim(A)[-1]))
    ## Rotate:
    aperm(ret, c(2:length(dim(A)), 1))
}
{% endhighlight %}

Note that `X` is fairly large, with 6 &times; 10<sup>4</sup> rows and 750 columns.

**Check correctness:**

{% highlight r %}
all.equal(
    array(X%*%theta_vec, dim=c(n1, n2, n3)),
    # RH(X3, RH(X2, RH(X1, Theta))):
    Reduce(RH_r,
           list(X3, X2, X1),
           init=Theta,
           right=TRUE))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}

all.equal(
    array(X%*%theta_vec, dim=c(n1, n2, n3)),
    # RH(X3, RH(X2, RH(X1, Theta))):
    Reduce(RH,
           list(X3, X2, X1),
           init=Theta,
           right=TRUE))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



**Check performance:**

{% highlight r %}
library(rbenchmark)
benchmark(
    array(X%*%theta_vec, dim=c(n1, n2, n3)),
    Reduce(RH_r,
           list(X3, X2, X1),
           Theta,
           TRUE),
    Reduce(RH,
           list(X3, X2, X1),
           Theta,
           TRUE),
    replications =  100)[,c(1,3:4)]
{% endhighlight %}



<pre class="output">
                                         test elapsed relative
1 array(X %*% theta_vec, dim = c(n1, n2, n3))  27.252  198.920
3   Reduce(RH, list(X3, X2, X1), Theta, TRUE)   0.137    1.000
2 Reduce(RH_r, list(X3, X2, X1), Theta, TRUE)   0.325    2.372
</pre>


*Note:* An alternative version with proper formula notation can be found [here](http://rpubs.com/fabian-s/arraycpp).
