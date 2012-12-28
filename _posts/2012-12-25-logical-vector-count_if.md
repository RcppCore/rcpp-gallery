---
title: Using LogicalVector
author: Ross Bennett
license: GPL (>= 2)
tags: sugar
summary: Illustrates the use of logical vectors
layout: post
src: 2012-12-25-logical-vector-count_if.cpp
---



The fact that expressions with binary logical operators 
such as `x < 4` create a logical sugar expression 
(i.e. a LogicalVector type) is very powerful. This enables
one to easily write simple and expressive functions with
a LogicalVector as an argument.
Any of the logical operators `<, <=, >, >=, ==, !=` can be used
to create a logical sugar expression. The Rcpp sugar documentation
has additional examples of using binary logical operators.

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
int count_if(LogicalVector x) {
	int counter = 0;
	for(int i = 0; i < x.size(); i++) {
		if(x[i] == TRUE) {
			counter += 1;
		}
	}
	return counter;
}
{% endhighlight %}


{% highlight r %}
 x <- 1:10
 count_if(x < 4)
{% endhighlight %}



<pre class="output">
[1] 3
</pre>



{% highlight r %}
 count_if(x != 8)
{% endhighlight %}



<pre class="output">
[1] 9
</pre>


A simple function using just C++ and the STL to count the
number of elements in a vector less than a given number could
be written as follows. While this function is simple, the
downside is that additional functions will have to be
written for other logical operators and other types.

{% highlight cpp %}
#include <vector>
#include <functional>
#include <algorithm>

// [[Rcpp::export]]
int count_if_lt(std::vector<double> x, int n) {
	return count_if(x.begin(), x.end(), bind2nd(std::less<double>(), n));
}
{% endhighlight %}


{% highlight r %}
 x <- 1:10
 count_if_lt(x, 4)
{% endhighlight %}



<pre class="output">
[1] 3
</pre>

