---
title: Accessing environments
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics environment
summary: This example shows how to access R environments
layout: post
src: 2013-01-03-accessing-environments.cpp
---

Extending R with C++ code by using Rcpp typically involves function
calls by leveraging the existing `.Call()` interface of the R API.
Passing values back and forth is then done in manner similar to
programming with functions.

However, on occassion it is useful to access enviroments (such as
the global environment). We can also pass environments (which are
first-class datatypes for R) around to instantiate the Rcpp class
`Environment`.

Here we illustrating how to access values from the global environment.

First, let us set some values:




{% highlight r %}
someNumber <<- 42
stooges <<- c("moe", "larry", "curly")
{% endhighlight %}


We can access these values in a C++ function setup with Rcpp:

{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
int checkEnv() {
    // access the global env.
    Environment env = Environment::global_env();
    CharacterVector v = env["stooges"];
    Rcout << "Stooge Nb 2 is: " << v[1] << std::endl;
    return env["someNumber"];
}
{% endhighlight %}


Running the example yields:

{% highlight r %}
checkEnv()
{% endhighlight %}



<pre class="output">
Stooge Nb 2 is: larry
</pre>



<pre class="output">
[1] 42
</pre>

