use dhs::{get_bell2009, Dijkstra, Graph, Hyperpath};

fn print_hyperpath(g: &Graph, w_min: &[f32], w_max: &[f32], results: &[(String, f32)]) {
    println!("{:<10} {:>12} {:>8} {:>8} {:>10}", "Edge", "From->To", "W_min", "W_max", "Prob");
    println!("{}", "-".repeat(55));

    let mut total_prob = 0.0f32;
    for (edge_id, prob) in results {
        let edge_idx = g.eid_to_idx.get(edge_id).unwrap();
        let edge = &g.edges[*edge_idx];
        let from_id = &g.vertices[edge.from_vertex].id;
        let to_id = &g.vertices[edge.to_vertex].id;
        let wm = w_min[*edge_idx];
        let wx = w_max[*edge_idx];
        total_prob += prob;
        println!(
            "{:<10} {:>4}->{:<4} {:>8.4} {:>8.4} {:>10.6}",
            edge_id, from_id, to_id, wm, wx, prob
        );
    }

    println!("{}", "-".repeat(55));
    println!("Total probability sum: {:.6}", total_prob);
}

fn main() {
    let data = get_bell2009();

    // Build graph
    let mut g = Graph::new();
    let mut w_min = Vec::new();
    let mut w_max = Vec::new();

    for (eid, fv, tv, wm, wx) in &data {
        let edge_id = format!("e{}", eid);
        let from_id = format!("{}", fv);
        let to_id = format!("{}", tv);
        g.add_edge(&edge_id, &from_id, &to_id);
        w_min.push(*wm);
        w_max.push(*wx);
    }

    println!("=== Bell 2009 Network ===");
    println!("Vertices: {}", g.get_vertex_number());
    println!("Edges:    {}", g.get_edge_number());

    let origin = if let Some(arg) = std::env::args().nth(1) {
        arg
    } else {
        "1".to_string()
    };
    let destination = if let Some(arg) = std::env::args().nth(2) {
        arg
    } else {
        "37".to_string()
    };

    // Step 1: Dijkstra from origin to compute potentials (lower bounds from each vertex to origin)
    let mut dijk = Dijkstra::new(&g);
    dijk.set_weights(w_min.clone());
    dijk.run(&origin);
    let h = dijk.get_potentials();

    println!("\n=== Dijkstra Potentials (from vertex \"{}\") ===", origin);
    for i in 0..g.get_vertex_number() {
        let vid = &g.vertices[i].id;
        let pot = h[i];
        print!("  {}={:.4}", vid, pot);
        if (i + 1) % 8 == 0 {
            println!();
        }
    }
    println!();

    // Step 2: Run Hyperpath with Dijkstra potentials
    let mut hp = Hyperpath::new(&g, w_min.clone(), w_max.clone());
    hp.set_potentials(h);
    hp.run(&origin, &destination);

    let results = hp.get_hyperpath();

    println!("\n=== Hyperpath (with potentials): \"{}\" -> \"{}\" ===", origin, destination);
    println!("Hyperpath edges: {}", results.len());
    print_hyperpath(&g, &w_min, &w_max, &results);

    // Step 3: Run with zero potentials for comparison
    let mut hp2 = Hyperpath::new(&g, w_min.clone(), w_max.clone());
    let h_zero = vec![0.0; g.get_vertex_number()];
    hp2.set_potentials(h_zero);
    hp2.run(&origin, &destination);

    let results2 = hp2.get_hyperpath();

    println!("\n=== Hyperpath (zero potentials): \"{}\" -> \"{}\" ===", origin, destination);
    println!("Hyperpath edges: {}", results2.len());
    print_hyperpath(&g, &w_min, &w_max, &results2);
}
