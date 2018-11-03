#include <boost/python.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <boost/python/numpy.hpp>
#include <boost/make_shared.hpp>
#include <boost/python/tuple.hpp>
#include "graph.h"
#include "stdio.h"
#include "hyperpath.h"
#include "dijkstra.h"
#include <set>
#include <boost/python/exception_translator.hpp>
#include <boost/python/with_custodian_and_ward.hpp>

using namespace boost::python;
namespace bp = boost::python;
using namespace std;

// ------------ pickle settings ----------------------
// TODO: this is not solved yet...
struct graph_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(Graph const& g) { return boost::python::make_tuple(g.get_vertex_number(), g.get_edge_number()); }

    static boost::python::tuple
    getstate(boost::python::object g_obj){
        Graph const &g = boost::python::extract<Graph const&>(g_obj);
        // serialize as vertices, saves space
        /*        boost::python::tuple vertices;
         for (int i = 0; i < g.get_vertex_number(); ++i) {
         auto v_i = g.get_vertex(i);
         boost::python::tuple in_edges;
         boost::python::tuple out_edges;
         for (int in = 0; in < v_i->in_cnt ; ++in) {
         in_edges[in] = v_i->in_edges[in]->id;
         }
         
         for (int in = 0; in < v_i->out_cnt ; ++in) {
         out_edges[in] = v_i->out_edges[in]->id;
         }
         // NOTE: the storage idx may different after de-pickled (actually the same?)
         vertices[i] = boost::python::make_tuple(v_i->id, in_edges, out_edges);
         }
         return vertices;
         */
        boost::python::tuple edges;
        // serialize as edges
        boost::python::list l;
        for (unsigned int i = 0; i < g.get_edge_number(); ++i) {
            l.append(g.get_edge(i)->id);
            l.append(g.get_edge(i)->from_vertex->id);
            l.append(g.get_edge(i)->to_vertex->id);
            edges[i] = l;
        }
        return edges;
    }
    static void
    setstate(Graph &g, boost::python::tuple state) {
        for(unsigned int i = 0; i< g.get_edge_number(); ++i)
        {
            string eid = boost::python::extract<string>(state[i][0]);
            string fid = boost::python::extract<string>(state[i][1]);
            string tid = boost::python::extract<string>(state[i][2]);
            g.add_edge(eid, fid, tid);
        }
    }
};

// ---------------------------------------------------------

// ------------ Exception translations ----------------------
void translate_notaccessible(const GraphException::NotAccessible & e)
{
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

void translate_graphnotset(const GraphException::GraphNotSet & e)
{
    PyErr_SetString(PyExc_RuntimeError, e.what());
}
// ---------------------------------------------------------

// describe the array by telling number of vertices and edges
const bp::list describe(const bp::object &array) {
    size_t m = bp::len(array);
    std::set<string> vertices_set;
    for (unsigned int i = 0; i < m; ++i) {
        string fid = extract<string>(array[i][1]);
        string tid = extract<string>(array[i][2]);
        vertices_set.insert(fid);
        vertices_set.insert(tid);
    }
    size_t n = vertices_set.size();
    bp::list l;
    l.append(n);
    l.append(m);
    return l;
}

// the input should by m rows, 3 columns array
const boost::shared_ptr<Graph> make_graph(const bp::object& array, int n, int m) {
    boost::shared_ptr<Graph> g (boost::make_shared<Graph>(n, m));
    for (int i = 0; i < bp::len(array); ++i) {
        string eid = extract<string>(array[i][0]);
        string fid = extract<string>(array[i][1]);
        string tid = extract<string>(array[i][2]);
        g->add_edge(eid, fid, tid);
    }
    return g;
}

BOOST_PYTHON_MODULE(dhs)
{
    // disable C++ auto docstring, keep user-defined docstring and C++ signature
    //docstring_options local_docstring_options(true, true, false);

    // keep user-defined docstring only
    docstring_options local_docstring_options(true, false, false);

    // Register exceptions
    register_exception_translator<GraphException::NotAccessible>(&translate_notaccessible);
    register_exception_translator<GraphException::GraphNotSet>(&translate_graphnotset);

    /// ************************************************************************
    ///                                 Vertex
    /// ************************************************************************
    class_<Vertex> pyVertex("Vertex", "Vertex type\n", init<string>(args("id"),
                "Create a Vertex with an id string\n"));
    pyVertex.def_readwrite("id", &Vertex::id, "Vertex id string\n");
    pyVertex.def_readonly("idx", &Vertex::idx, "Internal vertex index integer\n");
    pyVertex.def_readonly("in_cnt", &Vertex::in_cnt, "Number of incoming edges\n");
    pyVertex.def_readonly("out_cnt", &Vertex::out_cnt, "Number of outgoing edges\n");
    pyVertex.def_readwrite("in_edges", &Vertex::in_edges, "List of incoming edges\n");
    pyVertex.def_readwrite("out_edges", &Vertex::out_edges, "List of outgoing edges\n");

    /// ************************************************************************
    ///                                 Edge
    /// ************************************************************************
    class_<Edge> pyEdge("Edge","Edge type\n", init<string, Vertex*, Vertex*>(args("id","fv","tv"),
                "Create an edge from two vertices\n"));
    pyEdge.def_readwrite("id", &Edge::id, "Edge id string\n");
    pyEdge.def_readonly("idx", &Edge::idx, "Internal edge index integer\n");

    // python won't delete the pointer if using reference_existing_object policy
    // on the contrary, if using manage_new_object, the pointer deletion will be python's duty.
    pyEdge.def("get_fv", &Edge::get_fv,
               return_value_policy<reference_existing_object>(), "from/tail vertex\n");
    pyEdge.def("get_tv", &Edge::get_tv,
               return_value_policy<reference_existing_object>(), "to/head vertex\n");

    /// ************************************************************************
    ///                                 Graph
    /// ************************************************************************
    //
    // manually create two function pointers to enable function overload.
    // autooverloading only appies to void functions
    void (Graph::*add_edge_v)(const string &_id, Vertex* _fv,
                              Vertex* _tv) = &Graph::add_edge;
    void (Graph::*add_edge_s)(const string &_id, const string &_fv_id,
                              const string &_tv_id) = &Graph::add_edge;

    // get vertex overloading
    Vertex* (Graph::*get_vertex_byid)(const string &id) const = &Graph::get_vertex;
    Vertex* (Graph::*get_vertex_byidx)(int idx) const = &Graph::get_vertex;

    // get edge overloading
    Edge* (Graph::*get_edge_byid)(string id) const = &Graph::get_edge;
    Edge* (Graph::*get_edge_byidx)(int idx) const= &Graph::get_edge;

    // shared_ptr should be added to the class declaration
    class_<Graph, boost::shared_ptr<Graph> >("Graph", "Graph type\n", init<int, int>(args("n","m"),
            "Graph(n,m)\n\n"
            "Create a graph contains maximum n vertices and m edges\n\n"
            "Parameters\n"
            "----------\n"
            "n, m : int\n\n"
            "Returns\n"
            "----------\n"
            "Graph type\n\n"
            "Examples\n"
            "----------\n"
            "# create Graph g with 2 vertices and 1 edges at maximum\n"
            ">>>g = Graph(2,1)\n"))
        .def("add_vertex", &Graph::add_vertex)
        .def_pickle(graph_pickle_suite())
        .def("add_edge", add_edge_v,
            "add_edge(name, fv, tv)\n\n"
            "Add an edge by vertex\n\n"
            "Parameters\n"
            "----------\n"
            "name : string\n"
            "fv, tv : Vertex type\n\n"
            "Returns\n"
            "----------\n"
            "None\n\n"
            "Examples\n"
            "----------\n"
            ">>>g = Graph(2,1)\n"
            ">>>g.add_vertex('v1')\n"
            ">>>g.add_vertex('v2')\n"
            ">>>fv = g.get_vertex('v1')\n"
            ">>>tv = g.get_vertex('v2')\n"
            ">>>g.add_edge('e1',fv,tv)\n")
        .def("add_edge", add_edge_s,
            "add_edge(name, fv_name, tv_name)\n\n"
            "Add an edge by vertex strings\n\n"
            "Parameters\n"
            "----------\n"
            "name, fv_name, tv_name : string\n\n"
            "Returns\n"
            "----------\n"
            "None\n\n"
            "Examples\n"
            "----------\n"
            ">>>g = Graph(2,1)\n"
            ">>>g.add_edge('e1','v1','v2')\n\n"
            "Note: add_edge will create the vertices if not existed\n")
        .def_readonly("edge_num", &Graph::get_edge_number, "Number of edges")
        .def_readonly("vertex_num", &Graph::get_vertex_number, "Number of vertices")
        .def("get_vertex", get_vertex_byid, return_value_policy<reference_existing_object>())
        .def("get_vertex", get_vertex_byidx, return_value_policy<reference_existing_object>())
        .def("reverse", &Graph::make_reverse)
        .def("get_edge", get_edge_byid, return_value_policy<reference_existing_object>())
        .def("get_edge", get_edge_byidx, return_value_policy<reference_existing_object>());

    // Graph from array
//    boost::python::numeric::array::set_module_and_type("numpy", "ndarray");
    //	def("make_graph", make_graph, return_value_policy<manage_new_object>());
    //no need to use manage_new_object since shared_ptr is used
    def("make_graph", make_graph,
            "make_graph(arr, n, m)\n\n"
            "Make a graph from edge array\n\n"
            "Parameters\n"
            "----------\n"
            "arr : array-like\n"
            "   m*3 array with (eid, fvid, tvid) at each row\n"
            "n : int\n"
            "   number of vertices \n"
            "m : int\n"
            "   number of edges \n\n"
            "Returns\n"
            "----------\n"
            "Graph type\n\n"
            "Examples\n"
            "----------\n"
            ">>>arr = [['e1','v1','v2'],['e2','v2','v3']]\n"
            ">>>g = make_graph(arr, *describe(arr))\n");

    def("describe", describe,
            "describe(arr)"
            "Calculate number of vertices and edges from an array\n\n"
            "Parameters\n"
            "----------\n"
            "arr : array-like\n"
            "   m*3 array with (eid, fvid, tvid) at each row\n\n"
            "Returns\n"
            "----------\n"
            "out : list\n"
            "   the 1st element is vertice number n\n"
            "   the 2nd element is edge number m\n\n"
            "Examples\n"
            "----------\n"
            ">>>arr = [['e1','v1','v2'],['e2','v2','v3']]\n"
            ">>>describe(arr)\n"
            "[3, 2]\n");

    /// ************************************************************************
    ///                Dijkstra for node potential generation
    /// ************************************************************************
    class_<Dijkstra> pyDijkstra("Dijkstra",
            init<Graph*>(args("g"),"Dijkstra(g)\n\n"
                ">>>alg = Dijkstra(g)\n"));

    pyDijkstra.def("run", &Dijkstra::run,
        ">>>alg.run('oid', h)\n"
        );

    pyDijkstra.def("get_potentials", &Dijkstra::get_potentials,
        ">>>alg.get_potentials()\n"
        );

    pyDijkstra.def("set_weights", &Dijkstra::set_weights,
        ">>>alg.set_weights(array-like)\n"
        );

    pyDijkstra.def("get_path", &Dijkstra::get_path,
        ">>>alg.get_path(oid, did)\n"
        );

    pyDijkstra.def("recover", &Dijkstra::recover,
        ">>>alg.recover()\n"
        );

    /// ************************************************************************
    ///                                 Hyperpath
    /// ************************************************************************
    class_<Hyperpath> pyHyperpath("Ma2013",
            "Dijkstra-Hyperstar algorithm in Ma, J., Fukuda, D. and Schmoecker, J.D. 2013\n"
            "Faster hyperpath generating algorithms for vehicle navigation\n"
            "Transportmetrica A: Transport Science, Vol. 9, 925 – 948.\n"
            "http://www.tandfonline.com/doi/abs/10.1080/18128602.2012.719165\n",
            init<Graph*>(args("g"),"Ma2013(g)\n\n"
                "Create an algorithm instance for a give graph\n\n"
                "Parameters\n"
                "----------\n"
                "g : Graph type\n\n"
                "Returns\n"
                "----------\n"
                "Hyperpath type\n\n"
                "Examples\n"
                "----------\n"
                ">>>g = Graph(2,1)\n"
                ">>>g.add_edge('e1','v1','v2')\n"
                ">>>alg = ma2013(g)\n"));

    pyHyperpath.def("set_weights", &Hyperpath::set_weights);

    pyHyperpath.def("set_potentials", &Hyperpath::set_potentials);

    pyHyperpath.def("run", &Hyperpath::run,
        "run(fv, tv)\n\n"
        "Run algorithm to calculate the exact hyperpath \n\n"
        "Parameters\n"
        "----------\n"
        "fv, tv : string\n"
        "   names of from vertex and to vertex\n"
        "Returns\n"
        "----------\n"
        "None\n\n"
        "Examples\n"
        "----------\n"
        ">>>g = Graph(2,1)\n"
        ">>>g.add_edge('e1','v1','v2')\n"
        ">>>g.add_edge('e2','v2','v3')\n"
        ">>>alg = Ma2013(g)\n"
        ">>>w_min, w_max, h = [[1.2, 1.5], [0.2, 0.4], [0,0]]\n"
        ">>>alg.set_weights(w_min, w_max)\n"
        ">>>alg.set_potentials(h)"
        ">>>alg.run('v1','v3')\n"
        );
    pyHyperpath.def_readonly("hyperpath", &Hyperpath::get_hyperpath,
            "Hyperpath result \n\n"
            "Note: the first element is edge index, the second element is choice possibility\n");

    pyHyperpath.def("recover", &Hyperpath::recover,
            "recover() \n");
}
