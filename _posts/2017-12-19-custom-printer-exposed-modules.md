---
title: Define a custom print method for exposed C++ classes
author: Daniel Schalk
license: GPL (>= 2)
tags: modules
summary: How to define a custom print method for an exposed C++ class. 
layout: post
src: 2017-12-19-custom-printer-exposed-modules.cpp
---

When writing an `R` package wrapping some `C++` data structures, using
[Rcpp Modules](http://dirk.eddelbuettel.com/code/rcpp/Rcpp-modules.pdf)
is a convenient option. After exposing a class to `R`, it can be used to 
easily create new instances of that class.

As an example, let us look at the `Uniform` class of the Rcpp Modules vignette: 


{% highlight cpp %}
// cf Rcpp Modules vignette

#include <Rcpp.h>

using namespace Rcpp;

class Uniform {
  public:
    Uniform(double min_, double max_) : min(min_), max(max_) {}
    NumericVector draw(int n) const {
      RNGScope scope;
      return runif( n, min, max );
    }
    double min, max;
};

double uniformRange( Uniform* w) {
  return w->max - w->min;
}

RCPP_MODULE(unif_module) {
  class_<Uniform>( "Uniform" )
  
  .constructor<double,double>()

  .field( "min", &Uniform::min )
  .field( "max", &Uniform::max )

  .method( "draw", &Uniform::draw )
  .method( "range", &uniformRange )
  ;
}
{% endhighlight %}


After sourcing the file the `Uniform` class can be used:


{% highlight r %}
library(methods)    ## needed for S4 

## Create new instance myuniform:
myuniform <- new(Uniform, 0, 10)

# Print the new uniform isntance:
myuniform
{% endhighlight %}



<pre class="output">
C++ object &lt;0x55fd63177450&gt; of class 'Uniform' &lt;0x55fd63788a20&gt;
</pre>


What happens now, is that the uniform instance calls its default print
method -- which results in a fairly uninformative display. 
It would be nice if the printer of the instances could be customized 
to provide more information about the specific object. 

## Customize the Printer

It is possible to make use of the underlying 
[S4 structure](http://adv-r.had.co.nz/OO-essentials.html#s4) of the exposed 
`C++` classes:

{% highlight r %}
# Check exposed S4 structure:
isS4(myuniform)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
# Get class name:
class(myuniform)
{% endhighlight %}



<pre class="output">
[1] &quot;Rcpp_Uniform&quot;
attr(,&quot;package&quot;)
[1] &quot;.GlobalEnv&quot;
</pre>


For the `Uniform` class this is `Rcpp_Uniform`. To obtain a custom printer the 
last step now is to set the method `show` and define the function which should
be used as printer:

{% highlight r %}
# Define the printer:
ignoreMe <- setMethod("show", "Rcpp_Uniform", function (object) {
  cat("\n Hi, I am an uniform object!\n")
  cat("\n I was initialized with a minimum value of", object$min)
  cat("\n and a maximum value of ", object$max, ".\n", sep = "")
  cat("\n Therefore my range is ", object$range(), ".", sep = "")
  cat("\n\n")
})

# Test the printer:
myuniform
{% endhighlight %}



<pre class="output">

 Hi, I am an uniform object!

 I was initialized with a minimum value of 0
 and a maximum value of 10.

 Therefore my range is 10.
</pre>


This works very nicely. Now it is possible to provide some more informations about
a new `Uniform` instance. One thing to note is that `setMethod` returns the
method name as string, which we assign to an otherwise unused variable.

## Use in Packages

To get that print method as a default printer after exposing the `C++` class to `R` 
within a package, it is sufficient to create a `R` file (e.g. 
`R/uniform_printer.R`) and put the following code in there:

{% highlight r %}
ignoreMe <- setMethod("show", "Rcpp_Uniform", function (object) {
  cat("\n Hi, I am an uniform object!\n")
  cat("\n I was initialized with a minimum value of", object$min)
  cat("\n and a maximum value of ", object$max, ".\n", sep = "")
  cat("\n Therefore my range is ", object$range(), ".", sep = "")
  cat("\n\n")
})
{% endhighlight %}
