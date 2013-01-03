
{% highlight cpp %}
// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
{% endhighlight %}

---
title: Armadillo subsetting
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: armadillo matrix featured
summary: This example shows how to subset Armadillo matrices
layout: post
src: 2013-01-02-armadillo-subsetting.cpp
---
A [StackOverflow
question](http://stackoverflow.com/questions/10212247/conversion-from-armaumat-to-armamat)
asked how convert from `arma::umat` to `arma::mat`.  The former is
format used for `find` and other logical indexing.

For the particular example at hand, a call to the `conv_to`
converter provided the solution. We rewrite the answer here using
the newer format offered by Rcpp attributes and its `sourceCpp()`
function.




{% highlight cpp %}
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export]]
arma::mat matrixSubset(arma::mat M) {
    // logical conditionL where is transpose larger?
    arma::umat a = trans(M) > M;   
    arma::mat  N = arma::conv_to<arma::mat>::from(a);
    return N;
}
{% endhighlight %}


{% highlight r %}
M <- matrix(1:9, 3, 3)
M
{% endhighlight %}



<pre class="output">
     [,1] [,2] [,3]
[1,]    1    4    7
[2,]    2    5    8
[3,]    3    6    9
</pre>



{% highlight r %}
matrixSubset(M)
{% endhighlight %}



<pre class="output">
     [,1] [,2] [,3]
[1,]    0    0    0
[2,]    1    0    0
[3,]    1    1    0
</pre>


This generalizes to other uses, and the vector or matrix of
unsigned ints can be used inside the `elem()` member function.
Here were we return all values of `M * M'` that are greater or
equal to 100.   


{% highlight cpp %}
// [[Rcpp::export]]
arma::vec matrixSubset2(arma::mat M) {
    arma::mat Z = M * M.t();
    arma::vec v = Z.elem( arma::find( Z >= 100 ) );
    return v;
}
{% endhighlight %}


The result:

{% highlight r %}
matrixSubset2(M)
{% endhighlight %}



<pre class="output">
     [,1]
[1,]  108
[2,]  108
[3,]  126
</pre>

