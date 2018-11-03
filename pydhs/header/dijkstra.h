// ========================================================
//  dijkstra.h
//  MyGraph
//
//  Created by tonny.achilles on 5/25/14.
//  Copyright (c) 2014 Jiangshan Ma. All rights reserved.
// ========================================================

#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <limits>
#include <string>
#include "algorithm.h"
#include "graph.h"
#include "radixheap.h"
#include <boost/python/numpy.hpp>
namespace bp = boost::python;
class Dijkstra :
public Algorithm
{
private:
    
    Graph* g;
    
    float* u;
    
    int* pre_idx;
    
    bool* open;
    
    bool* close;
    
    float* weights;
    
public:
    
    // or const & here: passing by reference or passing a pointer
    Dijkstra(Graph* const _g);
    
    ~Dijkstra();

    void set_weights(const bp::object& _weight); 

    void recover(); 

    void run(string _oid);
    
    bp::list get_potentials();

    bp::list get_path(string _oid, string _did);
    
};
#endif /* DIJKSTRA_H_ */
