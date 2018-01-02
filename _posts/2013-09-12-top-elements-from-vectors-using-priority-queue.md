---
title: Getting indices of top elements from a vector using a priority queue
author: Romain Francois and Martin Morgan
license: GPL (>= 2)
tags: stl
summary: Using a stl data structure (a priority queue) to get the indices of
 the top elements of a vector
layout: post
src: 2013-09-12-top-elements-from-vectors-using-priority-queue.Rmd
---

This post is based on a question on 
[Stack Overflow](http://stackoverflow.com/questions/14821859/how-to-find-the-indices-of-the-top-10-000-elements-in-a-symmetric-matrix12k-x-1/14840614)
and more precisely on Martin Morgan's answer. 

The problem is to find the indices of top `n` elements from a vector. 

An inefficient way of doing this is to run `order` on the vector and then 
only keep the last `n` values: 


{% highlight r %}
top <- function(x, n) {
    tail(order(x), n)
}
{% endhighlight %}

This is inefficient because it requires sorting the entire vector, which is 
expensive. 

Instead, [Martin](http://stackoverflow.com/questions/14821859/how-to-find-the-indices-of-the-top-10-000-elements-in-a-symmetric-matrix12k-x-1/14840614#14840614)
's idea is to use a priority queue, this is a [container adapter](http://www.cplusplus.com/reference/queue/priority_queue/)
from the Standard Template Libary which is designed so that 
its top element is always the greatest. 

So Martin used a priority queue of `std::pair<double,int>`, using the 
default implementation of the "greater" operator for pairs which just 
does lexicographic ordering. The idea is then to only keep `n` such pairs
in the queue and feed it by scanning the vector. 

Here is Martin's full code: 


{% highlight cpp %}
#include <Rcpp.h>
#include <queue>

using namespace Rcpp;
using namespace std;

// [[Rcpp::export]]
IntegerVector top_i_pq(NumericVector v, int n) {
    typedef pair<double, int> Elt;
    priority_queue< Elt, vector<Elt>, greater<Elt> > pq;
    vector<int> result;

    for (int i = 0; i != v.size(); ++i) {
        if (pq.size() < n)
            pq.push(Elt(v[i], i));
        else {
            Elt elt = Elt(v[i], i);
            if (pq.top() < elt) {
                pq.pop();
                pq.push(elt);
            }
        }
    }

    result.reserve(pq.size());
    while (!pq.empty()) {
        result.push_back(pq.top().second + 1);
        pq.pop();
    }

    return wrap(result);
}
{% endhighlight %}

The version we present here uses the `Compare` template argument of the
`priority_queue` to control the comparison. This way, instead of storing 
pairs of (value, index) we will only store the indices and implement 
the comparison between two indices by going back to the data. 

For that purpose, we need to define the `IndexCompare` class, which we make a
template to handle different input types (integer vector, numeric vector, character
vector). 

Then we abstract some of the concept out of main function by providing 
our own queue template class : `IndexQueue` which is templated by the 
type of data that is in the vector. 

With these types, we can now implement the `top_index` template function, which 
once again is templated on the type of vector we deal with. Finally we have 
the attributes compatible `top_index` function that dispatches to the 
appropriate version using a simple `switch`



{% highlight cpp %}
#include <Rcpp.h>
#include <queue>

using namespace Rcpp;

template <int RTYPE>
class IndexComparator {
public:
    typedef typename Rcpp::traits::storage_type<RTYPE>::type STORAGE;
    
    IndexComparator(const Vector<RTYPE>& data_) : data(data_.begin()) {}
    
    inline bool operator()(int i, int j) const {
        return data[i] > data[j] || (data[i] == data[j] && j > i);    
    }

private:
    const STORAGE* data;
};

template <>
class IndexComparator<STRSXP> {
public:
    IndexComparator( const CharacterVector& data_ ) : data(data_.begin()) {}
    
    inline bool operator()(int i, int j) const {
        return (String)data[i] > (String)data[j] || (data[i] == data[j] && j > i );    
    }

private:
   const Vector<STRSXP>::const_iterator data;
};

template <int RTYPE>
class IndexQueue {
    public:
        typedef std::priority_queue<int, std::vector<int>, IndexComparator<RTYPE> > Queue;
        
        IndexQueue(const Vector<RTYPE>& data_) : comparator(data_), q(comparator), data(data_) {}
        
        inline operator IntegerVector() {
            int n = q.size();
            IntegerVector res(n);
            for( int i=0; i<n; i++) {
                // +1 for 1-based R indexing
                res[i] = q.top() + 1;
                q.pop();
            }
            return res;
        }
        inline void input( int i) { 
            // if( data[ q.top() ] < data[i] ) {
            if (comparator(i, q.top())) {
                q.pop(); 
                q.push(i);    
            }
        }
        inline void pop() { q.pop(); }
        inline void push(int i) { q.push(i); }
        
    private:
       IndexComparator<RTYPE> comparator;
       Queue q;  
       const Vector<RTYPE>& data;
};


template <int RTYPE>
IntegerVector top_index(Vector<RTYPE> v, int n) {
    int size = v.size();
    
    // not interesting case. Less data than n
    if( size < n){
        return seq( 0, n-1 );
    }
    
    IndexQueue<RTYPE> q( v );
    for( int i=0; i<n; i++) q.push(i);
    for( int i=n; i<size; i++) q.input(i);   
    return q;
}

// [[Rcpp::export]]
IntegerVector top_index(SEXP x, int n) {
    switch (TYPEOF(x)) {
    case INTSXP: return top_index<INTSXP>(x, n);
    case REALSXP: return top_index<REALSXP>(x, n);
    case STRSXP: return top_index<STRSXP>(x, n);
    default: stop("type not handled"); 
    }
    return IntegerVector() ; // not used
}


{% endhighlight %}

We will use the template implementation above for integer and numeric vectors. The 
`IndexCompare` keeps a reference to the internal data of the vector, as a 
raw pointer for better performance and defines the parenthesis operator
using this information. 

For character vectors, data is internally stored into an array of `SEXP` of 
type `CHARSXP`, which we can compare thanks to the implementation of the 
greater operator in the `String` class. We however need a specific implementation
because we need a cast to `String` to use the operator. 

The implementation of the operator takes into account ties. When there is a tie, we
want to retain the value that has the smallest index. This is the reason for this part 
of the operator : `|| (data[i] == data[j] && j > i )`

The `IndexQueue` template embeds a `priority_queue` with the right parameters, 
and implements: 
- `input` : this first compares the top of the queue and replaces it if necessary
- `pop` : delegates to `priority_queue`
- `push` : idem
- conversion to an `IntegerVector` for when we want to get the results. 

With this, the implementation of `top_index` is straightforward. We handle the 
case where there are less than `n` data points which is not interesting, then 
we `push` the first `n` points into the queue, and finally we `input` the rest of the
data. 

Let's check that we get what we want: 


{% highlight r %}
x <- rnorm(1000)

res_cpp <- top_index(x, 30L)
res_r   <- tail(order(x), 30L)  
identical(res_cpp, res_r)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
top_index(letters, 10) 
{% endhighlight %}



<pre class="output">
 [1] 17 18 19 20 21 22 23 24 25 26
</pre>

And then let us benchmark: 


{% highlight r %}
require(microbenchmark)
{% endhighlight %}



<pre class="output">
Loading required package: microbenchmark
</pre>



{% highlight r %}
x <- rnorm(1e5)
microbenchmark( 
   R_order = top(x, 100),
   cpp1    = top_i_pq( x, 100), 
   cpp2    = top_index( x, 100 )
)
{% endhighlight %}



<pre class="output">
Unit: microseconds
    expr       min         lq       mean   median         uq       max
 R_order 28264.304 28461.3320 29190.3675 28602.28 29412.4410 42631.973
    cpp1   745.554   755.0810   774.4438   762.33   777.0160  1030.393
    cpp2   447.278   455.8515   478.6965   464.09   471.9845  1472.173
 neval cld
   100   b
   100  a 
   100  a 
</pre>

