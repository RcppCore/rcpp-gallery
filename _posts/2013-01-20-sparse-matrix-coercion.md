---
title: Coercion of matrix to sparse matrix (dgCMatrix) and maintaining dimnames.
author: Søren Højsgaard
license: GPL (>= 2)
tags: eigen matrix
summary: We illustrate 1) a fast way of coercing a dense matrix to a sparse matrix and 2) how to copy the dimnames from the dense to the sparse matrix.
layout: post
src: 2013-01-20-sparse-matrix-coercion.Rmd
---


Consider the following matrix


{% highlight r %}
nr <- nc <- 6
set.seed <- 123
m  <- matrix(sample(c(rep(0,9), 1),nr*nc, replace=T), nrow=nr, ncol=nc)
sum(m)/length(m)
{% endhighlight %}



<pre class="output">
[1] 0.1667
</pre>



{% highlight r %}
dimnames(m) <- list(letters[1:nr], letters[1:nc])
m
{% endhighlight %}



<pre class="output">
  a b c d e f
a 0 0 0 0 0 1
b 0 0 0 1 0 1
c 0 0 0 0 0 0
d 0 0 0 0 0 0
e 1 1 0 0 0 0
f 0 0 0 1 0 0
</pre>

This matrix can be coerced to a sparse matrix with


{% highlight r %}
library("Matrix")
{% endhighlight %}



<pre class="output">
Loading required package: methods
</pre>



<pre class="output">
Loading required package: lattice
</pre>



{% highlight r %}
M1 <- as(m, "dgCMatrix")
M1 
{% endhighlight %}



<pre class="output">
6 x 6 sparse Matrix of class "dgCMatrix"
  a b c d e f
a . . . . . 1
b . . . 1 . 1
c . . . . . .
d . . . . . .
e 1 1 . . . .
f . . . 1 . .
</pre>



{% highlight r %}
str(M1)
{% endhighlight %}



<pre class="output">
Formal class 'dgCMatrix' [package "Matrix"] with 6 slots
  ..@ i       : int [1:6] 4 4 1 5 0 1
  ..@ p       : int [1:7] 0 1 2 2 4 4 6
  ..@ Dim     : int [1:2] 6 6
  ..@ Dimnames:List of 2
  .. ..$ : chr [1:6] "a" "b" "c" "d" ...
  .. ..$ : chr [1:6] "a" "b" "c" "d" ...
  ..@ x       : num [1:6] 1 1 1 1 1 1
  ..@ factors : list()
</pre>


Using [Eigen](http://eigen.tuxfamily.org) via
[RcppEigen](http://cran.r-project.org/web/packages/RcppEigen/index.html) we
can obtain the coercion as:


{% highlight cpp %}
// [[Rcpp::depends(RcppEigen)]]

#include <RcppEigen.h>
#include <Rcpp.h>

using namespace Rcpp;
// [[Rcpp::export]]
SEXP asdgCMatrix_( SEXP XX_ ){
  typedef Eigen::SparseMatrix<double> SpMat;   
  typedef Eigen::Map<Eigen::MatrixXd> MapMatd; // Input: must be double
  MapMatd X(Rcpp::as<MapMatd>(XX_));
  SpMat Xsparse = X.sparseView();              // Output: sparse matrix
  S4 Xout(wrap(Xsparse));                      // Output: as S4 object
  NumericMatrix Xin(XX_);                      // Copy dimnames
  Xout.slot("Dimnames") = clone(List(Xin.attr("dimnames")));
  return(Xout);
}
{% endhighlight %}




{% highlight r %}
(M2 <- asdgCMatrix_(m * 1.0))
{% endhighlight %}



<pre class="output">
6 x 6 sparse Matrix of class "dgCMatrix"
  a b c d e f
a . . . . . 1
b . . . 1 . 1
c . . . . . .
d . . . . . .
e 1 1 . . . .
f . . . 1 . .
</pre>



{% highlight r %}
str(M2)
{% endhighlight %}



<pre class="output">
Formal class 'dgCMatrix' [package "Matrix"] with 6 slots
  ..@ i       : int [1:6] 4 4 1 5 0 1
  ..@ p       : int [1:7] 0 1 2 2 4 4 6
  ..@ Dim     : int [1:2] 6 6
  ..@ Dimnames:List of 2
  .. ..$ : chr [1:6] "a" "b" "c" "d" ...
  .. ..$ : chr [1:6] "a" "b" "c" "d" ...
  ..@ x       : num [1:6] 1 1 1 1 1 1
  ..@ factors : list()
</pre>





{% highlight r %}
identical(M1, M2)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>


Compare the performance:


{% highlight r %}
cols <- c("test", "replications", "elapsed", "relative", "user.self", "sys.self")	
rbenchmark::benchmark(asdgCMatrix_(m * 1.0), as(m, "dgCMatrix"),	
                      columns=cols, order="relative", replications=1000)
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative user.self sys.self
1 asdgCMatrix_(m * 1)         1000   0.025     1.00     0.024    0.000
2  as(m, "dgCMatrix")         1000   0.246     9.84     0.240    0.004
</pre>


For larger matrices the difference in performance gain is smaller:


{% highlight r %}
## 100 x 100 matrix
nr <- nc <- 100
set.seed <- 123
m  <- matrix(sample(c(rep(0,9), 1),nr*nc, replace=T), nrow=nr, ncol=nc)
rbenchmark::benchmark(asdgCMatrix_(m * 1.0), as(m, "dgCMatrix"),	
                      columns=cols, order="relative", replications=1000)
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative user.self sys.self
1 asdgCMatrix_(m * 1)         1000   0.137    1.000     0.136    0.000
2  as(m, "dgCMatrix")         1000   0.443    3.234     0.432    0.008
</pre>



{% highlight r %}

## 1000 x 1000 matrix
nr <- nc <- 1000
set.seed <- 123
m  <- matrix(sample(c(rep(0,9), 1),nr*nc, replace=T), nrow=nr, ncol=nc)
rbenchmark::benchmark(asdgCMatrix_(m * 1.0), as(m, "dgCMatrix"),	
                      columns=cols, order="relative", replications=100)
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative user.self sys.self
1 asdgCMatrix_(m * 1)          100   1.193    1.000     1.180    0.008
2  as(m, "dgCMatrix")          100   2.201    1.845     2.064    0.128
</pre>



{% highlight r %}

## 3000 x 3000 matrix
nr <- nc <- 3000
set.seed <- 123
m  <- matrix(sample(c(rep(0,9), 1),nr*nc, replace=T), nrow=nr, ncol=nc)
rbenchmark::benchmark(asdgCMatrix_(m * 1.0), as(m, "dgCMatrix"),	
                      columns=cols, order="relative", replications=100)
{% endhighlight %}



<pre class="output">
                 test replications elapsed relative user.self sys.self
1 asdgCMatrix_(m * 1)          100   8.911    1.000     6.024    2.828
2  as(m, "dgCMatrix")          100  21.557    2.419    16.930    4.500
</pre>


Thanks to Doug Bates for illustrating to me how set the dimnames attribute.
