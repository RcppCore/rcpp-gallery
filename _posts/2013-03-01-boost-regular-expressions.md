---
title: Using Rcpp with Boost.Regex for regular expression
author: Dirk Eddelbuettel
license: GPL (>= 2)
tags: boost basics 
summary: This post provides a simple example of regular expression use via Boost
layout: post
src: 2013-03-01-boost-regular-expressions.Rmd
---                                                                                                                                                          
Gabor [asked](http://thread.gmane.org/gmane.comp.lang.r.rcpp/5019/focus=5023) 
about Rcpp use with regular expression libraries.  This post shows a very simple example, based on  
[one of the Boost.RegEx examples](http://www.boost.org/doc/libs/1_53_0/libs/regex/example/snippets/credit_card_example.cpp).

We need to set linker options. This can be as simple as


{% highlight r %}
Sys.setenv("PKG_LIBS"="-lboost_regex")
{% endhighlight %}


With that, the following example can be built:


{% highlight cpp %}

// cf www.boost.org/doc/libs/1_53_0/libs/regex/example/snippets/credit_card_example.cpp

#include <Rcpp.h>

#include <string>
#include <boost/regex.hpp>

bool validate_card_format(const std::string& s) {
   static const boost::regex e("(\\d{4}[- ]){3}\\d{4}");
   return boost::regex_match(s, e);
}

const boost::regex e("\\A(\\d{3,4})[- ]?(\\d{4})[- ]?(\\d{4})[- ]?(\\d{4})\\z");
const std::string machine_format("\\1\\2\\3\\4");
const std::string human_format("\\1-\\2-\\3-\\4");

std::string machine_readable_card_number(const std::string& s) {
   return boost::regex_replace(s, e, machine_format, boost::match_default | boost::format_sed);
}

std::string human_readable_card_number(const std::string& s) {
   return boost::regex_replace(s, e, human_format, boost::match_default | boost::format_sed);
}

// [[Rcpp::export]]
Rcpp::DataFrame regexDemo(std::vector<std::string> s) {
    int n = s.size();
    
    std::vector<bool> valid(n);
    std::vector<std::string> machine(n);
    std::vector<std::string> human(n);
    
    for (int i=0; i<n; i++) {
        valid[i]  = validate_card_format(s[i]);
        machine[i] = machine_readable_card_number(s[i]);
        human[i] = human_readable_card_number(s[i]);
    }
    return Rcpp::DataFrame::create(Rcpp::Named("input") = s,
                                   Rcpp::Named("valid") = valid,
                                   Rcpp::Named("machine") = machine,
                                   Rcpp::Named("human") = human);
}
{% endhighlight %}


We can test the function using the same input as the Boost example:


{% highlight r %}
s <- c("0000111122223333", "0000 1111 2222 3333", "0000-1111-2222-3333", "000-1111-2222-3333")
regexDemo(s)
{% endhighlight %}



<pre class="output">
                input valid          machine               human
1    0000111122223333 FALSE 0000111122223333 0000-1111-2222-3333
2 0000 1111 2222 3333  TRUE 0000111122223333 0000-1111-2222-3333
3 0000-1111-2222-3333  TRUE 0000111122223333 0000-1111-2222-3333
4  000-1111-2222-3333 FALSE  000111122223333  000-1111-2222-3333
</pre>


