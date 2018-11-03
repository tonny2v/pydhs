#ifndef GRAPH_H
#define GRAPH_H
constexpr auto MAX_OUT_DEGREE = 20;
constexpr auto MAX_IN_DEGREE = 20;

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <exception>

// TODO: pickle/serialize the graph

using namespace std;

namespace GraphException
{
    struct NotAccessible : std::exception
    {
        const char* what() const throw() { return "GRAPH_ERROR: OD pair not accessible"; }
    };

    struct GraphNotSet : std::exception
    {
        const char* what() const throw() { return "GRAPH_ERROR: Graph not yet set"; }
    };
}

struct Edge;

// vertex index by using its alias, used for nodes on mesh boarders.
struct Vertex {
    Vertex(const std::string &_id) {
        id = _id;
        idx = 0;
        in_cnt = 0;
        out_cnt = 0;
        in_edges.reserve(MAX_IN_DEGREE);
        out_edges.reserve(MAX_OUT_DEGREE);
    }
    ~Vertex() {
    }
    string id;
    int idx;
    vector<Edge*> in_edges;
    vector<Edge*> out_edges;
    int in_cnt;
    int out_cnt;
};

struct Edge {
    Edge(const string &_id, Vertex* _fv, Vertex* _tv) {
        id = _id;
        idx = 0;
        from_vertex = _fv;
        to_vertex = _tv;
    }
    ~Edge() {
    }
    string id;
    int idx;
    Vertex* from_vertex;
    Vertex* get_fv() {
        return from_vertex;
    }
    void set_fv(Vertex * _fv) {
        from_vertex = _fv;
    }
    
    Vertex* to_vertex;
    Vertex* get_tv() {
        return to_vertex;
    }
    void set_tv(Vertex * _tv) {
        to_vertex = _tv;
    }
};

class Graph {
private:
    set<string> vertex_ids;
    set<string> edge_ids;
    Vertex** vertices;
    Edge** edges;
    std::unordered_map<string, int> vid_to_idx;
    std::unordered_map<string, int> eid_to_idx;
    int m_cnt;
    int n_cnt;
public:
    Graph(int n, int m) {
        m_cnt = 0;
        n_cnt = 0;
        vertices = new Vertex*[n];
        edges = new Edge*[m];
    }
    
    ~Graph() {
        for (int i = 0; i < n_cnt; ++i)
            delete vertices[i];
        for (int i = 0; i < m_cnt; ++i)
            delete edges[i];
        delete[] vertices;
        vertices = nullptr;
        delete[] edges;
        edges = nullptr;
    }
    
    const boost::shared_ptr<Graph> make_reverse(){
        size_t m = get_edge_number();
        size_t n = get_vertex_number();
        //    Graph* gr = new Graph(int(n), int(m));
        boost::shared_ptr<Graph> gr (boost::make_shared<Graph>(n, m));
        for (unsigned int i = 0; i< m; ++i){
            gr->add_edge(get_edge(i)->id, get_edge(i)->to_vertex->id,get_edge(i)->from_vertex->id);
        }
        return gr;
    }
    
    inline Vertex** get_vertices() {
        return vertices;
    }
    
    inline Edge** get_edges() {
        return edges;
    }
    
    
    
    int get_vidx(const string &_vid) {
        if (vid_to_idx.find(_vid) == vid_to_idx.end())
            throw "ERROR: vertex not exist: " + _vid;
        return vid_to_idx[_vid];
    }
    
    int get_eidx(const string &_eid) {
        if (eid_to_idx.find(_eid) == vid_to_idx.end())
            throw "ERROR: edge not exist: " + _eid;
        return eid_to_idx[_eid];
    }
    
    // build graph methods
    
    void add_vertex(const string &_id) {
        if (vertex_ids.find(_id) == vertex_ids.end()) // do insertion only when the vertex hasn't been inserted
        {
            vertex_ids.insert(_id);
            Vertex* v = new Vertex(_id);
            vertices[n_cnt] = v;
            vid_to_idx[_id] = n_cnt;
            v->idx = n_cnt;
            n_cnt++;
        }
    }
    
    void add_edge(const string &_id, Vertex* _fv, Vertex* _tv) {
        if (edge_ids.find(_id) == edge_ids.end()) // do insertion only when the edge hasn't been inserted
        {
            edge_ids.insert(_id);
            Edge* e = new Edge(_id, _fv, _tv);
            edges[m_cnt] = e;
            //        _fv->out_edges[_fv->out_cnt] = e;
            _fv->out_edges.push_back(e);
            _fv->out_cnt++;
            //		_tv->in_edges[_tv->in_cnt] = e;
            _tv->in_edges.push_back(e); //has to be used together to ensure efficiency
            _tv->in_cnt++;
            eid_to_idx[_id] = m_cnt;
            e->idx = m_cnt;
            m_cnt++;
        }
    }
    
    void add_edge(const string &_id, const string &_fv_id, const string &_tv_id) {
        if (edge_ids.find(_id) == edge_ids.end()) // do insertion only when the edge hasn't been inserted
        {
            edge_ids.insert(_id);
            add_vertex(_fv_id);
            add_vertex(_tv_id);
            auto fv = get_vertex(_fv_id);
            auto tv = get_vertex(_tv_id);
            Edge* e = new Edge(_id, fv, tv);
            
            edges[m_cnt] = e;
            
            //        _fv->out_edges[_fv->out_cnt] = e;
            fv->out_edges.push_back(e);
            fv->out_cnt++;
            
            //		_tv->in_edges[_tv->in_cnt] = e;
            tv->in_edges.push_back(e); //has to be used together to ensure efficiency
            tv->in_cnt++;
            eid_to_idx[_id] = m_cnt;
            e->idx = m_cnt;
            m_cnt++;
        }
    }
    
    // get vertex methods
    inline Vertex* get_vertex (const string &_id) const{
        int idx = vid_to_idx.at(_id);
        return vertices[idx];
    }
    
    inline Vertex* get_vertex(int _idx) const{
        return vertices[_idx];
    }
    
    // get the common vertex of two edges
    Vertex* get_vertex(Edge* _e1, Edge* _e2) const{
        auto e1_fv = _e1->from_vertex;
        auto e1_tv = _e1->to_vertex;
        auto e2_fv = _e2->from_vertex;
        auto e2_tv = _e2->to_vertex;
        if (e1_fv == e2_fv || e1_fv == e2_tv)
            return e1_fv;
        else if (e1_tv == e2_fv || e1_tv == e2_tv)
            return e1_tv;
        else
            return nullptr;
    }
    
    // get edge methods
    inline Edge* get_edge(string _id) const{
        int idx = eid_to_idx.at(_id);
        return edges[idx];
    }
    
    inline Edge* get_edge(Vertex* _fv, Vertex* _tv) const{
        for (const auto& e1 : _fv->out_edges)
        {
            for (const auto& e2: _tv->in_edges){
                if (e1->id == e2->id) return get_edge(e1->id);
            }
        }
        return nullptr;
    }
    
    inline Edge* get_edge(string _fid, string _tid) const{
        return get_edge(get_vertex(_fid), get_vertex(_tid));
    }
    
    inline Edge* get_edge (int _idx) const{
        return edges[_idx];
    }
    
    // graph infomation methods
    inline size_t get_edge_number() const {
        return m_cnt;
    }
    inline size_t get_vertex_number() const{
        return n_cnt;
    }
};

#endif
