---
title: A second example of using Boost
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics boost
summary: This post shows how to use some string functionality from Boost
layout: post
src: 2013-01-15-a-second-boost-example.cpp
---
We introduced [Boost](http://www.boost.org) in a 
[first post doing some integer math](../a-first-boost-example). In this post we want
to look at the very versatile 
[Boost.Lexical_Cast](http://www.boost.org/doc/libs/1_52_0/doc/html/boost_lexical_cast.html)
library to convert text to numbers -- see the 
[Motivation](http://www.boost.org/doc/libs/1_51_0/doc/html/boost_lexical_cast.html#boost_lexical_cast.motivation)
for more.

As before, I should note that I write this post on a machine with [Boost](http://www.boost.org) headers 
in a standard system location. <em>So stuff just works.</em> If you have to install Boost from source, 
and into a non-standard location, you may need to add a <code>-I</code> flag, not unlike how added 
the C++11 flag in [this post](../first-steps-with-C++11) .




{% highlight cpp %}
#include <Rcpp.h>
#include <boost/lexical_cast.hpp>  	// one file, automatically found for me

using namespace Rcpp;

using boost::lexical_cast;
using boost::bad_lexical_cast;
 
// [[Rcpp::export]]
std::vector<double> lexicalCast(std::vector<std::string> v) {

    std::vector<double> res(v.size());

    for (int i=0; i<v.size(); i++) {
        try {
            res[i] = lexical_cast<double>(v[i]);
        } catch(bad_lexical_cast &) {
            res[i] = NA_REAL;
        }
    }

    return res;
}
{% endhighlight %}


This simple program uses the [exceptions idiom we
discussed](../intro-to-exceptions) to branch: when a value cannot
be converted, a `NA` value is inserted.  

We can test the example:


{% highlight r %}
v <- c("1.23", ".4", "1000", "foo", "42", "pi/4")
lexicalCast(v)
{% endhighlight %}



<pre class="output">
[1]    1.23    0.40 1000.00      NA   42.00      NA
</pre>

