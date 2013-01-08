---
title: Handling Strings with Rcpp
author: Kevin Ushey
license: GPL (>= 2)
tags: string vector
summary: Demonstrates how one might handle a vector of strings with `Rcpp`,
  in addition to returning output.
layout: post
src: 2013-01-08-strings_with_rcpp.Rmd
---
 
This is a quick example of how you might use Rcpp to send and receive R
'strings' to and from R. We'll demonstrate this with a few operations.
 
### Sort a String with R
 
Note that we can do this in R in a fairly fast way:

{% highlight r %}
my_strings <- c("apples", "and", "cranberries")
R_str_sort <- function(strings) {
  sapply( strings, USE.NAMES=FALSE, function(x) {
    intToUtf8( sort( utf8ToInt( x ) ) )
    })
  }
R_str_sort( my_strings )
{% endhighlight %}



<pre class="output">
[1] "aelpps"      "adn"         "abceeinrrrs"
</pre>

 
### Sort a String with C++/Rcpp
 
Let's see if we can re-create the output with Rcpp.


{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
std::vector< std::string > cpp_str_sort( std::vector< std::string > strings ) {
  
  int len = strings.size();
 
  for( int i=0; i < len; i++ ) {
    std::sort( strings[i].begin(), strings[i].end() );
  }
  
  return strings;
}
{% endhighlight %}

 
Note the main things we do here:

* Rcpp's attributes handle any `as`-ing and `wrap`-ing of vectors; we even just
specify our return type as `std::vector< std::string >`.
* We then call the `void` method `std::sort`, which can sort a string in place,
* ... and we return that vector of strings.
 
Now, let's test it, and let's benchmark it as well.


{% highlight r %}
cpp_str_sort( my_strings )
{% endhighlight %}



<pre class="output">
[1] "aelpps"      "adn"         "abceeinrrrs"
</pre>



{% highlight r %}
long_strings <- rep( paste( collapse="", sample( letters, 1E5, replace=TRUE ) ),
                     times=100 )
 
rbenchmark::benchmark( cpp_str_sort(long_strings),
                       R_str_sort(long_strings),
                       replications=3
                       )
{% endhighlight %}



<pre class="output">
                        test replications elapsed relative user.self
1 cpp_str_sort(long_strings)            3   0.898    1.000     0.883
2   R_str_sort(long_strings)            3   2.356    2.624     2.350
  sys.self user.child sys.child
1    0.014          0         0
2    0.007          0         0
</pre>

 
Note that the C++ implementation is quite a bit faster (on my machine). However,
`std::sort` will not handle UTF-8 encoded vectors.
 
Now, let's do something crazy -- let's see if we can use Rcpp to perform an
operation that takes a vector of strings, and returns a list of vectors of
strings. (Or, in R parlance, a list of vectors of type character).
 
We'll do a simple 'split', such that each string is split every `n` indices.
 
### Split a string at consecutive indices n
 

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
List cpp_str_split( std::vector< std::string > strings, int n ) {
  
  int num_strings = strings.size();
  
  List out(num_strings);
  
  for( int i=0; i < num_strings; i++ ) {
    
    int num_substr = strings[i].length() / n;
    std::vector< std::string > tmp;
    
    for( int j=0; j < num_substr; j++ ) {
      
      tmp.push_back( strings[i].substr( j*n, n ) );
      
    }
    
    out[i] = tmp;
    
  }
  
  return out;
}
{% endhighlight %}

 
Main things to notice:
* We declare the output to be a `List`,
* We form a `List` container of size `num_strings`,
* We construct the split strings one by one, then place them back into our 
output container (note how with `out[i] = tmp`, we can assign our vector
of strings directly as an element of the list),
* We return the list we constructed.
 

{% highlight r %}
cpp_str_split( c("abcd", "efgh", "ijkl"), 2 )
{% endhighlight %}



<pre class="output">
[[1]]
[1] "ab" "cd"

[[2]]
[1] "ef" "gh"

[[3]]
[1] "ij" "kl"
</pre>



{% highlight r %}
cpp_str_split( c("abc", "de"), 2 )
{% endhighlight %}



<pre class="output">
[[1]]
[1] "ab"

[[2]]
[1] "de"
</pre>

 
My solution is perhaps a bit deficient (bug or feature?) in that it truncates
any strings not long enough; ideally, we'd either improve the C++ code or form
an appropriate wrapper to the function in R (and warn the user if truncation
might occur).
 
Hopefully this gives you a better idea how you might use Rcpp to perform more
extensive string manipulation with R character vectors.
