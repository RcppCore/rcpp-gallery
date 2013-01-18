---
title: Creating xts objects from source
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: basics 
summary: This post shows how to create an xts object at C++ source level
layout: post
src: 2013-01-17-creating-xts-from-c++.cpp
---
A recent post showed how to access the 
[attributes of an xts object](../getting-attributes-for-xts-example/). 
We used an `xts` object as these are powerful and popular---but any R object
using attributed could be used to illustrate the point.

In this short post, we show how one can also do the inverse in
order to _create_ an xts object at the C++ source level.

We use a somewhat useless object with values from `1:10` index by
dates in the same range. As zero corresponds to the epoch, these
will be early 1970-dates. But the values do not matter when showing
the principle.



{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::NumericVector createXts() {

    IntegerVector ind = seq(1, 10);      // values

    NumericVector dv(ind);               // date(time)s are real values
    dv = dv * 86400;                     // scaled to days
    dv.attr("tzone")    = "UTC";         // the index has attributes
    dv.attr("tclass")   = "Date";

    NumericVector xv(ind);               // data her same index
    xv.attr("dim")      = IntegerVector::create(10,1);
    xv.attr("index")    = dv;
    CharacterVector klass = CharacterVector::create("xts", "zoo");
    xv.attr("class")    = klass;
    xv.attr(".indexCLASS") = "Date";
    xv.attr("tclass")   = "Date";
    xv.attr(".indexTZ") = "UTC";
    xv.attr("tzone")   = "UTC";
    
    return xv;

}
{% endhighlight %}


We can run this function, and look at the (numerous) attributes in the generated object:

{% highlight r %}
suppressMessages(library(xts))
foo <- createXts() 
foo
{% endhighlight %}



<pre class="output">
           [,1]
1970-01-02    1
1970-01-03    2
1970-01-04    3
1970-01-05    4
1970-01-06    5
1970-01-07    6
1970-01-08    7
1970-01-09    8
1970-01-10    9
1970-01-11   10
</pre>



{% highlight r %}
attributes(foo)
{% endhighlight %}



<pre class="output">
$dim
[1] 10  1

$index
 [1]  86400 172800 259200 345600 432000 518400 604800 691200 777600 864000
attr(,"tzone")
[1] "UTC"
attr(,"tclass")
[1] "Date"

$class
[1] "xts" "zoo"

$.indexCLASS
[1] "Date"

$tclass
[1] "Date"

$.indexTZ
[1] "UTC"

$tzone
[1] "UTC"
</pre>


It turns out that creating an `xts` object the usual way creates an object that is equal:

{% highlight r %}
bar <- xts(1:10, order.by=as.Date(1:10)) 
all.equal(foo, bar)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>


So now we can create `xts` objects at the source level.  
