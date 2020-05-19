---
title: Nullable Optional Arguments in Rcpp functions
author: Satyaprakash Nayak
license: GPL (>= 2)
tags: nullable
summary: This post shows how to set datatypes as NULL and use them if not NULL
layout: post
src: 2020-05-18-optional-null-function-arguments.Rmd
---



### Introduction

Often we need to have optional arguments in `R` of `Rcpp` functions with default values. Sometimes,
the default value for the optional parameters is set to be `NULL`. `Rcpp` provides the `Nullable <>`
to set default value as to be `R_NilValue` (equivalent of `NULL` in `Rcpp`). There have been several
StackOverflow [posts](https://stackoverflow.com/search?tab=relevance&q=rcpp%20Nullable) on using the
`Nullable` behavior. As seen from quite a few posts, the _key step_ in using `Rcpp::Nullable<>` is
to cast it to the underlying type first (i.e., instantiation) after checking that the input is not
`NULL`.

### Nullability of Vector, Matrix or Logical Vector 


{% highlight cpp %}
// Checking setting Vector, Matrix and LogicalVector to NULL by default and 
// using the input if not set to NULL
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void nullable1(Nullable<NumericVector> NV_ = R_NilValue, 
               Nullable<NumericMatrix> NM_ = R_NilValue, 
               Nullable<LogicalVector> LG_ = R_NilValue){
  
    if (NV_.isNotNull()) {
        NumericVector NV(NV_);        // casting to underlying type NumericVector
        Rcout << "Numeric Vector is set to not NULL." << std::endl;
        Rcout << NV << std::endl;
    } else if (NM_.isNotNull()){
        NumericMatrix NM(NM_);       // casting to underlying type NumericMatrix
        Rcout << "Numeric Matrix is set to not NULL." << std::endl;
        Rcout << NM << std::endl;
    } else if (LG_.isNotNull()){
        LogicalVector LG(LG_);       // casting to underlying type Boolean
        Rcout << "Logical Vector is set to not NULL." << std::endl;
        Rcout << LG << std::endl;
    } else {
        warning("All arguments are set to NULL.\n");
    }
}
{% endhighlight %}

Running a few examples with setting `NULL` for a vector, matrix or a boolean value gives us the
expected results.


{% highlight r %}
nullable1(c(1,2), NULL, NULL)
{% endhighlight %}



<pre class="output">
Numeric Vector is set to not NULL.
1 2
</pre>



{% highlight r %}
m <- matrix(-0.5, 3, 3)
nullable1(NULL, m, NULL)
{% endhighlight %}



<pre class="output">
Numeric Matrix is set to not NULL.
-0.500000 -0.500000 -0.500000
-0.500000 -0.500000 -0.500000
-0.500000 -0.500000 -0.500000
</pre>



{% highlight r %}
nullable1(NULL, NULL, FALSE)
{% endhighlight %}



<pre class="output">
Logical Vector is set to not NULL.
0
</pre>



{% highlight r %}
nullable1(NULL, NULL, NULL)
{% endhighlight %}



<pre class="output">
Warning in nullable1(NULL, NULL, NULL): All arguments are set to NULL.
</pre>



{% highlight r %}
nullable1(c(), NULL, NULL)
{% endhighlight %}



<pre class="output">
Warning in nullable1(c(), NULL, NULL): All arguments are set to NULL.
</pre>

We get the same result when the input to the `NumericVector` argument is not `NULL` but an empty
vector, i.e., `c()`, which is also expected since `is.null(c())` is `TRUE` in `R`.

A stricter test whether the input is usable can be (aptly named) `isUsable()`. 


{% highlight cpp %}
// Testing another check, isUsable for a Nullable Vector
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void nullable2(Nullable<NumericVector> NV_ = R_NilValue) {
  
    if (NV_.isUsable()) {
        NumericVector NV(NV_);        // casting to underlying type NumericVector
        Rcout << "Input is usable." << std::endl;
        Rcout << NV << std::endl;
    } else {
        Rcout << "Input is either NULL or not usable." << std::endl;
    }
}
{% endhighlight %}

### Nullability of DataFrame and List

`Rcpp::Nullable<>` works for `SEXP` based `Rcpp` types, so `Rcpp::DataFrame` and `Rcpp::List` can
also be set to `Nullable` and instantiated if not `NULL`.


{% highlight cpp %}
// Checking setting List and DataFrame to NULL by default and 
// using the input if not set to NULL
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void nullable3(Nullable<List> ls_ = R_NilValue, Nullable<DataFrame> df_ = R_NilValue){

    if (ls_.isNotNull()){
        Rcpp::List ls(ls_);                          // casting to underlying type List
        Rcout << "List is not NULL." << std::endl;
        Rcout << "List length of " << ls.length() << " elements." << std::endl;
    } else if(df_.isNotNull()) {
        Rcpp::DataFrame df(df_);                    // casting to underlying type DataFrame
        Rcout << "DataFrame is not NULL." << std::endl;
        Rcout << "DataFrame of " << df.nrows() << " rows and " << df.length() << " columns." << std::endl;
    } else {
        warning("Both inputs are NULL.\n");
    }
}
{% endhighlight %}

Testing with `Rcpp::List` and `Rcpp::DataFrame` gives expected results, i.e., 


{% highlight r %}
mylist <- list(A = 1:10, B = letters[1:10])
nullable3(mylist, NULL)
{% endhighlight %}



<pre class="output">
List is not NULL.
List length of 2 elements.
</pre>



{% highlight r %}
df  <- data.frame(A = 1:20, B = letters[1:20])
nullable3(NULL, df)
{% endhighlight %}



<pre class="output">
DataFrame is not NULL.
DataFrame of 20 rows and 2 columns.
</pre>

### Nullability of `RcppGSL::Matrix` 

In addition to `Rcpp` types, `RcppGSL::Matrix` can also be set with `Nullable` type (e.g, in the
`mvlabund`
[package](https://github.com/aliceyiwang/mvabund/blob/99cb1ea8420b9d0f97ba68ec818c4751f20fb9a5/src/Rinterface.cpp#L22)):
e.g.,


{% highlight cpp %}
// Checking setting RcppGSL Matrix to NULL by default and 
// using the input if not set to NULL
// [[Rcpp::depends(RcppGSL)]]
#include <RcppGSL.h>

using namespace Rcpp;

// [[Rcpp::export]]
void nullable4(Rcpp::Nullable<RcppGSL::Matrix> M_ = R_NilValue) {
  
    if (M_.isNotNull()){
        RcppGSL::Matrix M(M_);      // casting to underlying type RcppGSL::Matrix
        Rcout << "Input is not NULL." << std::endl;
        Rcout << "Input GSL matrix has " << M.nrow() << " and " << M.ncol() << " columns.\n";
    } else {
        warning("Input GSL Matrix is NULL.\n");
    }
}
{% endhighlight %}

Finally, testing with `RcppGSL::Matrix` which can also be set to `Nullable<>`, i.e., 


{% highlight r %}
nullable4(NULL)  # testing with NULL 
{% endhighlight %}



<pre class="output">
Warning in nullable4(NULL): Input GSL Matrix is NULL.
</pre>



{% highlight r %}
m <- matrix(-0.5, 3, 3) # testing with a non-NULL matrix
nullable4(m)
{% endhighlight %}



<pre class="output">
Input is not NULL.
Input GSL matrix has 3 and 3 columns.
</pre>

### Summary

`Rcpp` provides a convenient construct to set datatypes to `NULL` using `R_NilValue` and application
of the datatype if not set to `NULL` using the `.isNotNull()` check. This construct to applied to
set datatypes to `NULL` as default values and possible simple simplification.
