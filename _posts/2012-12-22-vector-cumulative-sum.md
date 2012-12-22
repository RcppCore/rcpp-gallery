---
title: Using Sugar Function cumsum()
author: Ross Bennett
license: GPL (>= 2)
tags: sugar
summary: Demonstrates different ways to compute the cumulative
  sum of a vector and illustrates the use of sugar function cumsum().
layout: post
src: 2012-12-22-vector-cumulative-sum.cpp
---



The traditional way to compute the cumulative sum of a vector is with a
for loop. This is demonstrated with the function cumsum1().

{% highlight cpp %}
#include <Rcpp.h>
#include <numeric>   	// for std::partial_sum
using namespace Rcpp;
 
// [[Rcpp::export]]
NumericVector cumsum1(NumericVector x){
 	// initialize an accumulator variable
   double acc = 0;
 	
 	// initialize the result vector
 	NumericVector res(x.size());
 	
 	for(int i = 0; i < x.size(); i++){
 		acc += x[i];
 		res[i] = acc;
 	}
 	return res;
}
{% endhighlight %}


The C++ standard template library (STL) has the partial_sum() function
that computes the cumulative sum of a vector. This is demonstrated with
the function cumsum2().

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector cumsum2(NumericVector x){
 	// initialize the result vector
 	NumericVector res(x.size());
 	std::partial_sum(x.begin(), x.end(), res.begin());
 	return res;
 }
{% endhighlight %}


With Rcpp sugar, there is a cumsum() function which makes writing 
this function in C++ very similar to using the cumsum function in R.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector cumsum_sug(NumericVector x){
 	// initialize the result vector
 	NumericVector res = cumsum(x);
 	return res;
}
{% endhighlight %}


{% highlight r %}
 x <- 1:10
 cumsum1(x)
{% endhighlight %}



<pre class="output">
 [1]  1  3  6 10 15 21 28 36 45 55
</pre>



{% highlight r %}
 cumsum2(x)
{% endhighlight %}



<pre class="output">
 [1]  1  3  6 10 15 21 28 36 45 55
</pre>



{% highlight r %}
 cumsum_sug(x)
{% endhighlight %}



<pre class="output">
 [1]  1  3  6 10 15 21 28 36 45 55
</pre>



{% highlight r %}
 # cumsum function in base R
 cumsum(x)
{% endhighlight %}



<pre class="output">
 [1]  1  3  6 10 15 21 28 36 45 55
</pre>

