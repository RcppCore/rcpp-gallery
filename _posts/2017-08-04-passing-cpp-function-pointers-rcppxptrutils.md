---
title: Passing user-supplied C++ functions with RcppXPtrUtils
author: Iñaki Ucar
license: GPL (>= 2)
tags: function
summary: Demonstrates how to build and check user-supplied C++ functions 
  with the RcppXPtrUtils package
layout: post
src: 2017-08-04-passing-cpp-function-pointers-rcppxptrutils.Rmd
---

Sitting on top of R's external pointers, the `RcppXPtr` class provides
a powerful and generic framework for
[Passing user-supplied C++ functions](https://gallery.rcpp.org/articles/passing-cpp-function-pointers/)
to a C++ backend. This technique is exploited in the
[RcppDE](https://cran.r-project.org/package=RcppDE) package, an
efficient C++ based implementation of the
[DEoptim](https://cran.r-project.org/package=DEoptim) package that
accepts optimisation objectives as both R and compiled functions (see
`demo("compiled", "RcppDE")` for further details). This solution has a
couple of issues though:

1. Some repetitive scaffolding is always needed in order to bring the `XPtr` to R space.
2. There is no way of checking whether a user-provided C++ function
   complies with the internal signature supported by the C++ backend,
   which may lead to weird runtime errors.

## Better `XPtr` handling with RcppXPtrUtils

In a nutshell, RcppXPtrUtils provides functions for dealing with these
two issues: namely, `cppXPtr` and `checkXPtr`. As a package author,
you only need to 1) import and re-export `cppXPtr` to compile code and
transparently retrieve an `XPtr`, and 2) use `checkXPtr` to internally
check function signatures.

`cppXPtr` works in the same way as `Rcpp::cppFunction`, but instead of
returning a wrapper to directly call the compiled function from R, it
returns an `XPtr` to be passed to, unwrapped and called from C++. The
returned object is an R's `externalptr` wrapped into a class called
`XPtr` along with additional information about the function signature.










