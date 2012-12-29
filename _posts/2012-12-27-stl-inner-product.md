
{% highlight cpp %}
// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
{% endhighlight %}

---
title: STL Inner Product
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl
summary: This example shows how to use the STL's inner product
layout: post
src: 2012-12-27-stl-inner-product.cpp
---
The STL contains a large number of useful functions and algorithm.
One useful function is `inner_product` which can be used to compute
the sum oof the elements of two vectors.



{% highlight cpp %}
#include <Rcpp.h>
#include <numeric>

// [[Rcpp::export]]
double innerProduct(const std::vector<double>& x, 
                     const std::vector<double>& y) {
    double val = std::inner_product(x.begin(), x.end(), y.begin(), 0);
    return val;
}
{% endhighlight %}


{% highlight r %}
  x <- c(1,2,3)
  y <- c(4,5,6)
  cbind(x,y)
{% endhighlight %}



<pre class="output">
     x y
[1,] 1 4
[2,] 2 5
[3,] 3 6
</pre>



{% highlight r %}

  innerProduct(x, y)
{% endhighlight %}



<pre class="output">
[1] 32
</pre>



{% highlight r %}
  sum(x*y)  # check from R
{% endhighlight %}



<pre class="output">
[1] 32
</pre>




{% highlight r %}
  innerProduct(x, x)
{% endhighlight %}



<pre class="output">
[1] 14
</pre>



{% highlight r %}
  sum(x^2)
{% endhighlight %}



<pre class="output">
[1] 14
</pre>

