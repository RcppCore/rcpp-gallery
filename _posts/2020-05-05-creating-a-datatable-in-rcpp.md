---
title: Creating a data.table from C++
author: David Zimmermann, Leonardo Silvestri, Dirk Eddelbuettel
license: GPL (>= 2)
tags: data.table
summary: This post shows how to create a data.table (mostly) directly in Rcpp
layout: post
src: 2020-05-05-creating-a-datatable-in-rcpp.Rmd
---



### Introduction

`Rcpp` provides the `DataFrame` class which enables us to pass `data.frame` object between C++ and
R. `DataFrame` objects are key to R and used very widely.  They also provide the basis from which
two key packages extend them. One of these, the `tibble` package, operates in a similar fashion to
`data.frame` and treats the data as _immutable_. Upon a change to the data, a new version is created
internally---this is frequently referred to as "copy-on-write". Immutable data structures have some
desirable properties in terms of reasoning with and about data, but the copying comes at a price in
terms of performance especially once data becomes sizeable.

The other package, `data.table`, offers a contrasting approach. It generally modifies the data
_in-place_; this is often referred to as _by reference_. As (generally) no copies of the data are
needed, the runtime is often reduced.  This design difference is (along with _a lot_ of attention to
performance and optimisations in general) one of the reasons why `data.table` is so [looking rather
attractive](https://h2oai.github.io/db-benchmark/) in benchmarks.

As objects of type `data.table` also inherit from `data.frame`, we can use all `data.frame`
functions on a `data.table`.  Internally, however, the two are very different.  The question now is,
how can we create a `data.table` efficiently inside of `Rcpp` without the need for a deep copy of
the data?

This question is important if the data generation process takes place in `C++`, _e.g._ when parsing,
simulating, or otherwise generating data.  One example for which question came up is the
[RITCH](https://github.com/DavZim/RITCH) package which parses (large) binary ITCH files (financial
messages) in `Rcpp` and needs a fast data conversion to `data.table`.

## Very Short Answer

We first provide a very short answer, mostly for future reference. A more detailed answer is
provided below. 

Essentially two steps have to be taken:

- set the S3 class of the object to `"data.table", "data.frame"` in `Rcpp`


{% highlight cpp %}
Rcpp::List create_data_table() {
    Rcpp::List res;
  
    // Populate the list
    // ...
  
    res.attr("class") = Rcpp::CharacterVector::create("data.table", "data.frame");
    return res;
}
{% endhighlight %}

- in R call the `data.table::setalloccol(df)` function on the "partial" `data.table`


{% highlight r %}
foo <- function() {
    dt <- create_data_table()
    dt <- data.table::setalloccol(dt)
    return(dt)
}
{% endhighlight %}
A look inside the `data.table::setalloccol()` functions shows that it itself calls only a few
`data.table` functions needed to do some housekeeping and optimisation on internal state. It would
be useful to access these functions directly from another package at the C level, and we plan to
discuss this with the `data.table` team.


## Detailed Answer

A first thought would be to use the `Rcpp::DataFrame` or the `Rcpp::List` class in `Rcpp` to return
the data to `R` and then call `data.table::setDT()` on the object to properly convert it to a
`data.table`.  To give an example, lets consider creating a dataset of some data in `Rcpp` and
returing it to R as a `Data.Frame`


{% highlight cpp %}
#include<Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
DataFrame simulate_data(int n = 10, int nobs = 10000) {
    // create a list with n slots
    List res(n);
    CharacterVector list_names = CharacterVector(n);
  
    // populate the list elements
    for (int i = 0; i < n; i++) {
        list_names[i] = "col_" + std::to_string(i);
        res[i] = rnorm(nobs);
    }
  
    // set the names for the list elements
    res.names() = list_names;
    return res;
}
{% endhighlight %}

Using `setDT()`, we can convert the list to a `data.table`:


{% highlight r %}
create_dt_naive <- function(n = 10, nobs = 10000) {
    dt <- simulate_data(n, nobs)
    data.table::setDT(dt)
    return(dt)
}
set.seed(123)
dt <- create_dt_naive()
str(dt)
{% endhighlight %}



<pre class="output">
Classes 'data.table' and 'data.frame':	10000 obs. of  10 variables:
 $ col_0: num  -0.5605 -0.2302 1.5587 0.0705 0.1293 ...
 $ col_1: num  2.371 -0.167 0.927 -0.568 0.225 ...
 $ col_2: num  -0.836 -0.221 -2.104 -1.668 -1.098 ...
 $ col_3: num  -0.194 0.258 -0.538 -1.179 0.901 ...
 $ col_4: num  0.4825 0.7214 -0.5078 -0.0647 1.3021 ...
 $ col_5: num  0.26 0.918 -0.722 -0.808 -0.141 ...
 $ col_6: num  -0.3883 0.0274 -0.2761 -0.0867 2.1477 ...
 $ col_7: num  0.414 -0.641 0.281 -0.694 -0.367 ...
 $ col_8: num  0.783 -0.424 -0.844 0.876 1.125 ...
 $ col_9: num  0.5732 0.0183 -0.022 -0.4278 -0.4776 ...
 - attr(*, &quot;.internal.selfref&quot;)=&lt;externalptr&gt; 
</pre>

Inspecting the `data.table` code reveals that internally, `data.table` uses the `setalloccol()`
function to convert a `data.frame` to `data.table` after doing some checks.  This can be leveraged
in the following way:

Set the class attribute of the list to `"data.table", "data.frame"`


{% highlight cpp %}
#include<Rcpp.h>
using namespace Rcpp;
// [[Rcpp::export]]
List simulate_data_partial_dt(int n = 10, int nobs = 10000) {
    // create a list with n slots
    List res(n);
    CharacterVector list_names = CharacterVector(n);
  
    // populate the list elements
    for (int i = 0; i < n; i++) {
        list_names[i] = "col_" + std::to_string(i);
        res[i] = rnorm(nobs);
    }
    res.names() = list_names;
    // Set the class attribute of the list
    res.attr("class") = CharacterVector::create("data.table", "data.frame");
    return res;
}
{% endhighlight %}

and then use `setalloccol()` to reallocate the data by reference


{% highlight r %}
create_dt_correct <- function(n = 10, nobs = 10000) {
    dt <- simulate_data_partial_dt(n, nobs)
    # reallocate the data.table by reference
    dt <- data.table::setalloccol(dt)
    return(dt)
}
set.seed(123)
dt2 <- create_dt_correct()
str(dt2)
{% endhighlight %}



<pre class="output">
Classes 'data.table' and 'data.frame':	10000 obs. of  10 variables:
 $ col_0: num  -0.5605 -0.2302 1.5587 0.0705 0.1293 ...
 $ col_1: num  2.371 -0.167 0.927 -0.568 0.225 ...
 $ col_2: num  -0.836 -0.221 -2.104 -1.668 -1.098 ...
 $ col_3: num  -0.194 0.258 -0.538 -1.179 0.901 ...
 $ col_4: num  0.4825 0.7214 -0.5078 -0.0647 1.3021 ...
 $ col_5: num  0.26 0.918 -0.722 -0.808 -0.141 ...
 $ col_6: num  -0.3883 0.0274 -0.2761 -0.0867 2.1477 ...
 $ col_7: num  0.414 -0.641 0.281 -0.694 -0.367 ...
 $ col_8: num  0.783 -0.424 -0.844 0.876 1.125 ...
 $ col_9: num  0.5732 0.0183 -0.022 -0.4278 -0.4776 ...
 - attr(*, &quot;.internal.selfref&quot;)=&lt;externalptr&gt; 
</pre>



{% highlight r %}
all.equal(dt, dt2)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

Comparing the two methods, we see that the second implementation is a lot faster on smaller datasets.

On larger datasets, the differences is less dominant, which hints that `data.table` is able to use
the data in place and has no need to copy it.


{% highlight r %}
# small dataset
rbenchmark::benchmark(
    create_dt_naive(15, 1e3),
    create_dt_correct(15, 1e3),
    replications = 10
)
{% endhighlight %}



<pre class="output">
                         test replications elapsed relative user.self sys.self
2 create_dt_correct(15, 1000)           10   0.004     1.00     0.004    0.000
1   create_dt_naive(15, 1000)           10   0.011     2.75     0.009    0.001
  user.child sys.child
2          0         0
1          0         0
</pre>



{% highlight r %}
# larger dataset
rbenchmark::benchmark(
    create_dt_naive(15, 1e6),
    create_dt_correct(15, 1e6),
    replications = 10
)
{% endhighlight %}



<pre class="output">
                          test replications elapsed relative user.self sys.self
2 create_dt_correct(15, 1e+06)           10    3.92     1.00     3.820    0.100
1   create_dt_naive(15, 1e+06)           10    4.00     1.02     3.923    0.077
  user.child sys.child
2          0         0
1          0         0
</pre>

R 4.0.0 brings a new function `list2DF()` which we could consider as well.

Note: This post draws upon, and extends, an earlier writeup [at his
repo](https://github.com/lsilvest/data.table_at_c_level).

