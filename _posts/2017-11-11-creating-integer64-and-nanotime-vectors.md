---
title: "Creating integer64 and nanotime vectors in C++"
author: "Dirk Eddelbuettel"
license: GPL (>= 2)
tags: c++11 data.table nanotime integer64
summary: "Creating integer64 and nanotime vectors in C++, and returning as data.table"
layout: post
src: 2017-11-11-creating-integer64-and-nanotime-vectors.Rmd
---

### Motivation: More Precise Timestamps

R has excellent facilities for dealing with both dates and datetime objects.
For datetime objects, the `POSIXt` time type can be mapped to `POSIXct` and
its representation of fractional seconds since the January 1, 1970 "epoch" as
well as to the broken-out list representation in `POSIXlt`.  Many add-on
packages use these facilities. 

`POSIXct` uses a `double` type to provide 53 bits of resolution.  That is generally
good enough for timestamps down to just above a microsecond, and has served
the R community rather well.

But increasingly, time increments are measured in nanoseconds.  Other languages uses a (signed)
64-bit integer to represent (integer) nanoseconds since the epoch.  A bit over a year ago I realized
that we have this in R too---by combining the `integer64` type in the very nice
[bit64](https://cran.r-project.org/package=bit64) package by Jens Oehlschlaegel with the
[CCTZ](https://github.com/google/cctz)-based parser and formatter in my own
[RcppCCTZ](http://dirk.eddelbuettel.com/code/rcpp.cctz.html) package.  And thus the 
[nanotime](http://dirk.eddelbuettel.com/code/nanotime.html) package was created.

Since then, [Leonardo Silvestri](https://gitlab.com/lsilvest) joined in and significantly enhanced
[nanotime](http://dirk.eddelbuettel.com/code/nanotime.html) by redoing it as a S4 class.

A simple example:


{% highlight r %}
library(nanotime)
n <- nanotime(42)
n
{% endhighlight %}



<pre class="output">
[1] &quot;1970-01-01T00:00:00.000000042+00:00&quot;
</pre>

Here we used a single element with value 42, and created a `nanotime` vector from it---which is
taken to me 42 nanoseconds since the epoch, or basically almost at January 1, 1970. See the 
[nanotime](http://dirk.eddelbuettel.com/code/nanotime.html) page and package for more.


### Step 1: Large Integer Types

So more recently I had a need to _efficiently_ generate such (many such) integer vectors from
`int64_t` data.  Both [Leonardo](https://gitlab.com/lsilvest) and [Dan](https://github.com/dcdillon)
helped with initial discussions and tests.  One can either use a `reinterpret_cast<>`, or a straight
`memcpy` as the key trick in [bit64](https://cran.r-project.org/package=bit64) is to (re-)use the
underlying `int64_t` representation (which we do not have in R) via the 64-bit `double`
representative. Just never access it as a `double`.  So we have the space, we just need to ensure we
copy _the bits_ (_i.e._ actual binary content) rather than their value (when "mapped" to a type).
This leads to the following function to create an `integer64` vector for use in R at the C++ level:


{% highlight cpp %}

#include <Rcpp.h>
                                        
Rcpp::NumericVector makeInt64(std::vector<int64_t> v) {
    size_t len = v.size();
    Rcpp::NumericVector n(len);         // storage vehicle we return them in

    // transfers values 'keeping bits' but changing type
    // using reinterpret_cast would get us a warning
    std::memcpy(&(n[0]), &(v[0]), len * sizeof(double));

    n.attr("class") = "integer64";
    return n;
}
{% endhighlight %}

This uses the standard trick of setting a `class` attribute to set an S3 class.  Now the values in
`v` will return to R (exactly how is treated below), and R will treat the vector as `integer64`
object (provided the [bit64](https://cran.r-project.org/package=bit64) package has been loaded).

As mentioned, `reinterpret_cast<>()` can be used too, but leads to a compiler warning (under
`g++-6`). Per [Matt's](https://xania.org/) excellent [compiler explorer](https://godbolt.org/), both
approaches lead to the same `mov` semantics, so we prefer the variant that does not yell at us.


### Step 2: Nanotime

A `nanotime` vector is creating using an internal `integer64` vector.  So the previous function
almost gets us there. But we need to set the S4 type correctly.  So that needed some extra
work---and the following function seems to do it right:


{% highlight cpp %}
#include <Rcpp.h>

Rcpp::S4 makeNanotime(std::vector<int64_t> v) {
    size_t len = v.size();
    Rcpp::NumericVector n(len);         // storage vehicle we return them in

    // transfers values 'keeping bits' but changing type
    // using reinterpret_cast would get us a warning
    std::memcpy(&(n[0]), &(v[0]), len * sizeof(double));

    // do what needs to be done for the S4-ness: class, and .S3Class
    // this was based on careful reading of .Internal(inspect(nanotime(c(0,1))))
    Rcpp::CharacterVector cl = Rcpp::CharacterVector::create("nanotime");
    cl.attr("package") = "nanotime";
    n.attr(".S3Class") = "integer64";
    n.attr("class") = cl;
    SET_S4_OBJECT(n);

    return Rcpp::S4(n);
}
{% endhighlight %}

This creates a `nanotime` vector as a proper S4 object. As before, we set some class attributes
(though in a nested fashion as S4 is that fancy) and also invoke one R macro.

### Step 3: Returning them R via data.table

The astute reader will have noticed that neither one of the functions presented so far had an
`Rcpp::export` tag.  This is because of their function argument: `int64_t` is not representable
natively by R, which is why we need a workaround.  

[Matt Dowle](https://github.com/mattdowle) has been very helpful in providing excellent support for
`nanotime` in [data.table](https://github.com/Rdatatable/data.table/wiki) (even after we, ahem,
borked it by switching from S3 to S4).  This support was of course relatively straightforward
because [data.table](https://github.com/Rdatatable/data.table/wiki) already had support for the
underlying `integer64`, and we had the additional formatters etc.


{% highlight cpp %}
#include <Rcpp.h>

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins("cpp11")]]

Rcpp::NumericVector makeInt64(std::vector<int64_t> v) {
    size_t len = v.size();
    Rcpp::NumericVector n(len);         // storage vehicle we return them in

    // transfers values 'keeping bits' but changing type
    // using reinterpret_cast would get us a warning
    std::memcpy(&(n[0]), &(v[0]), len * sizeof(double));

    n.attr("class") = "integer64";
    return n;
}

Rcpp::S4 makeNanotime(std::vector<int64_t> v) {
    size_t len = v.size();
    Rcpp::NumericVector n(len);         // storage vehicle we return them in

    // transfers values 'keeping bits' but changing type
    // using reinterpret_cast would get us a warning
    std::memcpy(&(n[0]), &(v[0]), len * sizeof(double));

    // do what needs to be done for the S4-ness: class, and .S3Class
    // this was based on careful reading of .Internal(inspect(nanotime(c(0,1))))
    Rcpp::CharacterVector cl = Rcpp::CharacterVector::create("nanotime");
    cl.attr("package") = "nanotime";
    n.attr(".S3Class") = "integer64";
    n.attr("class") = cl;
    SET_S4_OBJECT(n);

    return Rcpp::S4(n);
}

// [[Rcpp::export]]
Rcpp::DataFrame getDT() {
    std::vector<int64_t> d = { 1L, 1000L, 1000000L, 1000000000L };
    std::vector<int64_t> ns = { 1510442294123456789L, 1510442295123456789L, 
        1510442296123456789L, 1510442297123456789L };
        
    Rcpp::DataFrame df = Rcpp::DataFrame::create(Rcpp::Named("int64s") = makeInt64(d),
                                                 Rcpp::Named("nanos") = makeNanotime(ns));
    df.attr("class") = Rcpp::CharacterVector::create("data.table", "data.frame");
    return(df);
}
{% endhighlight %}

### Example

The following example shows the output from the preceding function:


{% highlight r %}
suppressMessages(library("data.table"))
dt <- getDT()
print(dt)
{% endhighlight %}



<pre class="output">
       int64s                               nanos
1:          1 2017-11-11T23:18:14.123456789+00:00
2:       1000 2017-11-11T23:18:15.123456789+00:00
3:    1000000 2017-11-11T23:18:16.123456789+00:00
4: 1000000000 2017-11-11T23:18:17.123456789+00:00
</pre>



{% highlight r %}
dt[[1]]
{% endhighlight %}



<pre class="output">
integer64
[1] 1          1000       1000000    1000000000
</pre>



{% highlight r %}
dt[[2]]
{% endhighlight %}



<pre class="output">
[1] &quot;2017-11-11T23:18:14.123456789+00:00&quot;
[2] &quot;2017-11-11T23:18:15.123456789+00:00&quot;
[3] &quot;2017-11-11T23:18:16.123456789+00:00&quot;
[4] &quot;2017-11-11T23:18:17.123456789+00:00&quot;
</pre>



{% highlight r %}
diff(dt[[2]]) # here 1e9 nanoseconds between them
{% endhighlight %}



<pre class="output">
integer64
[1] 1000000000 1000000000 1000000000
</pre>

With that we're done for this piece.  Happy `nanotime`-ing from C++!
