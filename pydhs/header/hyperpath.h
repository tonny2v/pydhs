//
//  hyperpath.h
//  MyGraph
//
//  Created by tonny.achilles on 5/26/14.
//  Copyright (c) 2014 tonny.achilles. All rights reserved.
//

#ifndef HYPERPATH_H
#define HYPERPATH_H

#include <iostream>
#include <limits>
#include <string>
#include "algorithm.h"
#include "graph.h"
#include <unordered_map>
#include <boost/python.hpp>
using namespace std;
namespace bp = boost::python;
class Hyperpath: public Algorithm {
private:
    Graph *g;
    float *u_i; // node labels
    float* f_i; // weight sum
    float* p_i;
    
    float* wmin;
    float* wmax;
    float* h;

    float* u_a;
    float* p_a; // edge choice possiblities
    bool* open;
    bool* close;
    vector<pair<string, float> > hyperpath;
    vector<string> path_rec;
    
public:
    
    Hyperpath(Graph* const _g);
    
    ~Hyperpath();
    
    void set_weights(const bp::object &weights_min, const bp::object &weights_max);

    void set_potentials(const bp::object &h);

    bp::list get_hyperpath();
    
    void run(const string& _oid, const string& _did);

    void recover();
};

#endif /* defined(__MyGraph__hyperpath__) */
