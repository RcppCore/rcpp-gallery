---
title: "Using Scheme from R via Guile and Rcpp"
author: "Dirk Eddelbuettel"
license: GPL (>= 2)
tags: basics
summary: An illustration of using Guile scripts with R
layout: post
src: 2020-12-17-guile-via-rcpp.Rmd
---



### Introduction

[GNU Guile](https://www.gnu.org/software/guile/) is a programming language _"designed to help
programmers create flexible applications that can be extended by users or other programmers with
plug-ins, modules, or scripts"_. It is an extension language platform, and contains an efficient
compiler and virtual machine allowing both out of the box use for programs in Scheme, as well as
integration with C and C++ programs.

[Matías Guzmán Naranjo](https://mguzmann89.gitlab.io/) posed an almost-complete [question on
StackOverflow](https://stackoverflow.com/questions/65340724/linking-guile-to-rcpp) about how to
combine Guile with R via Rcpp. It turns out that the code by Matías came from [this
gist](https://gist.github.com/ncweinhold/9724254) showing both a Guile Scheme script, and a C
use case example.

#### The Guile Scheme Script `script.scm`

This file is posted in this [gist](https://gist.github.com/ncweinhold/9724254).

```scheme
(define simple-func
  (lambda ()
    (display "Script called, now I can change this") (newline)))

(define quick-test
  (lambda ()
    (display "Adding another function, can modify without recompilation") 
    (newline)
    (adding-without-recompilation)))

(define adding-without-recompilation
  (lambda ()
    (display "Called this, without recompiling the C code") (newline)))
```

#### The C wrapper `testguile.c`

This file is also posted in this [gist](https://gist.github.com/ncweinhold/9724254). We used (as
shown below) simpler link instructions which (on Ubuntu) can be ontained via `guile-config link` and
are just `-lguile-3.0 -lgc` on our Ubuntu 20.04 machine.


```c
/*
 * Compile using: 
 * gcc testguile.c \
 * -I/usr/local/Cellar/guile/1.8.8/include \
 * -D_THREAD_SAFE -L/usr/local/Cellar/guile/1.8.8/lib -lguile -lltdl -lgmp -lm -lltdl -o testguile
 */

#include <stdio.h>
#include <libguile.h>

int main(int argc, char** argv) {
    SCM func, func2;
    scm_init_guile();

    scm_c_primitive_load("script.scm");

    func = scm_variable_ref(scm_c_lookup("simple-func"));
    func2 = scm_variable_ref(scm_c_lookup("quick-test"));

    scm_call_0(func);
    scm_call_0(func2);

    return 0;
}
```

#### The Rcpp Caller 

This is an edited and updated version of [Matías' initial
file](https://stackoverflow.com/questions/65340724/linking-guile-to-rcpp).

```c++
#include <Rcpp.h>
#include <stdio.h>
#include <libguile.h>

using namespace Rcpp;

// [[Rcpp::export]]
int test_guile(std::string file) {
    SCM func, func2;
    scm_init_guile();

    scm_c_primitive_load(file.c_str());

    func = scm_variable_ref(scm_c_lookup("simple-func"));
    func2 = scm_variable_ref(scm_c_lookup("quick-test"));

    scm_call_0(func);
    scm_call_0(func2);

    return 0;
}
```

#### The System Integration

On our Ubuntu 20.04 box, Guile 3.0 is well supported and is easy installable. Header files are in a
public location, as are the library files.  The configuration helper `guile-config` (also
`guile-config-3.0` to permit continued use of 2.2 and 2.0) provides the required compiler and linker
flags which we simply hardwired into `src/Makevars`.  A proper package would need to discover these
from a script, typically `configure`, or `Makefile` invocations.  We leave this for another time; if
anybody reading this is interested in taking this further please get in touch.

```make
PKG_CXXFLAGS = -I"/usr/include/guile/3.0"
PKG_LIBS = -lguile-3.0 -lgc
```

#### Package

The rest of package was created via one call `Rcpp.package.skeleton("RcppGuile")` after which we
- simply copied in the C++ file above into `src/guile.cpp`;
- added the `src/Makevars` snippet above; and
- copied in the example Scheme scripts as `inst/guile/script.scm`

and that's it!  Then any of the usual steps with R will build and install a working package.

The complete package is available in [this
directory](https://github.com/eddelbuettel/stackoverflow/tree/master/65340724/RcppGuile).

#### Guile Demo

The package can be used as follows, retrieving the Scheme script from within the package and clearly
running it:

```r
> library(RcppGuile)
> test_guile(system.file("guile", "script.scm", package="RcppGuile"))
Script called, now I can change this
Adding another function, can modify without recompilation
Called this, without recompiling the C code
[1] 0
> 
```

[GNU Guile](https://www.gnu.org/software/guile/) is made for building extensions. This short note
showed how easy it is to connect R with Guile via a simple Rcpp package.
