---
title: Vector Subsetting in Rcpp
author: Kevin Ushey
license: GPL (>= 2)
tags: vector basics
summary: Flexible subsetting was introduced with Rcpp 0.11.1.
layout: post
src: 2014-03-16-subsetting.Rmd
---

Rcpp 0.11.1 has introduced flexible subsetting for Rcpp vectors. Subsetting is
implemented for the Rcpp vector types through the `[` operator, and intends to
mimic R's `[` operator for most cases.

We diverge from R's subsetting semantics in a few important ways:

1. For integer and numeric vectors, 0-based indexing is peformed, rather than
1-based indexing, for subsets.

2. We throw an error if an index is out of bounds, rather than returning an
`NA` value,

3. We require logical subsetting to be with vectors of the same length, thus
avoiding bugs that can occur when a logical vector is recycled for a subset
operation.

Some examples are showcased below:


{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector positives(NumericVector x) {
  return x[x > 0];
}

// [[Rcpp::export]]
List first_three(List x) {
  IntegerVector idx = IntegerVector::create(0, 1, 2);
  return x[idx];
}

// [[Rcpp::export]]
List with_names(List x, CharacterVector y) {
  return x[y];
}
{% endhighlight %}



{% highlight r %}
x <- -5:5
positives(x)
{% endhighlight %}



<pre class="output">
[1] 1 2 3 4 5
</pre>



{% highlight r %}
l <- as.list(1:10)
first_three(l)
{% endhighlight %}



<pre class="output">
[[1]]
[1] 1

[[2]]
[1] 2

[[3]]
[1] 3
</pre>



{% highlight r %}
l <- setNames(l, letters[1:10])
with_names(l, c("a", "e", "g"))
{% endhighlight %}



<pre class="output">
$a
[1] 1

$e
[1] 5

$g
[1] 7
</pre>


Most excitingly, the subset mechanism is quite flexible and works well with Rcpp
sugar. For example:


{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector in_range(NumericVector x, double low, double high) {
  return x[x > low & x < high];
}

// [[Rcpp::export]]
NumericVector no_na(NumericVector x) {
  return x[ !is_na(x) ];
}

bool is_character(SEXP x) {
  return TYPEOF(x) == STRSXP;
}

// [[Rcpp::export]]
List charvecs(List x) {
  return x[ sapply(x, is_character) ];
}
{% endhighlight %}



{% highlight r %}
set.seed(123)
x <- rnorm(5)
in_range(x, -1, 1)
{% endhighlight %}



<pre class="output">
[1] -0.56048 -0.23018  0.07051  0.12929
</pre>



{% highlight r %}
no_na( c(1, 2, NA, 4, NaN, 10) )
{% endhighlight %}



<pre class="output">
[1]  1  2  4 10
</pre>



{% highlight r %}
l <- list(1, 2, "a", "b", TRUE)
charvecs(l)
{% endhighlight %}



<pre class="output">
[[1]]
[1] &quot;a&quot;

[[2]]
[1] &quot;b&quot;
</pre>


And, these can be quite fast:


{% highlight r %}
library(microbenchmark)
R_in_range <- function(x, low, high) {
  return(x[x > low & x < high])
}
x <- rnorm(1E5)
identical( R_in_range(x, -1, 1), in_range(x, -1, 1) )
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
microbenchmark( times=5, 
  R_in_range(x, -1, 1),
  in_range(x, -1, 1)
)
{% endhighlight %}



<pre class="output">
Unit: milliseconds
                 expr   min    lq median    uq   max neval
 R_in_range(x, -1, 1) 4.927 5.163  5.549 5.743 6.274     5
   in_range(x, -1, 1) 2.628 2.694  3.442 3.632 4.775     5
</pre>



{% highlight r %}

R_no_na <- function(x) {
  return( x[!is.na(x)] )
}
x[sample(1E5, 1E4)] <- NA
identical(no_na(x), R_no_na(x))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
microbenchmark( times=5,
  R_no_na(x),
  no_na(x)
)
{% endhighlight %}



<pre class="output">
Unit: microseconds
       expr    min     lq median   uq  max neval
 R_no_na(x) 2313.9 2402.5 2457.8 2677 2952     5
   no_na(x)  822.9  828.8  876.8 1555 1562     5
</pre>


We hope users of Rcpp will find the new subset semantics fast, flexible, and
useful throughout their projects.
