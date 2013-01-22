---
title: Convert a list of lists into a data frame
author: John Merrill
license: GPL (>= 2)
tags: dataframe benchmark featured
summary: This post shows one method for creating a data frame quickly
layout: post
src: 2013-01-22-faster-data-frame-creation.cpp
---

Data frames are one of R's distinguishing features.  Exposing a
list of lists as an array of cases, they make many formal
operations such as regression or optimization easy to represent.

The R data.frame operation for lists is quite slow, in large part
because it exposes a vast amount of functionality.  This sample
shows one way to write a much faster data.frame creator in C++ if
one is willing to forego that generality.



{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
List CheapDataFrameBuilder(List a) {
    List returned_frame = clone(a);
    GenericVector sample_row = returned_frame(0);

    StringVector row_names(sample_row.length());
    for (int i = 0; i < sample_row.length(); ++i) {
        char name[5];
        sprintf(&(name[0]), "%d", i);
        row_names(i) = name;
    }
    returned_frame.attr("row.names") = row_names;

    StringVector col_names(returned_frame.length());
    for (int j = 0; j < returned_frame.length(); ++j) {
        char name[6];
        sprintf(&(name[0]), "X.%d", j);
        col_names(j) = name;
    }
    returned_frame.attr("names") = col_names;
    returned_frame.attr("class") = "data.frame";

    return returned_frame;
}
{% endhighlight %}


Here is the result of comparing the native function to this version.

{% highlight r %}
library(rbenchmark)
a <- replicate(250, 1:100, simplify=FALSE)

res <- benchmark(as.data.frame(a), 
                 CheapDataFrameBuilder(a), 
                 order="relative", replications=500)
res[,1:4]
{% endhighlight %}



<pre class="output">
                      test replications elapsed relative
2 CheapDataFrameBuilder(a)          500   0.106      1.0
1         as.data.frame(a)          500  16.888    159.3
</pre>


There are some subtleties in this code:

--- It turns out that one can't send super-large data frames to it
because of possible buffer overflows.  I've never seen that
problem when I've written Rcpp functions which exchanged SEXPs
with R, but this one uses Rcpp:export in order to use
sourceCpp.

--- Notice the invocation of clone() in the first line of the code.
If you don't do that, you wind up side-effecting the parameter,
which is not what most people would expect.
