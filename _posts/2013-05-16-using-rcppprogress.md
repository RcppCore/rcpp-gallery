---
title: Using RcppProgress to control the long computations in C++
author: Karl Forner
license: GPL (>= 2)
tags: interrupt openmp
updated: Jul 09, 2017
updateauthor: Matt Dziubinski
summary: Demonstrates how to display a progress bar and interrupt C++ code.
layout: post
src: 2013-05-16-using-rcppprogress.Rmd
---


Usually you write C++ code with R when you want to speedup some calculations.
Depending on the parameters, and especially during the development, it is
difficult to anticipate the execution time of your computation, so that you
do not know if you have to wait for one minute or several hours.

[RcppProgress](http://cran.r-project.org/web/packages/RcppProgress/index.html) 
is a tool to help you monitor the execution time of your C++ code, by
providing a way to interrupt the execution inside the C++ code, and also to
display a progress bar indicative of the state of your computation.

Additionally, it is compatible with multithreaded code, for example using
OpenMP, which is not as trivial as it may seem since you cannot just stop the
execution in one thread. Also, not all threads should be writing in the console
to avoid garbled output.
 

{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation(int nb) {
    double sum = 0;
    for (int i = 0; i < nb; ++i) {
        for (int j = 0; j < nb; ++j) {
	    sum += R::dlnorm(i+j, 0.0, 1.0, 0);
	}
    }
    return sum + nb;
}
{% endhighlight %}


{% highlight r %}
    system.time(s  <- long_computation(1000))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  0.116   0.000   0.114 
</pre>



{% highlight r %}
    s
{% endhighlight %}



<pre class="output">
[1] 1002.32
</pre>


## Checking for user interrupts

Let's modify our code to add a check for user interruption by calling the function
`Progress::check_abort`.  Note the `Rcpp::depends(RcppProgress)` attribute in
the header part that takes care of the include path for the *progress.hpp*
header.

Now the `long_computation2` call should be interruptible (with CTRL+C in the
classic R console).


{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation2(int nb) {
    double sum = 0;
    Progress p(0, false); // we need an instance, should be improved in next version
    for (int i = 0; i < nb; ++i) {
        if (Progress::check_abort() )
            return -1.0;
  	for (int j = 0; j < nb; ++j) {
	    sum += R::dlnorm(i+j, 0.0, 1.0, 0);
	}
    }
    return sum + nb;
}
{% endhighlight %}


{% highlight r %}
    system.time(s  <- long_computation2(3000)) # interrupt me
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  1.012   0.000   1.022 
</pre>



{% highlight r %}
    s
{% endhighlight %}



<pre class="output">
[1] 3002.32
</pre>


You may wonder why we put the `check_abort` call in the first loop instead
that in the second.  The performance cost of `check_abort` call is not
negligible. It should be put in a place called often enough (once per
second) yet not too often.

 
## Adding a progress bar
  
Time to add the progress bar. The `increment` function is quite fast, so we
can put it in the second loop.  In real life example, it is sufficient to put
it at a place called at least every second.
 

{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
#include <progress_bar.hpp>
// [[Rcpp::export]]
double long_computation3(int nb, bool display_progress=true) {
    double sum = 0;
    Progress p(nb*nb, display_progress);
    for (int i = 0; i < nb; ++i) {
        if (Progress::check_abort() )
            return -1.0;
        for (int j = 0; j < nb; ++j) {
            p.increment(); // update progress
	    sum += R::dlnorm(i+j, 0.0, 1.0, 0);
	}
    }
    return sum + nb;
}
{% endhighlight %}


{% highlight r %}
    system.time(s  <- long_computation3(3000)) # interrupt me
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  1.156   0.004   1.196 
</pre>



{% highlight r %}
    s
{% endhighlight %}



<pre class="output">
[1] 3002.32
</pre>

## OpenMP support

First we need this to enable OpenMP support for `gcc`. In the early days we used


{% highlight r %}
Sys.setenv("PKG_CXXFLAGS"="-fopenmp")
Sys.setenv("PKG_LIBS"="-fopenmp")
{% endhighlight %}

and more recent version of Rcpp have a plugin
Recent Rcpp versions should have a plugin which does this for us.

Here is an OpenMP version of our function:


{% highlight cpp %}
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::plugins(openmp)]]
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation_omp(int nb, int threads=1) {
#ifdef _OPENMP
    if ( threads > 0 )
        omp_set_num_threads( threads );
    REprintf("Number of threads=%i\n", omp_get_max_threads());
#endif
 
    double sum = 0;
#pragma omp parallel for schedule(dynamic)   
    for (int i = 0; i < nb; ++i) {
        double thread_sum = 0;
  	for (int j = 0; j < nb; ++j) {
	    thread_sum += R::dlnorm(i+j, 0.0, 1.0, 0);
	}
        sum += thread_sum;
    }
    return sum + nb;
}
{% endhighlight %}

Now check that it is parallelized:

{% highlight r %}
    system.time(s4 <- long_computation_omp(5000, 4))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  2.848   0.004   0.990 
</pre>



{% highlight r %}
    s4
{% endhighlight %}



<pre class="output">
[1] 5002.14
</pre>



{% highlight r %}
    system.time(s1 <- long_computation_omp(5000, 1))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  2.836   0.004   2.851 
</pre>



{% highlight r %}
    s1
{% endhighlight %}



<pre class="output">
[1] 5002.32
</pre>

## adding progress monitoring to the openMP function


{% highlight cpp %}
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::plugins(openmp)]]
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
#include <progress_bar.hpp>
// [[Rcpp::export]]
double long_computation_omp2(const int nb, int threads=1) {
#ifdef _OPENMP
    if ( threads > 0 )
        omp_set_num_threads( threads );
#endif
    Progress p(nb, true);
    double sum = 0;
#pragma omp parallel for default(none) reduction(+ : sum) schedule(dynamic)
    for (int i = 0; i < nb; ++i) {
        double thread_sum = 0;
        if ( ! Progress::check_abort() ) {
            p.increment(); // update progress
            for (int j = 0; j < nb; ++j) {
                thread_sum += R::dlnorm(i+j, 0.0, 1.0, 0);
            }
        }
        sum += thread_sum;
    }
    return sum + nb;
}
{% endhighlight %}


{% highlight r %}
system.time(s <- long_computation_omp2(5000, 4))
{% endhighlight %}

## Test it now

If you want to test it now in your R console, just paste the following code
(after installing the 
[RcppProgress](http://cran.r-project.org/web/packages/RcppProgress/index.html)
package, of course):


{% highlight cpp %}
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::plugins(openmp)]]
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
#include <progress_bar.hpp>

// [[Rcpp::export]]
double long_computation_omp2(int nb, int threads=1) {
#ifdef _OPENMP
    if ( threads > 0 )
        omp_set_num_threads( threads );
    REprintf("Number of threads=%i\\n", omp_get_max_threads());
#endif
    Progress p(nb, true);
    double sum = 0;
#pragma omp parallel for schedule(dynamic)   
    for (int i = 0; i < nb; ++i) {
        double thread_sum = 0;
        if ( ! Progress::check_abort() ) {
            p.increment(); // update progress
            for (int j = 0; j < nb; ++j) {
                thread_sum += R::dlnorm(i+j, 0.0, 1.0, 0);
            }
        }
        sum += thread_sum;
    }
  
    return sum + nb;
}
{% endhighlight %}

and run


{% highlight r %}
Rcpp::sourceCpp(code=code)
s <- long_computation_omp2(10000, 4)
{% endhighlight %}



Karl Forner  
*Quartz Bio*
