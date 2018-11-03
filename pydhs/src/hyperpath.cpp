//
//  hyperpath.cpp
//  MyGraph
//
//  Created by tonny.achilles on 5/26/14.
//  Copyright (c) 2014 tonny.achilles. All rights reserved.
//
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "hyperpath.h"
#include "fibheap.h"
#include "dijkstra.h"
#include "heap.h"
#include <algorithm>
#include <sstream>

#define LARGENUMBER 9999999999

Hyperpath::Hyperpath(Graph * const _g) {
    g = _g;
    size_t n = g->get_vertex_number();
    size_t m = g->get_edge_number();
    u_i = new float[n];
    f_i = new float[n];
    p_i = new float[n];


    u_a = new float[m];
    p_a = new float[m];
    open = new bool[m];
    close = new bool[m];

    h = new float[n];
    wmin = new float[m];
    wmax = new float[m];

    for (int i = 0; i < n; ++i) {
        u_i[i] = numeric_limits<float>::infinity();
        f_i[i] = 0.0;
        p_i[i] = 0.0;
        h[i] = 0.0;
    }

    for (int i = 0; i < m; ++i) {
        u_a[i] = numeric_limits<float>::infinity();
        p_a[i] = 0.0;
        open[i] = false;
        close[i] = false;
        wmin[i] = 0.0;
        wmax[i] = 0.0;
    }

}

Hyperpath::~Hyperpath() {
    delete[] u_i;
    u_i = nullptr;
    delete[] f_i;
    f_i = nullptr;
    delete[] p_i;
    p_i = nullptr;
    delete[] u_a;
    u_a = nullptr;
    delete[] p_a;
    p_a = nullptr;
    delete[] open;
    open = nullptr;
    delete[] close;
    close = nullptr;
    delete h;
    h = nullptr;
    wmin = nullptr;
    delete wmin;
    wmax = nullptr;
    delete wmax;
}


void Hyperpath::set_weights(const bp::object &_wmin, const bp::object &_wmax){
    size_t m = g->get_edge_number();
    for (int i=0; i<m; i++){
        wmin[i] = bp::extract<float>(_wmin[i]);
        wmax[i] = bp::extract<float>(_wmax[i]);
    }
    
}

void Hyperpath::set_potentials(const bp::object &_h){
    size_t n = g->get_vertex_number();
    for (int i=0; i<n; i++){
        h[i] = bp::extract<float>(_h[i]);
    }
}

//   const float * denotes a constant pointer while float * const denotes the pointed content is constant
//   since we may need to adjust weights_min and weights, the pointed content shouldn't be constant

// sf_di, link set overhead
void Hyperpath::run(const string&_oid, const string& _did) {

    HeapD<FHeap> heapD;
    Heap* heap = heapD.newInstance(g->get_edge_number());
    auto o_idx = g->get_vidx(_oid);
    auto d_idx = g->get_vidx(_did);

    //initialization
    vector<Edge*> po_edges;

    u_i[d_idx] = 0.0;
    p_i[o_idx] = 1.0;

    int j_idx = d_idx;
    int i_idx = 0;
    int a_idx = 0;

    // backward pass
    while (true) {
        auto j = g->get_vertex(j_idx);
        for (const auto &edge : j->in_edges) {
            a_idx = edge->idx;
            i_idx = edge->from_vertex->idx;
            j_idx = edge->to_vertex->idx;

            float temp = u_i[j_idx] + wmin[a_idx] + h[i_idx];
            if (u_a[a_idx] > temp) {
                u_a[a_idx] = temp;
                if (!close[a_idx]) {
                    if (!open[a_idx]) {
                        heap->insert(a_idx, u_a[a_idx]);
                        open[a_idx] = true;
                    } else {
                        heap->decreaseKey(a_idx, temp);
                    }
                }
            }
        }

        if (0 == heap->nItems()) {
            break;
        } else {
            a_idx = heap->deleteMin();
        }
        open[a_idx] = false;
        close[a_idx] = true;
        i_idx = g->get_edge(a_idx)->from_vertex->idx;
        j_idx = g->get_edge(a_idx)->to_vertex->idx;
        //updating
        float w_max = wmax[a_idx];
        float w_min = wmin[a_idx];

        if (u_i[i_idx] >= u_i[j_idx] + w_min) {
            float f_a = w_max == w_min ? LARGENUMBER : 1.0 / (w_max - w_min);
            float P_a = f_a / (f_i[i_idx] + f_a);

            if (f_i[i_idx] == 0) {
                u_i[i_idx] = u_i[j_idx] + w_max;
            } else {
                if (u_i[i_idx]
                        > (1 - P_a) * u_i[i_idx] + P_a * (u_i[j_idx] + w_min))
                    u_i[i_idx] = (1 - P_a) * u_i[i_idx]
                        + P_a * (u_i[j_idx] + w_min);
            }

            f_i[i_idx] += f_a;
            po_edges.push_back(g->get_edge(a_idx)); //hyperpath is saved by id index of links

        }

        if (u_i[j_idx] + w_min + h[i_idx] > u_i[o_idx])
            break;
        j_idx = i_idx;

    }

    // forward pass
    sort(po_edges.begin(), po_edges.end(),
            [&](Edge* a, Edge* b)->bool
            {
            return u_i[a->to_vertex->idx] + wmin[a->idx] > u_i[b->to_vertex->idx] + wmin[b->idx];
            });

    for (const auto &po_edge : po_edges) {
        auto a_idx = po_edge->idx;
        auto i_idx = po_edge->from_vertex->idx;
        auto j_idx = po_edge->to_vertex->idx;
        float w_max = wmax[a_idx];
        float w_min = wmin[a_idx];
        float f_a = w_max == w_min ? LARGENUMBER : 1.0 / (w_max - w_min);
        float P_a = f_a / f_i[i_idx];
        p_a[a_idx] = P_a * p_i[i_idx];
        p_i[j_idx] += p_a[a_idx];
    }

    for (const auto &po_edge : po_edges) {
        if (p_a[po_edge->idx] != 0)
            hyperpath.push_back(make_pair(po_edge->id, p_a[po_edge->idx]));
    }

    delete heap;
    heap = nullptr;
}

bp::list Hyperpath::get_hyperpath() {
    bp::list l;
    for (auto it = hyperpath.begin(); it != hyperpath.end(); it++) {
        bp::tuple e = bp::make_tuple((*it).first, (*it).second);
        l.append(e);
    }
    return l;
}

void Hyperpath::recover(){
    size_t n = g->get_vertex_number();
    size_t m = g->get_edge_number();

    for (int i = 0; i < n; ++i) {
        u_i[i] = numeric_limits<float>::infinity();
        f_i[i] = 0.0;
        p_i[i] = 0.0;
    }

    for (int i = 0; i < m; ++i) {
        u_a[i] = numeric_limits<float>::infinity();
        p_a[i] = 0.0;
        open[i] = false;
        close[i] = false;
    }

    hyperpath.clear();
    path_rec.clear();
}
