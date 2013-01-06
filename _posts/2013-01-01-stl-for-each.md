---
title: STL for_each and generalized iteration
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: stl featured
summary: Using the STL's for_each 
layout: post
src: 2013-01-01-stl-for-each.cpp
---
The STL contains a very general looping or sweeping construct in
the `for_each` algorith.  It can be used with function objects
(such as the simple `square` function used [here](../stl-transform))
but also with custom class which can be used to keep to keep state.




{% highlight cpp %}
#include <Rcpp.h>

using namespace Rcpp;

// somewhat silly little class derived from unary_function<T, void> to
// illustrate keeping state -- we interpret the vector x as containing
// growth rates (or returns), and we compute cumulative as well as
// relative gains.
template<class T> class cumProd : public std::unary_function<T, void> {
public:
    cumProd() : cp(1.0), cnt(1) {}      // constructor
    void operator() (T x) {             // default operator()
        cp *= 1.0 + x;
        Rcout << "Iteration "   << cnt++
              << " Growth "     << x
              << " Compounded " << cp 
              << " Proportion " << x/(cp - 1.0)
              << std::endl;
    }
private:  
    double cp;
    int cnt;
};

// [[Rcpp::export]]
void forEach(Rcpp::NumericVector x) {
    std::for_each(x.begin(), x.end(), cumProd<double>());
}
{% endhighlight %}


We can illustrate this on a simple example:

{% highlight r %}
set.seed(42)
x <- rnorm(6, 0, 0.01)
x
{% endhighlight %}



<pre class="output">
[1]  0.013710 -0.005647  0.003631  0.006329  0.004043 -0.001061
</pre>



{% highlight r %}
forEach(x)
{% endhighlight %}



<pre class="output">
Iteration 1 Growth 0.0137096 Compounded 1.01371 Proportion 1
Iteration 2 Growth -0.00564698 Compounded 1.00799 Proportion -0.707182
Iteration 3 Growth 0.00363128 Compounded 1.01165 Proportion 0.31182
Iteration 4 Growth 0.00632863 Compounded 1.01805 Proportion 0.350659
Iteration 5 Growth 0.00404268 Compounded 1.02216 Proportion 0.182403
Iteration 6 Growth -0.00106125 Compounded 1.02108 Proportion -0.0503469
</pre>

