---
title: Random number generation
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: rmath rng sugar
summary: This example shows how to use the random numbers generators from R
layout: post
src: 2012-12-24-random-number-generation.cpp
---
R comes with several random number generators supporting the
ability to draw random samples from a wide variety of statistical
distributions.

Rcpp builds on this, and provides access to the same random number
generators, and distributions.  Moreover, thanks to Rcpp sugar,
these can be accessed in a vectorised manner, as we illustrated
in the post [simulating pi](../simulating-pi).



{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix rngCpp(const int N) {
  NumericMatrix X(N, 4);
  X(_, 0) = runif(N);
  X(_, 1) = rnorm(N);
  X(_, 2) = rt(N, 5);
  X(_, 3) = rbeta(N, 1, 1);
  return X;
}
{% endhighlight %}


{% highlight r %}
 set.seed(42)     # setting seed
 M1 <- rngCpp(5)
 M1
{% endhighlight %}



<pre class="output">
       [,1]     [,2]    [,3]    [,4]
[1,] 0.9148  0.04788  2.9967 0.79234
[2,] 0.9371 -1.10460  0.8067 0.38822
[3,] 0.2861  0.53902 -1.7226 0.56423
[4,] 0.8304  0.58021  0.8337 0.02646
[5,] 0.6417 -0.65750  0.4019 0.04242
</pre>



{% highlight r %}

 set.seed(42)	  # resetting seed
 M2 <- cbind( runif(5), rnorm(5), rt(5, 5), rbeta(5, 1, 1))
 M2
{% endhighlight %}



<pre class="output">
       [,1]     [,2]    [,3]    [,4]
[1,] 0.9148  0.04788  2.9967 0.79234
[2,] 0.9371 -1.10460  0.8067 0.38822
[3,] 0.2861  0.53902 -1.7226 0.56423
[4,] 0.8304  0.58021  0.8337 0.02646
[5,] 0.6417 -0.65750  0.4019 0.04242
</pre>



{% highlight r %}
 
 all.equal(M1, M2)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>


The other method of using the R random-number generator is in
<em>scalar</em> mode, one variable and draw at a time. This is very
similar to the description of this API in the
[Writing R Extensions](http://cran.r-project.org/doc/manuals/R-exts.html) 
manual, and provided by Rcpp in the <code>R</code> namespace:

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector rngCppScalar() {
  NumericVector x(4);
  x[0] = R::runif(0,1);
  x[1] = R::rnorm(0,1);
  x[2] = R::rt(5);
  x[3] = R::rbeta(1,1);
  return(x);
}
{% endhighlight %}


{% highlight r %}
 set.seed(42)
 v1 <- rngCppScalar()
 v1
{% endhighlight %}



<pre class="output">
[1] 0.9148 1.5307 1.0510 0.8653
</pre>



{% highlight r %}

 set.seed(42)
 v2 <-c(runif(1), rnorm(1,0,1), rt(1,5), rbeta(1,1,1))
 v2
{% endhighlight %}



<pre class="output">
[1] 0.9148 1.5307 1.0510 0.8653
</pre>



{% highlight r %}

 all.equal(v1, v2)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

