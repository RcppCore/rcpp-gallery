---
title: Using GSL functions from R
author: Dirk Eddelbuettel
updated: Nov 21, 2015
license: GPL (>= 2)
tags: modeling gsl featured
summary: This example shows how to call a GSL function using RcppGSL
layout: post
src: 2012-12-22-gsl-colnorm-example.cpp
---
The [GNU GSL](http://www.gnu.org/software/gsl/) is a very
popular--and versatile--library convering many, many scientific
computing topics. It provides a standard C API. This API is
somewhat restrictive for C++ programmer. However, RcppGSL makes it
very easy to pass matrices and vectors in and out.

The following example, based on the code used in the complete (!!)
example package included within RcppGSL, which itself in based on
an example from the GSL documentation, illustrates this by
computing simple vector norm given matrix.

As explained in the package documentation, the RcppGSL clue code
instantiates C language pointers suitable for GSL (here the matrix
`M`). In versions prior to RcppGSL 0.3.0, those *had to* be freed
manually.  Since release 0.3.0, an simple internal mechanism takes
care of this automatically at the end of the score. This form is
more common to C++, and now shown below.  Other aspects of the the
example are straighforward: take a matrix, create a return vector
and compute the chosen norm for each column of the matrix.

This example is also shorter and simpler thanks to Rcpp Attributes.
For illustration, several older approaches are still 
[contained in the source file](https://github.com/eddelbuettel/rcppgsl/blob/master/inst/examples/RcppGSLExample/src/colNorm.cpp).




