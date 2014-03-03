---
title: Convex Hull of Polygon using Boost.Geometry
author: Sameer D'Costa 
license: GPL (>= 2)
tags: boost
summary: This post shows how to use custom as and wrap converters with Boost.Geometry. 
layout: post
src: 2014-02-23-boost-geometry-as-and-wrap-example.cpp
---
Rcpp can be used to convert basic R data types to and from 
[Boost.Geometry](http://www.boost.org/doc/libs/1_55_0b1/libs/geometry/doc/html/index.html) models. 

In this example, we take a matrix of 2d-points and convert it into a Boost.Geometry polygon. 
We then compute the convex hull of this polygon using the Boost.Geometry function 
`boost::geometry::convex_hull()`. The convex hull is then converted back to an R matrix. 

The conversions to and from R and Boost.Geometry types are are done using two custom 
`as()` and `wrap()` convenience converter functions which are implemented below, followed by
a function using them in order to take data from R, call a 
[Boost.Geometry](http://www.boost.org/doc/libs/1_55_0b1/libs/geometry/doc/html/index.html)
function, and return the result to R.




{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

typedef boost::tuple<double, double> point;
typedef boost::geometry::model::polygon<point, true, true> polygon; 

namespace Rcpp {

    // as<>() converter from R to Boost.Geometry's polygon type
    template <> polygon as(SEXP pointsMatrixSEXP) {
        // the coordinates are the rows of the (n x 2) matrix
        NumericMatrix pointsMatrix(pointsMatrixSEXP);
        polygon poly;
        for (int i = 0; i < pointsMatrix.nrow(); ++i) {
            double x = pointsMatrix(i,0);
            double y = pointsMatrix(i,1);
            point p(x,y);
            poly.outer().push_back(p); 
        }
        return (poly);
    } 
    
    // wrap() converter from Boost.Geometry's polygon to an R(cpp) matrix
    // The Rcpp NumericMatrix can be converted to/from a SEXP
    template <> SEXP wrap(const polygon& poly) {
        const std::vector<point>& points = poly.outer();
        NumericMatrix rmat(points.size(), 2);
        for (unsigned int i = 0; i < points.size(); ++i) {
            const point& p = points[i];
            rmat(i,0) = p.get<0>();
            rmat(i,1) = p.get<1>();
        }
        return Rcpp::wrap(rmat);
    }
}


// [[Rcpp::export]]
NumericMatrix convexHullRcpp(SEXP pointsMatrixSEXP){

    // Conversion of pointsMatrix here to boost::geometry polygon
    polygon poly = as<polygon>(pointsMatrixSEXP);

    polygon hull;
    // Compute the convex hull
    boost::geometry::convex_hull(poly, hull);

    // Convert hull into a NumericMatrixsomething that Rcpp can hand back to R
    return wrap(hull);
}
{% endhighlight %}


Now we can use R to replicate the `convex_hull` example 
[in the Boost.Geometry documentation](http://www.boost.org/doc/libs/1_55_0/libs/geometry/doc/html/geometry/reference/algorithms/convex_hull.html).

{% highlight r %}
points <- c(2.0, 1.3, 2.4, 1.7, 2.8, 1.8, 3.4, 1.2, 3.7, 1.6, 3.4, 2.0, 4.1, 3.0, 5.3, 2.6, 5.4, 1.2, 4.9, 0.8, 2.9, 0.7, 2.0, 1.3)
points.matrix <- matrix(points, ncol=2, byrow=TRUE)
points.matrix
{% endhighlight %}



<pre class="output">
      [,1] [,2]
 [1,]  2.0  1.3
 [2,]  2.4  1.7
 [3,]  2.8  1.8
 [4,]  3.4  1.2
 [5,]  3.7  1.6
 [6,]  3.4  2.0
 [7,]  4.1  3.0
 [8,]  5.3  2.6
 [9,]  5.4  1.2
[10,]  4.9  0.8
[11,]  2.9  0.7
[12,]  2.0  1.3
</pre>



{% highlight r %}
convexHullRcpp(points.matrix)
{% endhighlight %}



<pre class="output">
     [,1] [,2]
[1,]  2.0  1.3
[2,]  2.4  1.7
[3,]  4.1  3.0
[4,]  5.3  2.6
[5,]  5.4  1.2
[6,]  4.9  0.8
[7,]  2.9  0.7
[8,]  2.0  1.3
</pre>


In fact, because of the interplay because providing `as<>()` and
`wrap()` for the compiler, and how `compileAttributes()` works, we
can write the function more cleanly with the desired new types in
the interface.

{% highlight cpp %}
// [[Rcpp::export]]
polygon convexHullRcpp2(polygon pointsMatrixBG){
    polygon hull;
    boost::geometry::convex_hull(pointsMatrixBG, hull);
    return hull;
}
{% endhighlight %}


We can run this second function as well:

{% highlight r %}
convexHullRcpp2(points.matrix)
{% endhighlight %}



<pre class="output">
     [,1] [,2]
[1,]  2.0  1.3
[2,]  2.4  1.7
[3,]  4.1  3.0
[4,]  5.3  2.6
[5,]  5.4  1.2
[6,]  4.9  0.8
[7,]  2.9  0.7
[8,]  2.0  1.3
</pre>

