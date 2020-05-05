---
title: Creating a `data.table` in `Rcpp` 
author: David Zimmermann
license: GPL (>= 2)
tags: data.table
summary: This post shows how to create a data.table (mostly) directly in Rcpp
layout: post
src: 2020-05-05-creating-a-datatable-in-rcpp.Rmd
---


The following is mainly influenced by [Leonardo](https://github.com/lsilvest)'s work and his [MWE](https://github.com/lsilvest/data.table_at_c_level).

### Introduction

`Rcpp` provides the `DataFrame` class, which allows us to pass `data.frame`s between `C++` and `R`.
One of the main drawbacks of base `R` `data.frame`s (and for that matter `tibble`s as well), is that they need to create a copy of the data after each call, which can add up to substantial runtimes for larger datasets.

In contrast, `data.table` modifies the data inplace, which means that no copies of the data are needed, reducing the runtime.
This is one of the main reasons why `data.table` is so [blazingly fast](https://h2oai.github.io/db-benchmark/).

As objects of type `data.table` also inherit from `data.frame`, we can use all `data.frame` functions on a `data.table`.
Internally, however, the two are very different.
The question now is, how can we create a `data.table` efficiently inside of `Rcpp` without the need for a deep copy of the data?

This question is important if the data generation process takes place in `C++`, e.g., parsing, simulating, or otherwise generating data.
In particular, this question came up with the [RITCH](https://github.com/DavZim/RITCH) package, which parses binary ITCH files (financial messages) in `Rcpp` and needed a way for fast data conversion to `R`s `data.table`.

## Very Short Answer

To provide a very short answer, mostly for future reference (a more detailed answer is provided below):

- set the class of the object to `"data.table", "data.frame"` in `Rcpp`


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

## Detailed Answer

A first thought would be to use the `Rcpp::DataFrame` or the `Rcpp::List` class in `Rcpp` to return the data to `R` and then call `data.table::setDT()` on the object to properly convert it to a `data.table`. 
To give an example, lets consider creating a dataset of some data in `Rcpp` and returing it to R as a `Data.Frame`


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

Inspecting the `data.table` code reveals, that internally, `data.table` uses the `setalloccol()` function to convert a `data.frame` to `data.table` after doing some checks.
This can be leveraged in the following way: 

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

use `setalloccol()` to reallocate the data by reference

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

Comparing the two methods, we see that the correct implementation is a lot faster on smaller datasets.
On larger datasets, the differences is less dominant, which hints that `data.table` is able to use the data in place and has no need to copy it.


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
2 create_dt_correct(15, 1000)           10   0.005      1.0     0.005    0.000
1   create_dt_naive(15, 1000)           10   0.009      1.8     0.010    0.001
  user.child sys.child
2          0         0
1          0         0
</pre>



{% highlight r %}
# small dataset
rbenchmark::benchmark(
  create_dt_naive(15, 1e6),
  create_dt_correct(15, 1e6),
  replications = 10
)
{% endhighlight %}



<pre class="output">
                          test replications elapsed relative user.self sys.self
2 create_dt_correct(15, 1e+06)           10   4.220    1.000     4.101    0.121
1   create_dt_naive(15, 1e+06)           10   4.338    1.028     4.236    0.102
  user.child sys.child
2          0         0
1          0         0
</pre>
