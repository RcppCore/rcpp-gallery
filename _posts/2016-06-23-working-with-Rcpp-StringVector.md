---
title: Working with Rcpp&#58;&#58;StringVector
author: Brian J. Knaus
license: GPL (>= 2)
tags: basics vector
summary: Demonstrates creating and modifying Rcpp&#58;&#58;StringVector
layout: post
src: 2016-06-23-working-with-Rcpp-StringVector.Rmd
---

Vectors are fundamental containers in R.
This makes them equally important in Rcpp.
Vectors can be useful for storing multiple elements of a common class (e.g., integer, numeric, character).
In Rcpp, vectors come in the form of NumericVector, CharacterVector, LogicalVector, StringVector and more.
Look in the header file Rcpp/include/Rcpp/vector/instantiation.h for more types.
Here we explore how to work with Rcpp::StringVector as a way to manage non-numeric data.


We typically interface with Rcpp by creating functions.
There are several ways to include Rcpp functions in R.
The examples here can be copied and pasted into a text file named 'source.cpp' and compiled with the command `Rcpp::sourceCpp("source.cpp")` made from the R command prompt.


## Initialization


Here we create a simple function which initializes an empty vector of three elements in size and returns it.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function() {
    Rcpp::StringVector myvector(3);
    return myvector;
}
{% endhighlight %}


We can call this function from R as follows.



{% highlight r %}
x <- basic_function()
x
{% endhighlight %}



<pre class="output">
[1] &quot;&quot; &quot;&quot; &quot;&quot;
</pre>


The first two lines are pretty much mandatory and you should copy and paste them into all your code until you understand them.
The first line tells the program to use Rcpp.
The second line exports this function for use, as opposed to keeping it as an internal function that is unavailable to users.
Some people like to include `using namespace Rcpp;` to load the namespace.
I prefer to use the scope operator (::) when calling functions.
This is a matter of style and is therefore somewhat arbitrary.
Whatever your perspective on this, its best to maintain consistency so that your code will be easier for others to understand.


We see that we've returned a vector of length three.
We can also see that the default value is a string which contains nothing ("").
This is not a vector of NAs (missing data), even though NAs are supported by Rcpp::StringVector.


## Accessing elements

The individual elements of a StringVector can be easily accessed.
Here we'll create an Rcpp function that accepts an Rcpp::StringVector as an argument.
We'll print the elements from within Rcpp.
Then we'll return the vector to R.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function( Rcpp::StringVector myvector ) {

    for( int i=0; i < myvector.size(); i++ ){
        Rcpp::Rcout << "i is: " << i << ", the element value is: " << myvector(i);
        Rcpp::Rcout << "\n";
    }
  
    return myvector;
}
{% endhighlight %}


After we've compiled it we can call it from R.



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x1
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;banana&quot; &quot;orange&quot;
</pre>



{% highlight r %}
x2 <- basic_function(x1)
{% endhighlight %}



<pre class="output">
i is: 0, the element value is: apple
i is: 1, the element value is: banana
i is: 2, the element value is: orange
</pre>



{% highlight r %}
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;banana&quot; &quot;orange&quot;
</pre>


We see that the R vector contains the elements "apple", "banana" and "orange."
Within Rcpp we print each element to standard out with `Rcpp::Rcout <<` statements.
And we see that these values are returned to the vector `x2`.


We've also introduced the method `.size()` which returns the number of elements in an object.
This brings up an important difference among C++ and R.
Many function names in R may contain periods.
For example, the function name `write.table()` delimits words with a period.
However, in C++ the period indicates a method.
This means that C++ object names can't include a period.
Camel code or underscores are good alternatives.


There are at least two other important issues to learn from the above example.
First, in R we typically access elements with square brackets.
While some C++ objects are also accessed with square brackets, the Rcpp::StringVector is accessed with either parentheses or square brackets.
In the case of the Rcpp::StringVector these appear interchangeable.
However, be very careful, they are different in other containers.
A second, but very important, difference between R and C++ is that in R the vectors are 1-based, meaning the first element is accessed with a 1.
In C++ the vectors are zero-based, meaning the first element is accessed with a zero.
This creates an opportunity for one-off errors.
If you notice that the number of elements you've passed to C++ and back are off by one element, this would be something good to check.


### Elements of elements

In C++, a std::string can be see as a vector of chars.
Individual elements of a Rcpp::StringVector behave similarly.
Accessing each element of a StringVector is similar to working with a std::string.
Here we access each character of the second element of our StringVector.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
void basic_function(Rcpp::StringVector x) {
    for(int i=0; i < x[1].size(); i++){
        Rcpp::Rcout << "i is: " << i << ", element is: ";
        Rcpp::Rcout << x[1][i];
        Rcpp::Rcout << "\n";
    }
}
{% endhighlight %}


And call the code from R.



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
{% endhighlight %}



<pre class="output">
i is: 0, element is: b
i is: 1, element is: a
i is: 2, element is: n
i is: 3, element is: a
i is: 4, element is: n
i is: 5, element is: a
</pre>


We see that we've accessed and printed the individual characters of the second element of the vector.
We accomplish this by using the square brackets to access element one of the vector, and then use a second set of square brackets to access each character of this element.


## Modifying elements

The modification of elements is fairly straight forward.
We use the index (begining at zero) to modify the vector elements.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector x) {
    Rcpp::StringVector myvector = x;
    myvector[1] = "watermelon";
    myvector(2) = "kumquat";
    return myvector;
}
{% endhighlight %}




{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;      &quot;watermelon&quot; &quot;kumquat&quot;   
</pre>


We've successfully changed the second element from 'banana' to 'watermelon' and the third element from 'orange' to 'kumquat.'
This also illustrates that Rcpp::StringVectors are flexible in their use of both square and round brackets.
Trying that with standard library containers will usually result in an error.



In the above example we've passed an Rcpp::StringVector to a function and returned a new Rcpp::StringVector.
By copying the container in this manner it may seem intuitive to work on it.
If efficient use of memory is desired it is important to realize that pointers are being passed to the Rcpp function.
This means we can create a function which returns void and modifies the elements we're interested in modifying without the overhead of copying the container.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
void basic_function(Rcpp::StringVector x) {
    x(1) = "watermelon";
}
{% endhighlight %}



{% highlight r %}
x1 <- c("apple", "banana", "orange")
basic_function(x1)
x1
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;      &quot;watermelon&quot; &quot;orange&quot;    
</pre>


## Erasing elements


If we want to remove an element from a StringVector we can use the `.erase()` method.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector x) {
    Rcpp::StringVector myvector = x;
    myvector.erase(1);
    return myvector;
}
{% endhighlight %}


And see our changes with R code.



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;orange&quot;
</pre>


We see that we've erased the second element from the array.


## Growing and shrinking Rcpp::StringVectors


If you have an Rcpp::StringVector and you want to add elements, you can use the method `.push_back()`.
While Rcpp has push functionality, it does not appear to have pop functionality.
However, using techniques learned above, we could use `object.erase(object.size())` to attain similar functionality.
Here I illustrate their use to remove an element and then add two elements.




{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector x) {
    x.erase( x.size() - 1 ); // C++ vectors are zero based so remember the -1!
  
    for(int i=0; i < x.size(); i++){
        Rcpp::Rcout << "i is: " << i << ", the element value is: " << x(i);
        Rcpp::Rcout << "\n";
    }
  
    x.push_back("avocado");
    x.push_back("kumquat");
    return x;
}
{% endhighlight %}


And implement our example in R.



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
{% endhighlight %}



<pre class="output">
i is: 0, the element value is: apple
i is: 1, the element value is: banana
</pre>



{% highlight r %}
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;   &quot;banana&quot;  &quot;avocado&quot; &quot;kumquat&quot;
</pre>


From the Rcpp output we see that we've removed the last element from the vector.
We also see that we've added two elements to the 'back' of the vector.


If we want to add to the front of our vector we can accomplish that as well.
There does not appear to be 'push_front' or 'pop_front' methods, but we have the tools necessary to accomplish these tasks.
We use the erase and insert methods to push and pop to the front of our Rcpp::StringVector.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector x) {
    x.erase(0);
    x.erase(0);
  
    x.insert(0, "avocado");
    x.insert(0, "kumquat");
    return x;
}
{% endhighlight %}



And implement our example in R.



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;kumquat&quot; &quot;avocado&quot; &quot;orange&quot; 
</pre>


In general, growing and shrinking data structures comes with a performance cost.
And if you're interested in Rcpp, you're probably interested in performance.
You'll typically be better off setting a container size and sticking with it.
But there are times when growing and shrinking your container can be really helpful.
My recommendation is to use this functionality sparingly.


## Missing data


In R we handle missing data with 'NAs.'
In C++ the concept of missing data does not exist.
Instead, some sort of placeholder, such as -999, has to be used.
The Rcpp containers do support missing data to help make the interface between R and C++ easy.
We can see this by continuing our existing example, but use it to set the second element as missing.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector myvector) {
    myvector(1) = NA_STRING;
    return myvector;
}
{% endhighlight %}



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  NA       &quot;orange&quot;
</pre>


## Finding other methods


The Rcpp header files contain valuable information about objects defined in Rcpp.
However, they're rather technical and may not be very approachable to the novice.
(This document is an attempt to help users bridge that gap between a novice and someone who reads headers.)
If you don't know where the header files are, you can use `.libPaths()` to list the locations of your libraries.
In one of these locations you should find a directory called 'Rcpp.'
Within this directory you should find a directory named 'include' which is where the headers are.
For example, the header for the String object on my system is at:
`Rcpp/include/Rcpp/String.h`


## Type conversion

Once we have our data in an Rcpp function we may want to make use of the functionality of C++ containers.
This will require us to convert our Rcpp container to another C++ form.
Once we've processed these data we may want convert them back to Rcpp (so we can return them to R).
A good example is converting an element of a StringVector to a std::string.


### Implicit type conversion

Conversion from an Rcpp:StringVector to a std::string is a compatible conversion.
This means we can accomplish this implicitly by simply setting the value of one container to the other.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector StringV_to_vstrings(Rcpp::StringVector StringV){
    std::vector<std::string> vstrings(StringV.size());
    int i;
    for (i = 0; i < StringV.size(); i++){
        vstrings[i] = StringV(i);
    }

    Rcpp::StringVector StringV2(StringV.size());
    StringV2 = vstrings;
    return(StringV2);
}

{% endhighlight %}



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- StringV_to_vstrings(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;banana&quot; &quot;orange&quot;
</pre>

Note that while we have to load each element of the std::vector<std::string> individually.
However, the loading of the Rcpp::StringVector has been vectorized so that it works similar to R vectors.



### Explicit type conversion

In some instances we may need explicit type conversion.
Rcpp provides an 'as' method to accomplish this.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector StringV_to_vstrings(Rcpp::StringVector StringV){
    std::vector<std::string> vstrings(StringV.size());
    int i;
    for (i = 0; i < StringV.size(); i++){
        vstrings[i] = Rcpp::as< std::string >(StringV(i));
    }

    Rcpp::StringVector StringV2(StringV.size());
    StringV2 = vstrings;
    return(StringV2);
}

{% endhighlight %}



{% highlight r %}
x1 <- c("apple", "banana", "orange")
x2 <- StringV_to_vstrings(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;banana&quot; &quot;orange&quot;
</pre>


Type conversion is a lengthy topic and is frequently specific to the types which are being converted to and from.
Hopefully this introduction is enough to get you started with the tools provided in Rcpp.


## Attributes


R objects include attributes which help describe the object.
This is another concept that is absent in C++.
Again, the Rcpp objects implement attributes to help us and to maintain a behavior that is similar to R.



{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector myvector) {
    std::vector< std::string > mystrings =  myvector.attr("names");
    mystrings[2] = "citrus";
    myvector.attr("names") = mystrings;
    return myvector;
}
{% endhighlight %}




{% highlight r %}
x1 <- c("apple", "banana", "orange")
names(x1) <- c("pome", "berry", "hesperidium")
x1
{% endhighlight %}



<pre class="output">
       pome       berry hesperidium 
    &quot;apple&quot;    &quot;banana&quot;    &quot;orange&quot; 
</pre>



{% highlight r %}
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
    pome    berry   citrus 
 &quot;apple&quot; &quot;banana&quot; &quot;orange&quot; 
</pre>


Here we've stored the names of the Rcpp:StringVector in a std::vector of strings.
We've then modified one of the elements and reset the names attribute with this changed vector.
This illustrates the use of standard library containers along with those provided by Rcpp.
But we need to be a little careful of what we're doing here.
If we store the values of our vector in a vector of std::string we lose our attributes because neither a std::vector or std::string has attributes.




{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::StringVector basic_function(Rcpp::StringVector myvector) {
    std::vector< std::string > mystrings(myvector.size()); 
    mystrings[0] = myvector(0);
    mystrings[1] = myvector(1);
    mystrings[2] = myvector(2);
    myvector = mystrings;
    return myvector;
}
{% endhighlight %}




{% highlight r %}
x1 <- c("apple", "banana", "orange")
names(x1) <- c("pome", "berry", "hesperidium")
x2 <- basic_function(x1)
x2
{% endhighlight %}



<pre class="output">
[1] &quot;apple&quot;  &quot;banana&quot; &quot;orange&quot;
</pre>


Note that while we can assign a vector of strings to a Rcpp::StringVector we can not do the inverse.
Instead we need to assign each element to the vector of strings.
And we need to remember to keep our square brackets and round brackets associated with the correct data structure.




## More information

Below are some links I've found useful in writing this document.
Hopefully you'll find them as gateways for your exploration of Rcpp.


* [Rcpp webpage](http://dirk.eddelbuettel.com/code/rcpp.html) Dirk Eddelbuettel's Rcpp webpage.
* [Rcpp Gallery](http://gallery.rcpp.org/) A collection of examples on Rcpp usage.
* Vignette on [Rcpp-attributes](http://dirk.eddelbuettel.com/code/rcpp/Rcpp-attributes.pdf).
* [Rcpp-quickref](http://cran.r-project.org/web/packages/Rcpp/vignettes/Rcpp-quickref.pdf) A 'Cheatsheet' on Rcpp usage.
* Hadley Wickham's [Advanced R](http://adv-r.had.co.nz/Rcpp.html) book.
* [Handling Strings with Rcpp](http://gallery.rcpp.org/articles/strings_with_rcpp/).
* [StringVector](http://statr.me/rcpp-note/api/Vector_ctor.html#_CPPv212StringVector) at Rcpp Note.

Once you've crossed from R to C++ there are many of sources of information online.
One of my favorites is included below.

* [Tutorial at cpluplus.com](http://www.cplusplus.com/doc/tutorial/).
