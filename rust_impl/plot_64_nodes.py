import networkx as nx
import matplotlib.pyplot as plt

def get_bell2009_edges():
    edges = []
    # Horizontal edges
    for row in range(8):
        for col in range(7):
            u = row * 8 + col + 1
            v = u + 1
            edges.append((str(u), str(v)))
            edges.append((str(v), str(u)))
    
    # Vertical edges
    for row in range(7):
        for col in range(8):
            u = row * 8 + col + 1
            v = u + 8
            edges.append((str(u), str(v)))
            edges.append((str(v), str(u)))
    
    return edges

def plot_network():
    G = nx.DiGraph()
    edges = get_bell2009_edges()
    G.add_edges_from(edges)
    
    # Create a grid layout
    pos = {}
    for i in range(1, 65):
        row = (i - 1) // 8
        col = (i - 1) % 8
        pos[str(i)] = (col, -row) # -row to have node 1 at top-left
    
    plt.figure(figsize=(12, 12))
    nx.draw(G, pos, with_labels=True, node_color='lightblue', 
            node_size=500, font_size=8, arrows=True, arrowsize=10)
    
    plt.title("Bell 2009 Network (64 Nodes, 8x8 Grid)")
    plt.savefig("bell2009_network.png")
    print("Network plot saved to 'bell2009_network.png'")

if __name__ == "__main__":
    plot_network()
