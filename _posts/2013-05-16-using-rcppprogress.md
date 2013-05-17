---
title: Using RcppProgress to control the long computations in C++
author: Karl Forner
license: GPL (>= 2)
tags: interrupt openmp
summary: Demonstrates how to display a progress bar and interrupt c++ code.
layout: post
src: 2013-05-16-using-rcppprogress.Rmd
---


Usually you write c++ code with R when you want to speedup some calculations. 
Depending on the parameters, and especially during the development, it is difficult to anticipate the execution 
time of your computation, so that you do not know if you have to wait for 1 minute or hours.

[RcppProgress](http://cran.at.r-project.org/web/packages/RcppProgress/index.html) is a tool to help you monitor 
the execution time of your C++ code, by providing a way to interrupt 
the execution inside the c++ code, and also to display a progress bar indicative of the state of your computation.

Additionally, it is compatible with multithreaded code, for example using OpenMP, which is not as trivial as it may
seem since you cannot just stop the execution in one thread, and not all threads should be writing in the console to
avoid a garbled output.
 

{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation(int nb) {
  double sum = 0;
  for (int i = 0; i < nb; ++i) {
  	for (int j = 0; j < nb; ++j) {
			sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
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
  0.096   0.000   0.095 
</pre>



{% highlight r %}
  s
{% endhighlight %}



<pre class="output">
[1] 1002
</pre>



## checking interruption

Let's modify our code to add user interruption check, by calling `Progress::check_abort`.  
Note the `Rcpp::depends(RcppProgress)` attribute in the header part that takes care of the include path for 
the *progress.hpp* header.

Now the `long_computation2` call should be interruptible (with CTRL+C in the classic R console).


{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation2(int nb) {
  double sum = 0;
  Progress p(0, false); // in any case, we need to build an instance, should be improved in the next version
  for (int i = 0; i < nb; ++i) {
    if (Progress::check_abort() )
        return -1.0;
  	for (int j = 0; j < nb; ++j) {
			sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
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
  0.840   0.000   0.838 
</pre>



{% highlight r %}
  s
{% endhighlight %}



<pre class="output">
[1] 3002
</pre>



You may wonder Why do we put the `check_abort` call in the first loop instead that in the second ? 
The `check_abort` call is not neglectable, so it should be put in a place called often enough 
(once per second) but not too often.  

 
## adding a progress bar
  
 Time to add the progress bar. The `increment` function is quite fast, so we can put it in the second loop.
In real life example, it is sufficient to put it at a place called at least every second.
 

{% highlight cpp %}
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation3(int nb, bool display_progress=true) {
  double sum = 0;
  Progress p(nb*nb, display_progress);
  for (int i = 0; i < nb; ++i) {
    if (Progress::check_abort() )
    return -1.0;
    for (int j = 0; j < nb; ++j) {
      p.increment(); // update progress
			sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
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
  0.848   0.000   0.848 
</pre>



{% highlight r %}
  s
{% endhighlight %}



<pre class="output">
[1] 3002
</pre>


## openMP support

First we need this to enable gcc openMP support:


{% highlight r %}
Sys.setenv("PKG_CXXFLAGS"="-fopenmp")
Sys.setenv("PKG_LIBS"="-fopenmp")
{% endhighlight %}


Here's an openMP version of our function:


{% highlight cpp %}
#ifdef _OPENMP
#include <omp.h>
#endif
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
			thread_sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
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
  2.264   0.000   0.572 
</pre>



{% highlight r %}
  s4
{% endhighlight %}



<pre class="output">
[1] 5002
</pre>



{% highlight r %}
  system.time(s1 <- long_computation_omp(5000, 1))
{% endhighlight %}



<pre class="output">
   user  system elapsed 
  2.248   0.000   2.247 
</pre>



{% highlight r %}
  s1
{% endhighlight %}



<pre class="output">
[1] 5002
</pre>


## adding progress monitoring to the openMP function


{% highlight cpp %}
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>
// [[Rcpp::export]]
double long_computation_omp2(int nb, int threads=1) {
#ifdef _OPENMP
  if ( threads > 0 )
    omp_set_num_threads( threads );
 
#endif
  Progress p(nb, true);
  double sum = 0;
#pragma omp parallel for schedule(dynamic)   
  for (int i = 0; i < nb; ++i) {
    double thread_sum = 0;
    if ( ! Progress::check_abort() ) {
      p.increment(); // update progress
      for (int j = 0; j < nb; ++j) {
          thread_sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
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



<pre class="output">
   user  system elapsed 
  2.268   0.008   0.582 
</pre>


## Test it now

If you want to test it now in your R console, just paste the following code (after installing RcppProgress of course):

{% highlight r %}
library(Rcpp)
Sys.setenv("PKG_CXXFLAGS"="-fopenmp")
Sys.setenv("PKG_LIBS"="-fopenmp")

code='
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>

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
          thread_sum += Rf_dlnorm(i+j, 0.0, 1.0, 0);
        }
    }
    sum += thread_sum;
  }
  
  return sum + nb;
}
'

sourceCpp(code=code)
s <- long_computation_omp2(10000, 4)
{% endhighlight %}



Karl Forner  
*Quartz Bio*
