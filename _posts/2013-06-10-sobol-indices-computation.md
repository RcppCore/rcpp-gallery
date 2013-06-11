---
title: Sobol Sensitivity Analysis
author: Robin Girard
license: GPL (>= 2)
tags: sensitivity benchmark
summary: Compares speed performances of Monte Carlo estimation of Sobol indices with Rcpp and sensitivity package
layout: post
src: 2013-06-10-sobol-indices-computation.Rmd
---

Sensitivity analysis is the task of evaluating the sensitivity of a model output Y to input variables  (X1,...,Xp). Quite often, it is assumed that this output is related to the input through a known function f :Y= f(X1,...,Xp). 

Sobol indices are generalizing the coefficient of the coefficient of  determination in regression. The ith first order indice is the proportion of the output variance that is "due" to the ith input variable. Numerous details and references are found e.g. in [this](http://www.sciencedirect.com/science/article/pii/S0378475400002706) paper. In particular, the mentionned paper defines an estimation procedure that we use here.

This estimation procedure relies on the use of montecarlo estimates where computation speed and precision of the estimate are highly connected.  Let us show how to compute these indices efficiently in c++ while keeping the code simple.  

To fix idea we use a "test" function f that was introduced in the First papers of Sobol around sensitivity indices estimation, see e.g. his 1993 paper "Sensitivity analysis for non-linear mathematical models". This function is called "the Sobol Function". 

Here is a cpp implementation of sobol function and of the Sobol indices estimation procedure:



{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// the Sobol function (i.e. "f")
inline double sobol_fun_cpp(Rcpp::NumericVector X) 
{
    double it_a[8]={0, 1, 4.5, 9, 99, 99, 99, 99};
    int compteur=0;
    Rcpp::NumericVector::iterator it_X=X.begin();
    double res=1;
    while (compteur!=8)
    {
        res = res * (abs(4 * (*it_X) - 2) + 
                    (it_a[compteur])) / (1 + (it_a[compteur]));
        ++it_X;compteur++;
    }
    return res;
};

// a required function that returns X1[1],...,X1[i-1],X2[i],X1[i+1],...X[p]
inline NumericVector special_Cat(NumericVector X1,NumericVector X2,int i)
{ 
  NumericVector res=clone(X1);
  res[i]=X2[i];
  return(res);
}


//the sobol indices estimation 
// [[Rcpp::export]]
Rcpp::List FirstOrderSobol(int n) {
  // n is the number of replicates in the Riemann sum
    int p=8; // number of input variables 
    double tmpval,f0square=0,f0f0=0;
    NumericVector tmpcat(p),X1(p),X2(p),fj(p,0.),Dj(p,0.);
    for (int i=0; i<n; i++)
    {
      X1=runif(p); X2=runif(p); // here integral over the Lebesgue measure
      tmpval=sobol_fun_cpp(X1); // this first evaluation is reused hence stored
      f0square+=tmpval*tmpval;
      f0f0+=sobol_fun_cpp(X2)*tmpval;
      for (int j=0;j<p;j++)
      {    
        fj[j]+=sobol_fun_cpp(special_Cat(X2,X1,j))*tmpval; 
        Dj[j]+=sobol_fun_cpp(special_Cat(X1,X2,j))*tmpval; 
      }
    }
    double variance = (f0square/n-(f0f0/n));

    return Rcpp::List::create(
      Rcpp::Named("Sobol_Total_Indexes") = 1-( (Dj/(n-1)) - (f0f0/n) ) / variance,
      Rcpp::Named("Sobol_firstOrder_Indexes") = ((fj/(n-1)) - (f0f0/n) )/variance);
}
{% endhighlight %}



Now compare the results with a similar function to estimate Sobol indices from the excellent package [sensitivity](http://cran.r-project.org/web/packages/sensitivity/index.html) (it has a lot of other sensitivity indices and estimation procedures efficiently implemented and well documented).


{% highlight r %}
suppressPackageStartupMessages(require(sensitivity))
sensitivity_sobol2002<-function(n){
  X1 <- data.frame(matrix(runif(8 * n), nrow = n))
  X2 <- data.frame(matrix(runif(8 * n), nrow = n))
  return(sobol2002(model = sobol.fun, X1, X2))
}

n <- 10000
rbenchmark::benchmark(sensitivity_sobol2002(n),
                      FirstOrderSobol(n),
                      columns=c("test", "elapsed",
                                "relative", "user.self", "sys.self"),
                      order="relative")
{% endhighlight %}



<pre class="output">
                      test elapsed relative user.self sys.self
2       FirstOrderSobol(n)   3.263     1.00     3.255    0.006
1 sensitivity_sobol2002(n)  45.516    13.95    37.308    8.176
</pre>


It seems that the Rcpp code is faster (by a factor between 5 and 10), and part of the explanation may be in the use of a dataframe in the package sensitivity. 
The result for the case of the Sobol function are slightly different (see the first two indices) certainly due to different definition of indices. 


{% highlight r %}
n <- 10000
X1 <- data.frame(matrix(runif(8 * n), nrow = n))
X2 <- data.frame(matrix(runif(8 * n), nrow = n))
x <- sobol2002(model = sobol.fun, X1, X2) ## indices from package sensitivity
res=FirstOrderSobol(n)# our indices
{% endhighlight %}



{% highlight r %}
cat("Total indices\n From Sensitivity package : ",x$S[,1],"\n", 
    "From our function : ",res$Sobol_firstOrder_Indexes,"\n")
{% endhighlight %}



<pre class="output">
Total indices
 From Sensitivity package :  0.668 0.1927 0.02959 0.003347 3.782e-05 0.0001375 0.0004649 -1.697e-05 
 From our function :  0.8075 0.08668 0.002824 0.003711 0.0002175 -7.8e-05 0.0001499 0.0001217 
</pre>


The use of Rcpp here hence allows to have a faster code at the cost of changing manually the estimation loop in C++ if one want to make a sensitivity analysis of another function (than the "Sobol function"). 

For standard users this migth be a cost difficult to afford. A solution to overcome this problem is to let the user supply the function f and even the distribution of x. This requires that you know how to pass a function (either implemented in c++ or in R) as a parameter to FirstOrderSobol. Dirk has already done that in package [RcppDE](http://cran.r-project.org/web/packages/RcppDE/index.html). Part of the trick is explained in [this](http://gallery.rcpp.org/articles/passing-cpp-function-pointers/) Rcpp Gallery post , and the  answer 
[here](https://stat.ethz.ch/pipermail/r-devel/2011-September/062052.html) provides more details. Could be the purpose of a further package. In addition my experience with sensitivity analysis is that the Sobol Function is very simple, this means that in a lot of practical situation the gain of implementing the subsequent function in c++ can be much larger. 

Finally, I like the fact that student interested in Sobol indices get the details of the used estimator hence not seeing Sobol indices estimation as a black box. Rcpp makes it possible to do so while keeping the code very efficient (here more efficient than the optimized package) ! 
