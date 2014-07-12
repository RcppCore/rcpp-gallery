---
title: Using bigmemory with Rcpp
author: Michael Kane and Scott Ritchie
license: GPL (>= 2)
tags: basics boost
summary: This post shows how to access big.matrix objects from Rcpp
layout: post
src: 2013-03-14-using-bigmemory-with-rcpp.Rmd
---

The [bigmemory package](http://www.bigmemory.org) allows users to create
matrices that can be shared across R sessions. These can either be stored in
RAM, or stored on disk, allowing for the matrices to be much larger than the
system RAM. These objects are defined in the `big.matrix` class, and are
designed to have similar behaviour to native R matrices. However, they are
implemented in C++ and can be easily accessed and manipulated from `Rcpp`.

This example demonstrates how to write functions in `Rcpp` that operate on
`big.matrix` objects. Here we implement the equivalent of `colSums`:

{% highlight cpp %}
// Since bigmemory now accesses Boost headers from the BH package,
// we need to make sure we do so as well in this Rcpp::depends comment 
// when including both the Rcpp and bigmemory header files.

// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
using namespace Rcpp;

// This next line is all it takes to find the bigmemory 
// headers -- thanks to the magic of Rcpp attributes.
// [[Rcpp::depends(BH, bigmemory)]]
#include <bigmemory/MatrixAccessor.hpp>

#include <numeric>

// You will also need to add the following to your DESCRIPTION file if
// you're developing a package so that it will compile without warnings:
// SystemRequirements: C++11

// Logic for BigColSums.
template <typename T>
NumericVector BigColSums(XPtr<BigMatrix> pMat, MatrixAccessor<T> mat) {
    // Create the vector we'll store the column sums in.
    NumericVector colSums(pMat->ncol());
    for (size_t i=0; i < pMat->ncol(); ++i)
        colSums[i] = std::accumulate(mat[i], mat[i]+pMat->nrow(), 0.0);
    return colSums;
}

// Dispatch function for BigColSums
//
// [[Rcpp::export]]
NumericVector BigColSums(SEXP pBigMat) {
    // First we have to tell Rcpp what class to use for big.matrix objects.
    // This object stores the attributes of the big.matrix object passed to it
    // by R.
    XPtr<BigMatrix> xpMat(pBigMat);

    // To access values in the big.matrix, we need to create a MatrixAccessor
    // object of the appropriate type. Note that in every case we are still
    // returning a NumericVector: this is because big.matrix objects only store
    // numeric values in R, even if their type is set to 'char'. The types
    // simply correspond to the number of bytes used for each element.
    switch(xpMat->matrix_type()) {
      case 1:
        return BigColSums(xpMat, MatrixAccessor<char>(*xpMat));
      case 2:
        return BigColSums(xpMat, MatrixAccessor<short>(*xpMat));
      case 4:
        return BigColSums(xpMat, MatrixAccessor<int>(*xpMat));
      case 8:
        return BigColSums(xpMat, MatrixAccessor<double>(*xpMat));
      default:
        // This case should never be encountered unless the implementation of
        // big.matrix changes, but is necessary to implement shut up compiler
        // warnings.
        throw Rcpp::exception("unknown type detected for big.matrix object!");
    }
}
{% endhighlight %}

When accessing a `big.matrix` object in Rcpp, there are two objects you are
interested in creating. First, the External Pointer for the `big.matrix`;
`XPtr<BigMatrix>`, which also stores all of the attributes of the `big.matrix`,
including `nrow()`, `ncol()`, and `matrix_type()`. The second is the
`MatrixAccessor` which allows you to access elements within the `big.matrix`.
When creating the `MatrixAccessor` you must declare the `type` of the
`BigMatrix`, resulting in the design pattern above to correctly handle all
cases.

A `BigMatrix` object stores elements in a _column major_ format, meaning that
values are accessed and filled in by column, rather than by row. The
`MatrixAccessor` implements the bracket operator, returning a pointer to the
first element of a column. As a result, for a MatrixAccessor `mat`, the i-th
row and j-th column is accessed with `ma[j][i]` rather than `m[i, j]`, which
R users are more familiar with.

The code above defines a function `BigColSums`. This is broken into two
components: a dispatch function, and function which implements the logic. The
dispatch function takes as an argument a generic `SEXP` object, a container
object for all data objects in R. First, we tell Rcpp that the SEXP object is
an external pointer (`XPtr`) associated with a `BigMatrix` object. It then
creates the appropriate `MatrixAccessor` depending on the type of the data
stored in the `BigMatrix`, as detected at runtime, and dispatches both the
External Pointer and  Matrix Accessor objects for the `BigMatrix` to the
implementation of the logic for `BigColSums`.

Because the logic for `BigColSums` remains the same regardless of the underlying
representation of the data, we have simply implemented a generic template that
works the same for all four types (char, short, int, double). In the
implementation of `BigColSums`'s logic, first we create a `NumericVector` to
hold the results of the column sums, using the number of columns in the
`BigMatrix` to define the length of the results vector. Next, the function loops
through the columns. For each iteration of the loop the values in a single
column are accumulated and stored at the appropriate location in the `colSum`
vector. Finally, the columns' sums are returned to R via the dispatch function.

The code below shows how to use the new Rcpp function. A `big.matrix` object
is created, named bigmat, with 10000 rows and 3 columns. Matrix elements are
stored on disk in a "backingfile" named bigmat.bk. After creating the
big.matrix object, the column values are filled in and then the
`big.matrix`'s external pointer, which is references with the `address` slot,
is passed to the Rcpp `BigColSums` function. The corresponding R function is
shown below so that you can verify that our new function returns the correct
value.


{% highlight r %}
suppressMessages(require(bigmemory))

# set up big.matrix
nrows <- 10000
setwd("/tmp")
bkFile <- "bigmat.bk"
descFile <- "bigmatk.desc"
bigmat <- filebacked.big.matrix(nrow=nrows, ncol=3, type="double",
                                backingfile=bkFile, backingpath=".",
				descriptorfile=descFile,
				dimnames=c(NULL,NULL))

# Each column value with be the column number multiplied by
# samples from a standard normal distribution.
set.seed(123)
for (i in 1:3) bigmat[,i] = rnorm(nrows)*i

# Call the Rcpp function.
res <- BigColSums(bigmat@address)
print(res)
{% endhighlight %}

<pre class="output">
[1]  -23.72 -182.13 -212.98
</pre>

{% highlight r %}
# Verify that it is that same as running colSums on a matrix with equal values.
print(all.equal(res, colSums(bigmat[,])))
{% endhighlight %}

<pre class="output">
[1] TRUE
</pre>
