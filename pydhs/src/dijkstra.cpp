// ========================================================
//  dijkstra.cpp
//  MyGraph
//
//  Created by tonny.achilles on 5/25/14.
//  Copyright (c) 2014 Jiangshan Ma. All rights reserved.
// ========================================================

#include "dijkstra.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <Python.h>
#include <boost/python/numpy.hpp>
#include <boost/python.hpp>
#include <exception>
using namespace std;
namespace bp = boost::python;
Dijkstra::Dijkstra(Graph* const _g)
{
    g = _g;
    size_t n = g->get_vertex_number();
    size_t m = g->get_edge_number();
    u = new float[n];
    pre_idx = new int[n];
    open = new bool[n]; // vertices with T labels
    close = new bool[n]; //vertices with P labels

    weights = new float[m]; //vertices with P labels

    
    for (unsigned int i=0;i<n;++i){
        u[i] = numeric_limits<float>::infinity();
        pre_idx[i] = -1;
        open[i] = false;
        close[i] = false;
    }

}

Dijkstra::~Dijkstra(){
    delete [] u;
    u = nullptr;
    delete [] pre_idx;
    pre_idx = nullptr;
    delete [] open;
    open = nullptr;
    delete [] close;
    close = nullptr;
    delete weights;
    weights = nullptr;
}

void Dijkstra::set_weights(const bp::object& _weight){
    size_t m = g->get_edge_number();
    for (unsigned int i=0;i<m;++i){
        weights[i] = bp::extract<float>(_weight[i]);
    }
}

void Dijkstra::recover(){
    size_t n = g->get_vertex_number();
    for (unsigned int i=0;i<n;++i){
        u[i] = numeric_limits<float>::infinity();
        pre_idx[i] = -1;
        open[i] = false;
        close[i] = false;
    }
}

bp::list Dijkstra::get_potentials(){
    bp::list potentials;
    size_t n = g->get_vertex_number();
    for (unsigned int i=0; i<n; ++i) {
        potentials.append(u[i]); 
    }
    return potentials;
}

void Dijkstra::run(string _oid){
    size_t n = g->get_vertex_number();
    HeapD<RadixHeap> heapD;
    Heap* heap = heapD.newInstance(n);

    auto o_idx = g->get_vidx(_oid);
    
    //initialization
    u[o_idx] = 0.0;
    heap->insert(o_idx, u[o_idx]);
    
    int vis_idx = 0;
    
    while (heap->nItems() > 0)
    {
        vis_idx = heap->deleteMin();
        auto vis = g->get_vertex(vis_idx);
        close[vis_idx] = true;
        open[vis_idx] = false;
        auto vis_out = vis->out_edges;
        for (const auto &e : vis_out)
        {
            auto v = e->to_vertex;
            float dist = 0.0;
            if (!close[v->idx])
            {
                dist = u[vis_idx] + weights[e->idx];
                
                if (dist < u[v->idx])
                {
                    u[v->idx] = dist;
                    if (open[v->idx])
                    {
                        heap->decreaseKey(v->idx, dist);
                    }
                    else
                    {
                        heap->insert(v->idx, dist);
                        open[v->idx] = true;
                    }
                    pre_idx[v->idx] = vis_idx;
                }
            }
        }
    }
    delete heap;
    heap = nullptr;
}

bp::list Dijkstra::get_path(string _oid, string _did) {
    bp::list path;
    auto d_idx = g->get_vertex(_did)->idx;
    int idx = d_idx;
    do {
        path.append(g->get_vertex(idx)->id);
        idx = pre_idx[idx];
    } while (idx != -1);
    path.reverse();
    if (path[0] != _oid) {
        const string &s = "ERROR: " + _did + " unaccessible from " + _oid;
        PyErr_SetString(PyExc_Exception, s.c_str());
    }
    return path;
}
