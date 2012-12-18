---
title: Modifying a Data Frame
author: J.J. Allaire
tags: dataframes stl
summary: Demonstrates modifying a data frame passed to a function and 
  returning the modified version.
layout: post
src: 2012-12-14-modifying-a-data-frame.cpp
---
Data frames can be manipulated using the `DataFrame` class. The
indvidiual vectors composing a data frame can be accessed by name, 
modified, and then recombined into a new data frame.

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
DataFrame modifyDataFrame(DataFrame df) {

  // access the columns
  Rcpp::IntegerVector a = df["a"];
  Rcpp::CharacterVector b = df["b"];
  
  // make some changes
  a[2] = 42;
  b[1] = "foo";       

  // return a new data frame
  return DataFrame::create(_["a"]= a, _["b"]= b);
}
{% endhighlight %}


Note the use of the `_["a"]` syntax to create named arguments to the 
`DataFrame::create` function.

The function returns a modified copy of the data frame:

{% highlight r %}
df <- data.frame(a = c(1, 2, 3), b = c("x", "y", "z"))

modifyDataFrame(df)
{% endhighlight %}



<pre class="output">
##    a   b
## 1  1   x
## 2  2 foo
## 3 42   z
</pre>

