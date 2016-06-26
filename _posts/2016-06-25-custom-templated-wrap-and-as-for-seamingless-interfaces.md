---
title: Custom Templated as and wrap Functions within Rcpp.
author: James Joseph Balamuta
license: GPL (>= 2)
tags: basics boost 
summary: Creating custom template magic for as and wrap autocoversion functions
layout: post
src: 2016-06-25-custom-templated-wrap-and-as-for-seamingless-interfaces.Rmd
---


## Introduction 

Consider a need to be able to interface with a data type that is not
presently supported by Rcpp. The data type might come from a new library, or
from within one of our own applications. In either cases, Rcpp is faced with an issue of
consciousness as the new data type is not similar to known types so the
autocoversion or seamless R to C++ integration cannot be applied
correctly. The issue is two-fold as we need to consider both directions:

1. Converting from R to C++ using `Rcpp::as<T>(obj)`
2. Converting from C++ to R using `Rcpp::wrap(obj)`

Luckily, there is a wonderful Rcpp vignette called
[Extending Rcpp](https://cran.r-project.org/web/packages/Rcpp/vignettes/Rcpp-extending.pdf)
that addresses custom objects. However, the details listed are more
abstracted than one would like.  So, I am going to try to take you through the
steps with a bit of commentary. Please note that the approach used is via
**Templates and partial specialization** and will result in some nice
automagic at the end.

The overview of the discussion will focus on:

- Stage 1 - Forward Declarations
- Stage 2 - Including the Rcpp Header
- Stage 3 - Implementation of Forward Declarations
- Stage 4 - Testing Functionality
- Stage 5 - All together

## Explanation of Stages

### Stage 1 - Forward Declarations

In the first stage, we must declare our intent to the features we wish to use
prior to engaging `Rcpp.h`. To do so, we will load a different header file
and add some definitions to the `Rcpp::traits` namespace.

Principally, when we start writing the file, the first header that we must
load is `RcppCommon.h` and **not** the usual `Rcpp.h`!! If we do not place
the forward declaration prior to the `Rcpp.h` call, we will be unable to
appropriately register our extension.

Then, we must add in the different plugin markup for `sourceCpp()` to set the
appropriate flags during the compilation of the code. After the plugins, we
will include the actual headers that we want to use. In this document, we
will focus on Boost headers for the concrete example. Lastly, we must
add two special Rcpp function declaration, `Rcpp::as<T>(obj)` and
`Rcpp::wrap(obj)`, within the `Rcpp::traits` namespace. To enable multiple types,
we must create an `Exporter` class instead of a more direct call to `template
<> ClassName as( SEXP )`.


{% highlight cpp %}
// -------------- Stage 1: Forward Declarations with `RcppCommon.h`

#include <RcppCommon.h>

// Flags for C++ compiler: include Boost headers, use the C++11 standard

// [[Rcpp::depends(BH)]]
// [[Rcpp::plugins("cpp11")]]

// Third party library includes that provide the template class of ublas
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/matrix.hpp>

// Provide Forward Declarations
namespace Rcpp {

    namespace traits{
  
        // Setup non-intrusive extension via template specialization for
        // 'ublas' class boost::numeric::ublas
    
        // Support for wrap
        template <typename T> SEXP wrap(const boost::numeric::ublas::vector<T> & obj);
    
        // Support for as<T>
        template <typename T> class Exporter< boost::numeric::ublas::vector<T> >;
  
    }
}
{% endhighlight %}
    
### Stage 2 - Include the `Rcpp.h` header

It might seem frivolous to have a stage just to declare import order, but if
`Rcpp.h` is included before the forward declaration then `Rcpp::traits` is
not updated and we enter the abyss. Template programming can be delicate,
respecting this include order is one of many small details one must get right.

Thus:


{% highlight cpp %}

// -------------- Stage 2: Including Rcpp.h

// ------ Place <Rcpp.h> AFTER the Forward Declaration!!!!

#include <Rcpp.h>

// ------ Place Implementations of Forward Declarations AFTER <Rcpp.h>!

{% endhighlight %}

### Stage 3 - Implementing the Declarations

Now, we must actually implement the forward declarations. In particular, the
only implementation that will be slightly problematic is the `as<>` since the
`wrap()` is straight forward.


#### `wrap()`

To implement `wrap()` we must appeal to a built-in type conversion index
within Rcpp which is called
[`Rcpp::traits::r_sexptype_traits<T>::rtype`](https://github.com/RcppCore/Rcpp/blob/master/inst/include/Rcpp/traits/r_sexptype_traits.h). From
this, we are able to obtain an `int` containing the `RTYPE` and then
construct an `Rcpp::Vector`. For the construction of a matrix, the same ideas
hold true.

#### `as()`

For `as<>()`, we need to consider the template that will be passed
in. Furthermore, we setup a `typedef` directly underneath the `Exporter`
class definition to easily define an `OUT` object to be used within the
`get()` method. Outside of that, we use the same trick to move back and forth
from a C++ `T` template type to an `R` type (implemented as one of several
`SEXP` types).

In order to accomplish the `as<>`, or the direct port from R to C++, I had to
do something dirty: *I copied the vector contents*. The code that governs
this output is given within the `get()` of the `Exporter` class. You may wish
to spend some time looking into changing the assignment using pointers
perhaps. I am not very well versed with `ublas` so I did not see an easy
approach to resolve the pointer pass.


{% highlight cpp %}
// -------------- Stage 3: Implementation the Declarations

// Define template specializations for as<> and wrap
namespace Rcpp {

    namespace traits{
  
        // Defined wrap case
        template <typename T> SEXP wrap(const boost::numeric::ublas::vector<T> & obj){
            const int RTYPE = Rcpp::traits::r_sexptype_traits<T>::rtype ;
      
            return Rcpp::Vector< RTYPE >(obj.begin(), obj.end());
        };
    
    
        // Defined as< > case
        template<typename T> class Exporter< boost::numeric::ublas::vector<T> > {
            typedef typename boost::numeric::ublas::vector<T> OUT ;
      
            // Convert the type to a valid rtype. 
            const static int RTYPE = Rcpp::traits::r_sexptype_traits< T >::rtype ;
            Rcpp::Vector<RTYPE> vec;
      
            public:
            Exporter(SEXP x) : vec(x) {
                if (TYPEOF(x) != RTYPE)
                    throw std::invalid_argument("Wrong R type for mapped 1D array");
            }
            OUT get() {
        
                // Need to figure out a way to perhaps do a pointer pass?
                OUT x(vec.size());
        
                std::copy(vec.begin(), vec.end(), x.begin()); // have to copy data
        
                return x;
            }
        };
    }
}
{% endhighlight %}





### Stage 4 - Testing
    
Okay, let's see if what we worked on paid off (**spoiler** It did!
**spoiler**). To check, we should look at two different areas:

1. Trace diagnostics within the function and;
2. An automagic test. 

Both of which are given below. Note that I've opted to shorten the `ublas`
setup to just be:


{% highlight cpp %}

// -------------- Stage 4: Testing

// Here we define a shortcut to the Boost ublas class to enable multiple ublas
// types via a template.
// ublas::vector<T> => ublas::vector<double>, ... , ublas::vector<int>
namespace ublas = ::boost::numeric::ublas;

{% endhighlight %}

#### Trace Diagnostics


{% highlight cpp %}

// [[Rcpp::export]]
void containment_test(Rcpp::NumericVector x1) {
  
    Rcpp::Rcout << "Converting from Rcpp::NumericVector to ublas::vector<double>" << std::endl;

    // initialize the vector to all zero
    ublas::vector<double> x = Rcpp::as< ublas::vector<double> >(x1); 
  
    Rcpp::Rcout << "Running output test with ublas::vector<double>" << std::endl;
  
    for (unsigned i = 0; i < x.size (); ++ i)
        Rcpp::Rcout  << x(i) << std::endl;
  
    Rcpp::Rcout << "Converting from ublas::vector<double> to Rcpp::NumericVector" << std::endl;
  
    Rcpp::NumericVector test = Rcpp::wrap(x);
  
    Rcpp::Rcout << "Running output test with Rcpp::NumericVector" << std::endl;
  
    for (unsigned i = 0; i < test.size (); ++ i)
        Rcpp::Rcout  << test(i) << std::endl;
  
}

{% endhighlight %}

Test Call:


{% highlight r %}
containment_test(c(1,2,3,4))
{% endhighlight %}

Results:


<pre class="output">
Converting from Rcpp::NumericVector to ublas::vector&lt;double&gt;
Running output test with ublas::vector&lt;double&gt;
1
2
3
4
Converting from ublas::vector&lt;double&gt; to Rcpp::NumericVector
Running output test with Rcpp::NumericVector
1
2
3
4
</pre>

This test performed as expected. Onto the next test!

#### Automagic test


{% highlight cpp %}

// [[Rcpp::export]]
ublas::vector<double> automagic_ublas_rcpp(ublas::vector<double> x1) {
    return x1;
}

{% endhighlight %}

Test Call:


{% highlight r %}
automagic_ublas_rcpp(c(1,2,3.2,1.2))
{% endhighlight %}

Results:


{% highlight r %}
automagic_ublas_rcpp(c(1,2,3.2,1.2))
{% endhighlight %}



<pre class="output">
[1] 1.0 2.0 3.2 1.2
</pre>

Success!

### Stage 5 - All together 

Here is the combination of the above code chunks given by stage. If you copy
and paste this into your `.cpp` file, then everything *should* work. If not,
let me know.


{% highlight cpp %}
// -------------- Stage 1: Forward Declarations with `RcppCommon.h`

#include <RcppCommon.h>

// Flags for C++ compiler: include Boost headers, use the C++11 standard

// [[Rcpp::depends(BH)]]
// [[Rcpp::plugins("cpp11")]]

// Third party library includes that provide the template class of ublas
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/matrix.hpp>

// Provide Forward Declarations
namespace Rcpp {

    namespace traits{
  
        // Setup non-intrusive extension via template specialization for
        // 'ublas' class boost::numeric::ublas
    
        // Support for wrap
        template <typename T> SEXP wrap(const boost::numeric::ublas::vector<T> & obj);
    
        // Support for as<T>
        template <typename T> class Exporter< boost::numeric::ublas::vector<T> >;
  
    }
}

// -------------- Stage 2: Including Rcpp.h

// ------ Place <Rcpp.h> AFTER the Forward Declaration!!!!

#include <Rcpp.h>

// ------ Place Implementations of Forward Declarations AFTER <Rcpp.h>!

// -------------- Stage 3: Implementation the Declarations

// Define template specializations for as<> and wrap
namespace Rcpp {

    namespace traits{
  
        // Defined wrap case
        template <typename T> SEXP wrap(const boost::numeric::ublas::vector<T> & obj){
            const int RTYPE = Rcpp::traits::r_sexptype_traits<T>::rtype ;
      
            return Rcpp::Vector< RTYPE >(obj.begin(), obj.end());
        };
    
    
        // Defined as< > case
        template<typename T> class Exporter< boost::numeric::ublas::vector<T> > {
            typedef typename boost::numeric::ublas::vector<T> OUT ;
      
            // Convert the type to a valid rtype. 
            const static int RTYPE = Rcpp::traits::r_sexptype_traits< T >::rtype ;
            Rcpp::Vector<RTYPE> vec;
      
            public:
            Exporter(SEXP x) : vec(x) {
                if (TYPEOF(x) != RTYPE)
                    throw std::invalid_argument("Wrong R type for mapped 1D array");
            }
            OUT get() {
        
                // Need to figure out a way to perhaps do a pointer pass?
                OUT x(vec.size());
        
                std::copy(vec.begin(), vec.end(), x.begin()); // have to copy data
        
                return x;
            }
        };
    }
}

// -------------- Stage 4: Testing

// Here we define a shortcut to the Boost ublas class to enable multiple ublas
// types via a template.
// ublas::vector<T> => ublas::vector<double>, ... , ublas::vector<int>
namespace ublas = ::boost::numeric::ublas;


// [[Rcpp::export]]
void containment_test(Rcpp::NumericVector x1) {
  
    Rcpp::Rcout << "Converting from Rcpp::NumericVector to ublas::vector<double>" << std::endl;

    // initialize the vector to all zero
    ublas::vector<double> x = Rcpp::as< ublas::vector<double> >(x1); 
  
    Rcpp::Rcout << "Running output test with ublas::vector<double>" << std::endl;
  
    for (unsigned i = 0; i < x.size (); ++ i)
        Rcpp::Rcout  << x(i) << std::endl;
  
    Rcpp::Rcout << "Converting from ublas::vector<double> to Rcpp::NumericVector" << std::endl;
  
    Rcpp::NumericVector test = Rcpp::wrap(x);
  
    Rcpp::Rcout << "Running output test with Rcpp::NumericVector" << std::endl;
  
    for (unsigned i = 0; i < test.size (); ++ i)
        Rcpp::Rcout  << test(i) << std::endl;
  
}


// [[Rcpp::export]]
ublas::vector<double> automagic_ublas_rcpp(ublas::vector<double> x1) {
    return x1;
}

{% endhighlight %}

## Closing Remarks

Whew... That was a lot. Hopefully, the above provided enough information as
you may want to extend this post's content past `1D` vectors to perhaps a
`ublas::matrix` and so on. In addition, then you now have the autoconvert
magic of `Rcpp` for `ublas::vector<double>`! Moreover, all one needs to do is
specify the either the parameters or return type of the function to be
`ublas::vector<double>` -- and Voilà, automagic conversion!
