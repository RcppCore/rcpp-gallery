---
title: Setting Object Attributes
author: Hadley Wickham
license: GPL (>= 2)
tags: basics
summary: Demonstrates setting names and other attributes of R 
  objects from within C++
layout: post
src: 2012-12-10-setting-object-attributes.cpp
---
All R objects have attributes, which can be queried and modified with the
`attr` method. Rcpp also provides a `names()` method for the commonly used
attribute: `attr("names")`. The following code snippet illustrates these
methods:



{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector attribs() {
  NumericVector out = NumericVector::create(1, 2, 3);

  out.names() = CharacterVector::create("a", "b", "c");
  out.attr("my-attr") = "my-value";
  out.attr("class") = "my-class";

  return out;
}
{% endhighlight %}


Here's what the object we created in C++ looks like in R:

{% highlight r %}
attribs()
{% endhighlight %}



<pre class="output">
a b c 
1 2 3 
attr(,"my-attr")
[1] "my-value"
attr(,"class")
[1] "my-class"
</pre>

