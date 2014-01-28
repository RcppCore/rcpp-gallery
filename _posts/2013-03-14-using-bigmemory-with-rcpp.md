---
title: Using bigmemory with Rcpp
author: Michael Kane
license: GPL (>= 2)
tags: basics boost
summary: This post shows how to access big.matrix objects from Rcpp
layout: post
src: 2013-03-14-using-bigmemory-with-rcpp.Rmd
---

The [bigmemory package](http://www.bigmemory.org) allows users to create
matrices that are stored on disk, rather than in RAM.  When an element is
needed, it is read from the disk and cached in RAM. These objects can be much
larger than native R matrices. Objects stored as such larger-than-RAM
matrices are defined in the `big.matrix` class and they are designed to
behave similar to R matrices. However, they are actually implemented in C++
and can be easily accessed and manipulated directly from Rcpp as this example
shows.


{% highlight cpp %}
#include <Rcpp.h>

// The next line is all it takes to find the bigmemory
// headers -- thanks to the magic of Rcpp attributes, 
// and as bigmemory now accesses Boost headers from the BH package,
// we need to make sure we do so as well in this Rcpp::depends comment.
//
// [[Rcpp::depends(BH, bigmemory)]]
#include <bigmemory/MatrixAccessor.hpp>

#include <numeric>

// [[Rcpp::export]]
Rcpp::NumericVector BigColSums(Rcpp::XPtr<BigMatrix> pBigMat) {

    // Create the matrix accessor so we can get at the elements of the matrix.
    MatrixAccessor<double> ma(*pBigMat);
  
    // Create the vector we'll store the column sums in.
    Rcpp::NumericVector colSums(pBigMat->ncol());
    for (size_t i=0; i < pBigMat->ncol(); ++i)
        colSums[i] = std::accumulate(ma[i], ma[i]+pBigMat->nrow(), 0.0);
    return colSums;
}
{% endhighlight %}


A `BigMatrix` object stores elements in a _column major_ format, meaning that
values are accessed and filled in by column, rather than by row. The
`MatrixAccessor` implements the bracket operator, returning a pointer to the
first element of a column. As a result, for a MatrixAccessor `ma`, the i-th
row and j-th column is accessed with `ma[j][i]` rather than `m[i, j]`, which
R users are more familiar with.
  
The code above defines a function `BigColSums` that takes as an argument the
address of the external pointer associated with a `big.matrix` object. The
function starts by creating a `MatrixAccessor` to provide direct access to
the matrix elements. The `MatrixAccessor` constructor takes the type of
elements as a template parameter and a `BigMatrix` object as function
parameter.  Along with the MatrixAccessor, a NumericVector is created to hold
the return value. Next, the function loops through the columns. For each
iteration of the loop the values in a single column are accumulated and
stored at the appropriate location in the `colSum` vector. Finally, the
columns sums are returned to R.

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

