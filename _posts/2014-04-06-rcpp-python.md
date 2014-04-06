---
title: Call Python from R through Rcpp
author: Wush Wu
license: GPL (>= 2)
tags: python integration
summary: Integrate python into R with Rcpp and Boost.Python
layout: post
src: 2014-04-06-rcpp-python.Rmd
---




## Introduction

This is a brief introduction of calling python from R through Rcpp. [The official documentation of python](https://docs.python.org/2/extending/embedding.html) explains how to embed python into C/C++ application. Moreover, the [Boost.Python](http://www.boost.org/doc/libs/1_55_0/libs/python/doc/) library provides seamless interoperability between C++ and the Python programming language. Similarlly, Rcpp provides interoperability between C++ and R. Therefore, it is not hard to call python from R through Rcpp and Boost.Python.

Although there is a package [*rPython*](http://cran.r-project.org/web/packages/rPython/index.html) which provides an interface of python in R through Java, it is interesting to try to connect them through C++.

In this article, I'll show you how to call python 2.7 from R on Ubuntu. Please install the following packages:


## Hello World

The most difficult thing is to establish a development environment. To build an embeded python, we need to install the following package:

```
sudo apt-get install python2.7 python2.7-dev libboost-python-dev
```

Then, we should pass the following flags to the compiler:


{% highlight r %}
py_cflags <- system("python2.7-config --cflags", intern=TRUE)
Sys.setenv("PKG_CFLAGS"=sprintf("%s %s", Sys.getenv("PKG_CFLAGS"), py_cflags))
Sys.setenv("PKG_CXXFLAGS"=sprintf("%s %s", Sys.getenv("PKG_CXXFLAGS"), py_cflags))
py_ldflags <- system("python2.7-config --ldflags", intern=TRUE)
Sys.setenv("PKG_LIBS"=sprintf("%s %s %s", Sys.getenv("PKG_CFLAGS"), "-lboost_python-py27", py_ldflags))
{% endhighlight %}


The following hello world should work!


{% highlight cpp %}
#include <Rcpp.h>
#include <Python.h>

using namespace Rcpp;

//[[Rcpp::export]]
void initialize_python() {
  Py_SetProgramName("");  /* optional but recommended */
  Py_Initialize();
}

//[[Rcpp::export]]
void finalize_python() {
  Py_Finalize();
}

//[[Rcpp::export]]
void hello_python() {
  PyRun_SimpleString("from time import time,ctime\n"
                     "print 'Today is',ctime(time())\n");
}
{% endhighlight %}








Let's call them in R:


{% highlight r %}
initialize_python()
hello_python()
{% endhighlight %}



<pre class="output">
Today is Sun Apr  6 08:11:14 2014
</pre>


It shows that the hello_python function successfully initialize the engine of python and run the python script through `PyRun_SimpleString`.

## Type Conversion

With Boost.Python and Rcpp, we can easily transfer the data between R and python. The following C codes transfer the R `IntegerVector` to python `List`:


{% highlight cpp %}
#include <Rcpp.h>
#include <boost/python/raw_function.hpp>

namespace py = boost::python;

typedef Rcpp::XPtr<py::list> PyList;

using namespace Rcpp;

//[[Rcpp::export]]
SEXP IntVec_to_py_list(IntegerVector src) {
  PyList pretval(new py::list());
  int glue;
  for(int i = 0;i < src.size();i++) {
    glue = src[i];
    pretval->append(glue);
  }
  return pretval;
}
{% endhighlight %}



{% highlight r %}
IntVec_to_py_list(1:10)
{% endhighlight %}



<pre class="output">
&lt;pointer: 0x27d8780&gt;
</pre>


The pointer refers to the memory of the transformed python object 

## Call Python Function

The following example shows how to define a function in python and expose it in R.


{% highlight cpp %}
#include <Rcpp.h>
#include <Python.h>
#include <boost/python/raw_function.hpp>

namespace py = boost::python;

typedef Rcpp::XPtr<py::list> PyList;

using namespace Rcpp;

//[[Rcpp::export]]
void pycall(std::string py_script) {
  PyRun_SimpleString(py_script.c_str());
}

//[[Rcpp::export]]
void pyfun(std::string fun_name, SEXP fun_argument) {
  // create the module of python which is similar to the R_GlobalEnv
  py::object module((py::handle<>(py::borrowed(PyImport_AddModule("__main__")))));
  // look up and retrieve the function of the given name in the module
  py::object pyfun = module.attr("__dict__")[fun_name.c_str()];
  // call the function with the API of boost::python
  py::list argv(*PyList(fun_argument));
  pyfun(argv);
}
{% endhighlight %}



{% highlight r %}
pycall("
def print_list(src):
  for i in src:
    print i
")
a <- IntVec_to_py_list(1:10)
pyfun("print_list", a)
{% endhighlight %}



<pre class="output">
1
2
3
4
5
6
7
8
9
10
</pre>


## Error Handling

The error of python engine could be handled easily by the C++ try/catch as follow:


{% highlight cpp %}
#include <Rcpp.h>
#include <Python.h>
#include <boost/python/raw_function.hpp>

namespace py = boost::python;

typedef Rcpp::XPtr<py::list> PyList;

//[[Rcpp::export]]
void pyfun(std::string fun_name, SEXP fun_argument) {
  try {
    // create the module of python which is similar to the R_GlobalEnv
    py::object module((py::handle<>(py::borrowed(PyImport_AddModule("__main__")))));
    // look up and retrieve the function of the given name in the module
    py::object pyfun = module.attr("__dict__")[fun_name.c_str()];
    // call the function with the API of boost::python
    py::list argv(*PyList(fun_argument));
    pyfun(argv);
  }
  catch (py::error_already_set) {
    PyErr_Print();
  }
}
{% endhighlight %}



{% highlight r %}
pycall("
def print_list(src):
  for i in src:
    print i
")
a <- IntVec_to_py_list(1:10)
pyfun("print_lists", a) # a typo of the function name
{% endhighlight %}



<pre class="output">
KeyError: 'print_lists'
</pre>


## Summary

These examples show how to integrate python and R with Rcpp and Boost.Python. The similar idea could be used to integrate R with other scripting language such as javascript(nodejs). The core steps are initializing the engine (Hello World), transforming the data(Type Conversion), exposing the functions(Call Python Function), and handling the error properly(Error Handling). Some C++ libraries ease the work greatly such as Rcpp for R, Boost.Python for python and [V8Converter](https://code.google.com/p/v8-juice/) for javascript.
