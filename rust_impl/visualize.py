import dhs
import networkx as nx
import matplotlib.pyplot as plt
import re

def get_bell2009_raw_data():
    try:
        with open("src/lib.rs", "r") as f:
            content = f.read()
        
        # Find the get_bell2009 function content
        # It starts with pub fn get_bell2009() -> Vec<(i32, i32, i32, f32, f32)> {
        # and ends with ]\n}
        match = re.search(r"pub fn get_bell2009.*?\s+vec!\[(.*?)\s+\]\s+\}", content, re.DOTALL)
        if not match:
            print("Could not find get_bell2009 in src/lib.rs")
            return []
        
        data_str = match.group(1)
        # Extract (eid, fv, tv, wm, wx)
        # Handles integers and floats
        pattern = re.compile(r"\((\d+),\s*(\d+),\s*(\d+),\s*([\d\.]+),\s*([\d\.]+)\)")
        matches = pattern.findall(data_str)
        
        raw_data = []
        for m in matches:
            raw_data.append((f"e{m[0]}", str(m[1]), str(m[2]), float(m[3]), float(m[4])))
        
        print(f"Extracted {len(raw_data)} edges from src/lib.rs")
        return raw_data
    except Exception as e:
        print(f"Error extracting data: {e}")
        return []

def visualize_64_node_network():
    # 1. Prepare Data
    raw_data = get_bell2009_raw_data()
    if not raw_data:
        return
    
    # 2. Build dhs graph
    edges_for_dhs = [(r[0], r[1], r[2]) for r in raw_data]
    graph = dhs.make_graph(edges_for_dhs, 64, len(raw_data))
    
    # 3. Calculate Dijkstra Potentials (Heuristic) from 1
    start_node = "1"
    end_node = "37"
    w_min = [r[3] for r in raw_data]
    w_max = [r[4] for r in raw_data]
    
    dijk = dhs.Dijkstra(graph)
    dijk.set_weights(w_min)
    dijk.run(start_node)
    potentials = dijk.get_potentials()
    
    # 4. Calculate Hyperpath (Ma2013) from 1 to 37
    ma = dhs.Ma2013(graph)
    ma.set_weights(w_min, w_max)
    ma.set_potentials(potentials)
    ma.run(start_node, end_node)
    
    hp_results = ma.hyperpath
    hp_probs = {eid: prob for eid, prob in hp_results}
    print(f"Hyperpath found with {len(hp_results)} edges.")

    # 5. Build NetworkX graph for plotting
    G = nx.DiGraph()
    for eid, u, v, wmin, wmax in raw_data:
        prob = hp_probs.get(eid, 0.0)
        G.add_edge(u, v, id=eid, prob=prob)

    # 6. Layout and Plotting
    plt.figure(figsize=(16, 16))
    
    # Grid layout
    pos = {}
    for i in range(1, 65):
        row = (i - 1) // 8
        col = (i - 1) % 8
        pos[str(i)] = (col, -row)

    # Draw Nodes
    nx.draw_networkx_nodes(G, pos, node_size=500, node_color="#ecf0f1", edgecolors="#2c3e50")
    nx.draw_networkx_labels(G, pos, font_size=9, font_weight='bold')

    # Draw all edges (faint background)
    nx.draw_networkx_edges(G, pos, edge_color="#bdc3c7", width=0.5, alpha=0.1, arrows=False)

    # Draw Hyperpath Edges (Width proportional to probability)
    active_edges = [(u, v) for u, v, d in G.edges(data=True) if d['prob'] > 0]
    
    if active_edges:
        for u, v, d in G.edges(data=True):
            if d['prob'] > 0:
                nx.draw_networkx_edges(
                    G, pos, 
                    edgelist=[(u, v)], 
                    width=1 + d['prob'] * 8, 
                    edge_color="#3498db", 
                    alpha=0.8,
                    arrowsize=20
                )

        # Add probability labels
        # Only show labels for probabilities > 0
        edge_labels = {
            (u, v): f"{d['prob']:.2f}" 
            for u, v, d in G.edges(data=True) if d['prob'] > 0
        }
        nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=12, font_color="#2980b9", font_weight='bold')

    plt.title(f"64-Node Hyperpath Visualization: {start_node} -> {end_node}\n(Real Weights from Bell 2009 Benchmark)", fontsize=22)
    plt.axis('off')
    
    # Save the visualization
    output_file = "64_node_hyperpath.png"
    plt.savefig(output_file)
    print(f"Visualization saved to '{output_file}'")
    
    if hp_results:
        print("\nHyperpath Edges (Sorted by Probability):")
        for eid, prob in sorted(hp_results, key=lambda x: x[1], reverse=True):
            print(f"  {eid}: {prob:.4f}")

if __name__ == "__main__":
    visualize_64_node_network()
