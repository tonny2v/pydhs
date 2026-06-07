import dhs
import networkx as nx
import matplotlib.pyplot as plt

def main():
    # 1. Prepare Data (Matching the structure expected by dhs.make_graph)
    # Format: (edge_id, from_id, to_id, w_min, w_max)
    # Using a small subset of the Bell 2009 network or a custom diamond for clarity
    raw_data = [
        ("e1", "1", "2", 1.5, 2.0),
        ("e2", "1", "3", 2.0, 3.0),
        ("e3", "2", "4", 1.0, 1.5),
        ("e4", "3", "4", 0.5, 2.5),
    ]
    
    # 2. Build the Graph
    # dhs.make_graph expects (edge_list, vertex_count, edge_count)
    # edge_list elements should be (id, from, to)
    edges_for_graph = [(r[0], r[1], r[2]) for r in raw_data]
    graph = dhs.make_graph(edges_for_graph, 4, 4)
    
    print(f"Graph built with {graph.vertex_num} vertices and {graph.edge_num} edges.")

    # 3. Shortest Path with Dijkstra
    weights = [r[3] for r in raw_data]  # Use w_min as weights
    dijk = dhs.Dijkstra(graph)
    dijk.set_weights(weights)
    dijk.run("1")
    
    path = dijk.get_path("1", "4")
    print(f"Dijkstra Shortest Path (1 -> 4): {path}")
    potentials = dijk.get_potentials()

    # 4. Hyperpath with Ma2013
    w_min = [r[3] for r in raw_data]
    w_max = [r[4] for r in raw_data]
    
    ma = dhs.Ma2013(graph)
    ma.set_weights(w_min, w_max)
    ma.set_potentials(potentials) # Use Dijkstra potentials as heuristic
    ma.run("1", "4")
    
    hyperpath_results = ma.hyperpath
    print("\nHyperpath Probabilities:")
    for edge_id, prob in hyperpath_results:
        print(f"  Edge {edge_id}: {prob:.4f}")

    # 5. Simple Visualization with NetworkX
    G = nx.DiGraph()
    for eid, u, v, wmin, wmax in raw_data:
        G.add_edge(u, v, id=eid, wmin=wmin, wmax=wmax)

    pos = nx.spring_layout(G)
    nx.draw(G, pos, with_labels=True, node_color='lightblue', node_size=800)
    
    # Highlight Hyperpath edges
    hp_edges = [edge_id for edge_id, prob in hyperpath_results]
    edge_labels = { (u, v): f"{G[u][v]['id']}" for u, v in G.edges() }
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels)
    
    print("\n[Info] Visualization script created. In a real environment, use plt.show().")

if __name__ == "__main__":
    main()
