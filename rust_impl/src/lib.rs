use std::collections::{HashMap, HashSet, BinaryHeap};
use std::cmp::Ordering;
use pyo3::prelude::*;

// ===========================================================================
// Internal types — kept public for use by main.rs and tests
// ===========================================================================

#[derive(Debug, Clone)]
pub struct Vertex {
    pub id: String,
    pub idx: usize,
    pub in_edges: Vec<usize>,
    pub out_edges: Vec<usize>,
}

#[derive(Debug, Clone)]
pub struct Edge {
    pub id: String,
    pub idx: usize,
    pub from_vertex: usize,
    pub to_vertex: usize,
}

pub struct Graph {
    pub vertex_ids: HashSet<String>,
    pub edge_ids: HashSet<String>,
    pub vertices: Vec<Vertex>,
    pub edges: Vec<Edge>,
    pub vid_to_idx: HashMap<String, usize>,
    pub eid_to_idx: HashMap<String, usize>,
}

impl Graph {
    pub fn new() -> Self {
        Self {
            vertex_ids: HashSet::new(),
            edge_ids: HashSet::new(),
            vertices: Vec::new(),
            edges: Vec::new(),
            vid_to_idx: HashMap::new(),
            eid_to_idx: HashMap::new(),
        }
    }

    pub fn add_vertex(&mut self, id: &str) {
        if !self.vertex_ids.contains(id) {
            let idx = self.vertices.len();
            self.vertex_ids.insert(id.to_string());
            self.vid_to_idx.insert(id.to_string(), idx);
            self.vertices.push(Vertex {
                id: id.to_string(),
                idx,
                in_edges: Vec::new(),
                out_edges: Vec::new(),
            });
        }
    }

    pub fn add_edge(&mut self, id: &str, from_id: &str, to_id: &str) {
        if !self.edge_ids.contains(id) {
            self.add_vertex(from_id);
            self.add_vertex(to_id);

            let from_idx = *self.vid_to_idx.get(from_id).expect("from_id not found");
            let to_idx = *self.vid_to_idx.get(to_id).expect("to_id not found");
            let edge_idx = self.edges.len();

            self.edge_ids.insert(id.to_string());
            self.eid_to_idx.insert(id.to_string(), edge_idx);

            self.edges.push(Edge {
                id: id.to_string(),
                idx: edge_idx,
                from_vertex: from_idx,
                to_vertex: to_idx,
            });

            self.vertices[from_idx].out_edges.push(edge_idx);
            self.vertices[to_idx].in_edges.push(edge_idx);
        }
    }

    pub fn get_vertex_number(&self) -> usize {
        self.vertices.len()
    }

    pub fn get_edge_number(&self) -> usize {
        self.edges.len()
    }

    pub fn make_reverse(&self) -> Self {
        let mut g = Graph::new();
        for edge in &self.edges {
            let to_id = &self.vertices[edge.to_vertex].id;
            let from_id = &self.vertices[edge.from_vertex].id;
            g.add_edge(&edge.id, to_id, from_id);
        }
        g
    }
}

// ===========================================================================
// Heap helper types
// ===========================================================================

#[derive(Debug, Clone, Copy, PartialEq)]
struct State {
    idx: usize,
    val: f32,
}

impl Eq for State {}

impl PartialOrd for State {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for State {
    fn cmp(&self, other: &Self) -> Ordering {
        other.val.partial_cmp(&self.val).unwrap_or(Ordering::Equal)
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
struct VertexState {
    idx: usize,
    dist: f32,
}

impl Eq for VertexState {}

impl PartialOrd for VertexState {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for VertexState {
    fn cmp(&self, other: &Self) -> Ordering {
        other.dist.partial_cmp(&self.dist).unwrap_or(Ordering::Equal)
    }
}

// ===========================================================================
// Hyperpath (Ma2013) algorithm
// ===========================================================================

pub struct Hyperpath<'a> {
    g: &'a Graph,
    u_i: Vec<f32>,
    f_i: Vec<f32>,
    p_i: Vec<f32>,
    wmin: Vec<f32>,
    wmax: Vec<f32>,
    u_a: Vec<f32>,
    p_a: Vec<f32>,
    open: Vec<bool>,
    close: Vec<bool>,
    h: Vec<f32>,
    hyperpath: Vec<(String, f32)>,
}

impl<'a> Hyperpath<'a> {
    pub fn new(g: &'a Graph, wmin: Vec<f32>, wmax: Vec<f32>) -> Self {
        let n = g.get_vertex_number();
        let m = g.get_edge_number();
        Self {
            g,
            u_i: vec![f32::INFINITY; n],
            f_i: vec![0.0; n],
            p_i: vec![0.0; n],
            wmin,
            wmax,
            u_a: vec![f32::INFINITY; m],
            p_a: vec![0.0; m],
            open: vec![false; m],
            close: vec![false; m],
            h: vec![0.0; n],
            hyperpath: Vec::new(),
        }
    }

    pub fn set_weights(&mut self, wmin: Vec<f32>, wmax: Vec<f32>) {
        self.wmin = wmin;
        self.wmax = wmax;
    }

    pub fn set_potentials(&mut self, h: Vec<f32>) {
        for (i, val) in h.into_iter().enumerate() {
            if i < self.h.len() {
                self.h[i] = val;
            }
        }
    }

    pub fn run(&mut self, o_id: &str, d_id: &str) {
        let o_idx = *self.g.vid_to_idx.get(o_id).expect("Origin vertex not found");
        let d_idx = *self.g.vid_to_idx.get(d_id).expect("Destination vertex not found");

        let mut heap = BinaryHeap::new();
        let mut po_edges_indices = Vec::new();

        self.u_i[d_idx] = 0.0;
        self.p_i[o_idx] = 1.0;

        let mut j_idx = d_idx;

        // backward pass
        loop {
            {
                let j = &self.g.vertices[j_idx];
                for &a_idx in &j.in_edges {
                    let i_idx = self.g.edges[a_idx].from_vertex;
                    let current_j_idx = j.idx;

                    let temp = self.u_i[current_j_idx] + self.wmin[a_idx] + self.h[i_idx];
                    if self.u_a[a_idx] > temp {
                        self.u_a[a_idx] = temp;
                        if !self.close[a_idx] {
                            if !self.open[a_idx] {
                                self.open[a_idx] = true;
                                heap.push(State { idx: a_idx, val: temp });
                            } else {
                                heap.push(State { idx: a_idx, val: temp });
                            }
                        }
                    }
                }
            }

            if heap.is_empty() {
                break;
            }

            let State { idx: a_idx, .. } = heap.pop().unwrap();
            if self.close[a_idx] {
                continue;
            }

            self.open[a_idx] = false;
            self.close[a_idx] = true;

            let i_idx = self.g.edges[a_idx].from_vertex;
            let j_idx_curr = self.g.edges[a_idx].to_vertex;

            let w_max = self.wmax[a_idx];
            let w_min = self.wmin[a_idx];

            if self.u_i[i_idx] >= self.u_i[j_idx_curr] + w_min {
                let f_a = if w_max == w_min { 9999999999.0 } else { 1.0 / (w_max - w_min) };
                let p_a_val = f_a / (self.f_i[i_idx] + f_a);

                if self.f_i[i_idx] == 0.0 {
                    self.u_i[i_idx] = self.u_i[j_idx_curr] + w_max;
                } else {
                    let new_val = (1.0 - p_a_val) * self.u_i[i_idx] + p_a_val * (self.u_i[j_idx_curr] + w_min);
                    if self.u_i[i_idx] > new_val {
                        self.u_i[i_idx] = new_val;
                    }
                }

                self.f_i[i_idx] += f_a;
                po_edges_indices.push(a_idx);
            }

            if self.u_i[j_idx_curr] + w_min + self.h[i_idx] > self.u_i[o_idx] {
                break;
            }
            j_idx = i_idx;
        }

        // forward pass
        po_edges_indices.sort_by(|&a, &b| {
            let val_a = self.u_i[self.g.edges[a].to_vertex] + self.wmin[a];
            let val_b = self.u_i[self.g.edges[b].to_vertex] + self.wmin[b];
            val_b.partial_cmp(&val_a).unwrap_or(Ordering::Equal)
        });

        for &a_idx in &po_edges_indices {
            let i_idx = self.g.edges[a_idx].from_vertex;
            let j_idx = self.g.edges[a_idx].to_vertex;
            let w_max = self.wmax[a_idx];
            let w_min = self.wmin[a_idx];
            let f_a = if w_max == w_min { 9999999999.0 } else { 1.0 / (w_max - w_min) };
            let p_a_val = f_a / self.f_i[i_idx];
            self.p_a[a_idx] = p_a_val * self.p_i[i_idx];
            self.p_i[j_idx] += self.p_a[a_idx];
        }

        for &a_idx in &po_edges_indices {
            if self.p_a[a_idx] != 0.0 {
                self.hyperpath.push((self.g.edges[a_idx].id.clone(), self.p_a[a_idx]));
            }
        }
    }

    pub fn get_hyperpath(&self) -> Vec<(String, f32)> {
        self.hyperpath.clone()
    }

    pub fn recover(&mut self) {
        let n = self.g.get_vertex_number();
        let m = self.g.get_edge_number();

        for i in 0..n {
            self.u_i[i] = f32::INFINITY;
            self.f_i[i] = 0.0;
            self.p_i[i] = 0.0;
        }

        for i in 0..m {
            self.u_a[i] = f32::INFINITY;
            self.p_a[i] = 0.0;
            self.open[i] = false;
            self.close[i] = false;
        }

        self.hyperpath.clear();
    }
}

// ===========================================================================
// Dijkstra algorithm
// ===========================================================================

pub struct Dijkstra<'a> {
    g: &'a Graph,
    u: Vec<f32>,
    pre_idx: Vec<i32>,
    open: Vec<bool>,
    close: Vec<bool>,
    weights: Vec<f32>,
}

impl<'a> Dijkstra<'a> {
    pub fn new(g: &'a Graph) -> Self {
        let n = g.get_vertex_number();
        let m = g.get_edge_number();
        Self {
            g,
            u: vec![f32::INFINITY; n],
            pre_idx: vec![-1; n],
            open: vec![false; n],
            close: vec![false; n],
            weights: vec![0.0; m],
        }
    }

    pub fn set_weights(&mut self, weights: Vec<f32>) {
        self.weights = weights;
    }

    pub fn run(&mut self, origin_id: &str) {
        let o_idx = *self.g.vid_to_idx.get(origin_id).expect("Origin vertex not found");

        let mut heap = BinaryHeap::new();

        self.u[o_idx] = 0.0;
        heap.push(VertexState { idx: o_idx, dist: 0.0 });

        while let Some(VertexState { idx: vis_idx, .. }) = heap.pop() {
            if self.close[vis_idx] {
                continue;
            }
            self.close[vis_idx] = true;
            self.open[vis_idx] = false;

            let vis = &self.g.vertices[vis_idx];
            for &edge_idx in &vis.out_edges {
                let edge = &self.g.edges[edge_idx];
                let v_idx = edge.to_vertex;

                if !self.close[v_idx] {
                    let dist = self.u[vis_idx] + self.weights[edge_idx];
                    if dist < self.u[v_idx] {
                        self.u[v_idx] = dist;
                        if self.open[v_idx] {
                            heap.push(VertexState { idx: v_idx, dist });
                        } else {
                            heap.push(VertexState { idx: v_idx, dist });
                            self.open[v_idx] = true;
                        }
                        self.pre_idx[v_idx] = vis_idx as i32;
                    }
                }
            }
        }
    }

    pub fn get_potentials(&self) -> Vec<f32> {
        self.u.clone()
    }

    pub fn get_pre_idx(&self) -> Vec<i32> {
        self.pre_idx.clone()
    }

    pub fn get_path(&self, o_id: &str, d_id: &str) -> Vec<String> {
        let o_idx = *self.g.vid_to_idx.get(o_id).expect("Origin not found");
        let d_idx = *self.g.vid_to_idx.get(d_id).expect("Destination not found");

        let mut path = Vec::new();
        let mut cur = d_idx;
        while cur != o_idx {
            path.push(self.g.vertices[cur].id.clone());
            if self.pre_idx[cur] < 0 {
                break;
            }
            cur = self.pre_idx[cur] as usize;
        }
        path.push(self.g.vertices[o_idx].id.clone());
        path.reverse();
        path
    }

    pub fn recover(&mut self) {
        let n = self.g.get_vertex_number();
        for i in 0..n {
            self.u[i] = f32::INFINITY;
            self.pre_idx[i] = -1;
            self.open[i] = false;
            self.close[i] = false;
        }
    }
}

// ===========================================================================
// Bell 2009 sample network
// ===========================================================================

/// Bell 2009 sample network: 224 edges with real weights.
/// Format: (edge_id, from_vertex, to_vertex, w_min, w_max)
pub fn get_bell2009() -> Vec<(i32, i32, i32, f32, f32)> {
    vec![
        (1, 1, 2, 1.5, 2.0313),
        (2, 2, 3, 1.361, 2.0637),
        (3, 3, 4, 1.7729, 1.9446),
        (4, 4, 5, 1.8187, 2.4804),
        (5, 5, 6, 1.8722, 2.7979),
        (6, 6, 7, 1.582, 2.5567),
        (7, 7, 8, 1.5706, 1.9116),
        (8, 9, 10, 1.5597, 2.353),
        (9, 10, 11, 1.4173, 2.2853),
        (10, 11, 12, 1.2747, 2.1027),
        (11, 12, 13, 1.1977, 1.542),
        (12, 13, 14, 1.218, 2.0247),
        (13, 14, 15, 1.6626, 2.4919),
        (14, 15, 16, 1.966, 2.7127),
        (15, 17, 18, 1.3257, 2.1047),
        (16, 18, 19, 1.9818, 2.6311),
        (17, 19, 20, 1.3743, 2.1616),
        (18, 20, 21, 1.2728, 1.7491),
        (19, 21, 22, 1.9925, 2.3805),
        (20, 22, 23, 1.3273, 1.8476),
        (21, 23, 24, 1.6411, 2.2884),
        (22, 25, 26, 1.0008, 1.5225),
        (23, 26, 27, 1.0929, 1.6026),
        (24, 27, 28, 1.2745, 1.4252),
        (25, 28, 29, 1.2705, 1.4785),
        (26, 29, 30, 1.0575, 1.7635),
        (27, 30, 31, 1.1811, 1.2741),
        (28, 31, 32, 1.4555, 1.5995),
        (29, 33, 34, 1.4984, 2.2421),
        (30, 34, 35, 1.1762, 1.9719),
        (31, 35, 36, 1.5859, 1.8582),
        (32, 36, 37, 1.9953, 2.377),
        (33, 37, 38, 1.01, 1.0833),
        (34, 38, 39, 1.4532, 1.6382),
        (35, 39, 40, 1.8387, 2.3464),
        (36, 41, 42, 1.2054, 1.6944),
        (37, 42, 43, 1.274, 1.5237),
        (38, 43, 44, 1.6977, 2.3414),
        (39, 44, 45, 1.4136, 1.6979),
        (40, 45, 46, 1.0893, 1.454),
        (41, 46, 47, 1.1894, 1.9771),
        (42, 47, 48, 1.5936, 1.7226),
        (43, 49, 50, 1.78, 2.1573),
        (44, 50, 51, 1.6612, 2.4345),
        (45, 51, 52, 1.0437, 1.5044),
        (46, 52, 53, 1.6744, 2.3314),
        (47, 53, 54, 1.7681, 2.4454),
        (48, 54, 55, 1.0768, 2.0748),
        (49, 55, 56, 1.7852, 2.4412),
        (50, 57, 58, 1.1098, 1.3495),
        (51, 58, 59, 1.2464, 1.8267),
        (52, 59, 60, 1.5763, 2.136),
        (53, 60, 61, 1.6065, 1.9022),
        (54, 61, 62, 1.3059, 2.1876),
        (55, 62, 63, 1.112, 2.0487),
        (56, 63, 64, 1.2408, 2.2348),
        (57, 1, 9, 1.8413, 2.1603),
        (58, 2, 10, 1.317, 1.9583),
        (59, 3, 11, 1.5935, 2.1605),
        (60, 4, 12, 1.5603, 2.2543),
        (61, 5, 13, 1.173, 1.9003),
        (62, 6, 14, 1.0704, 1.8247),
        (63, 7, 15, 1.8659, 2.2029),
        (64, 8, 16, 1.8146, 2.6296),
        (65, 9, 17, 1.1564, 1.3487),
        (66, 10, 18, 1.403, 2.2217),
        (67, 11, 19, 1.9787, 2.775),
        (68, 12, 20, 1.2667, 1.5074),
        (69, 13, 21, 1.052, 1.2757),
        (70, 14, 22, 1.9117, 1.9537),
        (71, 15, 23, 1.0958, 1.4765),
        (72, 16, 24, 1.1689, 1.7812),
        (73, 17, 25, 1.9717, 2.8754),
        (74, 18, 26, 1.6148, 2.1688),
        (75, 19, 27, 1.0486, 1.2219),
        (76, 20, 28, 1.5807, 2.0024),
        (77, 21, 29, 1.3387, 1.52),
        (78, 22, 30, 1.427, 2.2043),
        (79, 23, 31, 1.2009, 1.4352),
        (80, 24, 32, 2.0, 2.1637),
        (81, 25, 33, 1.6582, 2.3882),
        (82, 26, 34, 1.4465, 2.3115),
        (83, 27, 35, 1.6267, 2.1137),
        (84, 28, 36, 1.8063, 2.3606),
        (85, 29, 37, 1.6019, 1.9686),
        (86, 30, 38, 1.0544, 1.2514),
        (87, 31, 39, 1.6007, 1.7697),
        (88, 32, 40, 1.5166, 2.3266),
        (89, 33, 41, 1.4043, 1.4396),
        (90, 34, 42, 1.827, 2.7693),
        (91, 35, 43, 1.4006, 2.2479),
        (92, 36, 44, 1.8538, 2.8415),
        (93, 37, 45, 1.5716, 1.9223),
        (94, 38, 46, 1.2603, 1.9406),
        (95, 39, 47, 1.4228, 2.4165),
        (96, 40, 48, 1.6276, 2.0199),
        (97, 41, 49, 1.9032, 2.6109),
        (98, 42, 50, 1.3071, 1.3944),
        (99, 43, 51, 1.6098, 1.7331),
        (100, 44, 52, 1.4277, 1.4684),
        (101, 45, 53, 1.8261, 2.7744),
        (102, 46, 54, 1.7268, 2.4115),
        (103, 47, 55, 1.1905, 1.6865),
        (104, 48, 56, 1.1297, 1.3214),
        (105, 49, 57, 1.0407, 1.638),
        (106, 50, 58, 1.2472, 1.8959),
        (107, 51, 59, 1.7329, 2.1132),
        (108, 52, 60, 1.2138, 2.1528),
        (109, 53, 61, 1.4803, 2.1346),
        (110, 54, 62, 1.5394, 2.2237),
        (111, 55, 63, 1.5097, 2.3887),
        (112, 56, 64, 1.8949, 2.7589),
        // Reverse direction edges
        (113, 2, 1, 1.5, 2.0313),
        (114, 3, 2, 1.361, 2.0637),
        (115, 4, 3, 1.7729, 1.9446),
        (116, 5, 4, 1.8187, 2.4804),
        (117, 6, 5, 1.8722, 2.7979),
        (118, 7, 6, 1.582, 2.5567),
        (119, 8, 7, 1.5706, 1.9116),
        (120, 10, 9, 1.5597, 2.353),
        (121, 11, 10, 1.4173, 2.2853),
        (122, 12, 11, 1.2747, 2.1027),
        (123, 13, 12, 1.1977, 1.542),
        (124, 14, 13, 1.218, 2.0247),
        (125, 15, 14, 1.6626, 2.4919),
        (126, 16, 15, 1.966, 2.7127),
        (127, 18, 17, 1.3257, 2.1047),
        (128, 19, 18, 1.9818, 2.6311),
        (129, 20, 19, 1.3743, 2.1616),
        (130, 21, 20, 1.2728, 1.7491),
        (131, 22, 21, 1.9925, 2.3805),
        (132, 23, 22, 1.3273, 1.8476),
        (133, 24, 23, 1.6411, 2.2884),
        (134, 26, 25, 1.0008, 1.5225),
        (135, 27, 26, 1.0929, 1.6026),
        (136, 28, 27, 1.2745, 1.4252),
        (137, 29, 28, 1.2705, 1.4785),
        (138, 30, 29, 1.0575, 1.7635),
        (139, 31, 30, 1.1811, 1.2741),
        (140, 32, 31, 1.4555, 1.5995),
        (141, 34, 33, 1.4984, 2.2421),
        (142, 35, 34, 1.1762, 1.9719),
        (143, 36, 35, 1.5859, 1.8582),
        (144, 37, 36, 1.9953, 2.377),
        (145, 38, 37, 1.01, 1.0833),
        (146, 39, 38, 1.4532, 1.6382),
        (147, 40, 39, 1.8387, 2.3464),
        (148, 42, 41, 1.2054, 1.6944),
        (149, 43, 42, 1.274, 1.5237),
        (150, 44, 43, 1.6977, 2.3414),
        (151, 45, 44, 1.4136, 1.6979),
        (152, 46, 45, 1.0893, 1.454),
        (153, 47, 46, 1.1894, 1.9771),
        (154, 48, 47, 1.5936, 1.7226),
        (155, 50, 49, 1.78, 2.1573),
        (156, 51, 50, 1.6612, 2.4345),
        (157, 52, 51, 1.0437, 1.5044),
        (158, 53, 52, 1.6744, 2.3314),
        (159, 54, 53, 1.7681, 2.4454),
        (160, 55, 54, 1.0768, 2.0748),
        (161, 56, 55, 1.7852, 2.4412),
        (162, 58, 57, 1.1098, 1.3495),
        (163, 59, 58, 1.2464, 1.8267),
        (164, 60, 59, 1.5763, 2.136),
        (165, 61, 60, 1.6065, 1.9022),
        (166, 62, 61, 1.3059, 2.1876),
        (167, 63, 62, 1.112, 2.0487),
        (168, 64, 63, 1.2408, 2.2348),
        // Cross-link reverse edges
        (169, 9, 1, 1.8413, 2.1603),
        (170, 10, 2, 1.317, 1.9583),
        (171, 11, 3, 1.5935, 2.1605),
        (172, 12, 4, 1.5603, 2.2543),
        (173, 13, 5, 1.173, 1.9003),
        (174, 14, 6, 1.0704, 1.8247),
        (175, 15, 7, 1.8659, 2.2029),
        (176, 16, 8, 1.8146, 2.6296),
        (177, 17, 9, 1.1564, 1.3487),
        (178, 18, 10, 1.403, 2.2217),
        (179, 19, 11, 1.9787, 2.775),
        (180, 20, 12, 1.2667, 1.5074),
        (181, 21, 13, 1.052, 1.2757),
        (182, 22, 14, 1.9117, 1.9537),
        (183, 23, 15, 1.0958, 1.4765),
        (184, 24, 16, 1.1689, 1.7812),
        (185, 25, 17, 1.9717, 2.8754),
        (186, 26, 18, 1.6148, 2.1688),
        (187, 27, 19, 1.0486, 1.2219),
        (188, 28, 20, 1.5807, 2.0024),
        (189, 29, 21, 1.3387, 1.52),
        (190, 30, 22, 1.427, 2.2043),
        (191, 31, 23, 1.2009, 1.4352),
        (192, 32, 24, 2.0, 2.1637),
        (193, 33, 25, 1.6582, 2.3882),
        (194, 34, 26, 1.4465, 2.3115),
        (195, 35, 27, 1.6267, 2.1137),
        (196, 36, 28, 1.8063, 2.3606),
        (197, 37, 29, 1.6019, 1.9686),
        (198, 38, 30, 1.0544, 1.2514),
        (199, 39, 31, 1.6007, 1.7697),
        (200, 40, 32, 1.5166, 2.3266),
        (201, 41, 33, 1.4043, 1.4396),
        (202, 42, 34, 1.827, 2.7693),
        (203, 43, 35, 1.4006, 2.2479),
        (204, 44, 36, 1.8538, 2.8415),
        (205, 45, 37, 1.5716, 1.9223),
        (206, 46, 38, 1.2603, 1.9406),
        (207, 47, 39, 1.4228, 2.4165),
        (208, 48, 40, 1.6276, 2.0199),
        (209, 49, 41, 1.9032, 2.6109),
        (210, 50, 42, 1.3071, 1.3944),
        (211, 51, 43, 1.6098, 1.7331),
        (212, 52, 44, 1.4277, 1.4684),
        (213, 53, 45, 1.8261, 2.7744),
        (214, 54, 46, 1.7268, 2.4115),
        (215, 55, 47, 1.1905, 1.6865),
        (216, 56, 48, 1.1297, 1.3214),
        (217, 57, 49, 1.0407, 1.638),
        (218, 58, 50, 1.2472, 1.8959),
        (219, 59, 51, 1.7329, 2.1132),
        (220, 60, 52, 1.2138, 2.1528),
        (221, 61, 53, 1.4803, 2.1346),
        (222, 62, 54, 1.5394, 2.2237),
        (223, 63, 55, 1.5097, 2.3887),
        (224, 64, 56, 1.8949, 2.7589),
    ]
}

// ===========================================================================
// PyO3 Python bindings
// ===========================================================================

/// Python-exposed Vertex type
#[pyclass(name = "Vertex")]
#[derive(Clone)]
struct PyVertex {
    #[pyo3(get, set)]
    id: String,
    #[pyo3(get)]
    idx: usize,
    in_edge_ids: Vec<String>,
    out_edge_ids: Vec<String>,
}

#[pymethods]
impl PyVertex {
    #[new]
    #[pyo3(signature = (id))]
    fn new(id: String) -> Self {
        Self {
            id,
            idx: 0,
            in_edge_ids: Vec::new(),
            out_edge_ids: Vec::new(),
        }
    }

    #[getter]
    fn in_cnt(&self) -> usize {
        self.in_edge_ids.len()
    }

    #[getter]
    fn out_cnt(&self) -> usize {
        self.out_edge_ids.len()
    }

    #[getter]
    fn in_edges(&self) -> Vec<String> {
        self.in_edge_ids.clone()
    }

    #[getter]
    fn out_edges(&self) -> Vec<String> {
        self.out_edge_ids.clone()
    }
}

/// Python-exposed Edge type
#[pyclass(name = "Edge")]
#[derive(Clone)]
struct PyEdge {
    #[pyo3(get, set)]
    id: String,
    #[pyo3(get)]
    idx: usize,
    from_id: String,
    to_id: String,
}

#[pymethods]
impl PyEdge {
    #[new]
    #[pyo3(signature = (id, from_id, to_id))]
    fn new(id: String, from_id: String, to_id: String) -> Self {
        Self {
            id,
            idx: 0,
            from_id,
            to_id,
        }
    }

    fn get_fv(&self) -> PyVertex {
        PyVertex::new(self.from_id.clone())
    }

    fn get_tv(&self) -> PyVertex {
        PyVertex::new(self.to_id.clone())
    }
}

/// Python-exposed Graph type.
///
/// Graph(n, m)
///
/// Create a graph. The n and m parameters are accepted for API compatibility
/// but not strictly needed since storage grows dynamically.
#[pyclass]
struct PyGraph {
    inner: Graph,
}

#[pymethods]
impl PyGraph {
    #[new]
    #[pyo3(signature = (n, m))]
    fn new(n: usize, m: usize) -> Self {
        let _ = (n, m); // accepted for API compat, not used
        Self {
            inner: Graph::new(),
        }
    }

    fn add_vertex(&mut self, id: &str) {
        self.inner.add_vertex(id);
    }

    #[pyo3(signature = (name, fv_name, tv_name))]
    fn add_edge(&mut self, name: &str, fv_name: &str, tv_name: &str) {
        self.inner.add_edge(name, fv_name, tv_name);
    }

    #[getter]
    fn vertex_num(&self) -> usize {
        self.inner.get_vertex_number()
    }

    #[getter]
    fn edge_num(&self) -> usize {
        self.inner.get_edge_number()
    }

    /// Get vertex by id string or index
    fn get_vertex(&self, key: &Bound<'_, PyAny>) -> PyResult<PyVertex> {
        if let Ok(id) = key.extract::<String>() {
            let idx = *self
                .inner
                .vid_to_idx
                .get(&id)
                .ok_or_else(|| pyo3::exceptions::PyRuntimeError::new_err(
                    format!("Vertex '{}' not found", id),
                ))?;
            let v = &self.inner.vertices[idx];
            Ok(PyVertex {
                id: v.id.clone(),
                idx: v.idx,
                in_edge_ids: v.in_edges.iter().map(|&ei| self.inner.edges[ei].id.clone()).collect(),
                out_edge_ids: v.out_edges.iter().map(|&ei| self.inner.edges[ei].id.clone()).collect(),
            })
        } else if let Ok(idx) = key.extract::<usize>() {
            let v = self
                .inner
                .vertices
                .get(idx)
                .ok_or_else(|| pyo3::exceptions::PyRuntimeError::new_err(
                    format!("Vertex index {} out of range", idx),
                ))?;
            Ok(PyVertex {
                id: v.id.clone(),
                idx: v.idx,
                in_edge_ids: v.in_edges.iter().map(|&ei| self.inner.edges[ei].id.clone()).collect(),
                out_edge_ids: v.out_edges.iter().map(|&ei| self.inner.edges[ei].id.clone()).collect(),
            })
        } else {
            Err(pyo3::exceptions::PyTypeError::new_err(
                "Expected str or int",
            ))
        }
    }

    /// Get edge by id string or index
    fn get_edge(&self, key: &Bound<'_, PyAny>) -> PyResult<PyEdge> {
        if let Ok(id) = key.extract::<String>() {
            let e = self
                .inner
                .eid_to_idx
                .get(&id)
                .map(|&idx| &self.inner.edges[idx])
                .ok_or_else(|| pyo3::exceptions::PyRuntimeError::new_err(
                    format!("Edge '{}' not found", id),
                ))?;
            Ok(PyEdge {
                id: e.id.clone(),
                idx: e.idx,
                from_id: self.inner.vertices[e.from_vertex].id.clone(),
                to_id: self.inner.vertices[e.to_vertex].id.clone(),
            })
        } else if let Ok(idx) = key.extract::<usize>() {
            let e = self
                .inner
                .edges
                .get(idx)
                .ok_or_else(|| pyo3::exceptions::PyRuntimeError::new_err(
                    format!("Edge index {} out of range", idx),
                ))?;
            Ok(PyEdge {
                id: e.id.clone(),
                idx: e.idx,
                from_id: self.inner.vertices[e.from_vertex].id.clone(),
                to_id: self.inner.vertices[e.to_vertex].id.clone(),
            })
        } else {
            Err(pyo3::exceptions::PyTypeError::new_err(
                "Expected str or int",
            ))
        }
    }

    /// Return a reversed copy of this graph (edges flipped direction)
    fn reverse(&self) -> Self {
        Self {
            inner: self.inner.make_reverse(),
        }
    }
}

/// Python-exposed Dijkstra algorithm
#[pyclass(name = "Dijkstra")]
struct PyDijkstra {
    graph: Py<PyGraph>,
    weights: Vec<f32>,
    potentials: Vec<f32>,
    pre_idx: Vec<i32>,
}

#[pymethods]
impl PyDijkstra {
    #[new]
    fn new(graph: Py<PyGraph>) -> Self {
        Self {
            graph,
            weights: Vec::new(),
            potentials: Vec::new(),
            pre_idx: Vec::new(),
        }
    }

    /// Set edge weights from a Python list of floats
    fn set_weights(&mut self, weights: Vec<f32>) {
        self.weights = weights;
    }

    /// Run Dijkstra from origin vertex
    fn run(&mut self, py: Python<'_>, origin_id: &str) {
        let g = self.graph.borrow(py);
        let mut dijk = Dijkstra::new(&g.inner);
        dijk.set_weights(self.weights.clone());
        dijk.run(origin_id);
        self.potentials = dijk.get_potentials();
        self.pre_idx = dijk.get_pre_idx();
    }

    /// Get the computed potentials as a list of floats
    fn get_potentials(&self) -> Vec<f32> {
        self.potentials.clone()
    }

    /// Get the shortest path from origin to destination as a list of vertex ids
    fn get_path(&self, py: Python<'_>, o_id: &str, d_id: &str) -> PyResult<Vec<String>> {
        let g = self.graph.borrow(py);
        let o_idx = *g.inner.vid_to_idx.get(o_id).ok_or_else(|| {
            pyo3::exceptions::PyRuntimeError::new_err(format!("Origin '{}' not found", o_id))
        })?;
        let d_idx = *g.inner.vid_to_idx.get(d_id).ok_or_else(|| {
            pyo3::exceptions::PyRuntimeError::new_err(format!("Destination '{}' not found", d_id))
        })?;

        let mut path = Vec::new();
        let mut cur = d_idx;
        while cur != o_idx {
            path.push(g.inner.vertices[cur].id.clone());
            if cur >= self.pre_idx.len() || self.pre_idx[cur] < 0 {
                break;
            }
            cur = self.pre_idx[cur] as usize;
        }
        path.push(g.inner.vertices[o_idx].id.clone());
        path.reverse();
        Ok(path)
    }

    /// Reset algorithm state
    fn recover(&mut self) {
        self.potentials.clear();
        self.pre_idx.clear();
    }
}

/// Python-exposed Ma2013 (Hyperpath) algorithm.
///
/// Dijkstra-Hyperstar algorithm from:
/// Ma, J., Fukuda, D. and Schmoecker, J.D. 2013
/// "Faster hyperpath generating algorithms for vehicle navigation"
/// Transportmetrica A: Transport Science, Vol. 9, 925–948.
#[pyclass(name = "Ma2013")]
struct PyMa2013 {
    graph: Py<PyGraph>,
    wmin: Vec<f32>,
    wmax: Vec<f32>,
    h: Vec<f32>,
    hyperpath_result: Vec<(String, f32)>,
}

#[pymethods]
impl PyMa2013 {
    #[new]
    fn new(graph: Py<PyGraph>) -> Self {
        Self {
            graph,
            wmin: Vec::new(),
            wmax: Vec::new(),
            h: Vec::new(),
            hyperpath_result: Vec::new(),
        }
    }

    /// Set weight arrays: w_min and w_max
    fn set_weights(&mut self, wmin: Vec<f32>, wmax: Vec<f32>) {
        self.wmin = wmin;
        self.wmax = wmax;
    }

    /// Set potential array (lower-bound heuristic from Dijkstra)
    fn set_potentials(&mut self, h: Vec<f32>) {
        self.h = h;
    }

    /// Run the hyperpath algorithm from origin to destination
    fn run(&mut self, py: Python<'_>, origin_id: &str, dest_id: &str) {
        let g = self.graph.borrow(py);
        let mut hp = Hyperpath::new(&g.inner, self.wmin.clone(), self.wmax.clone());
        hp.set_potentials(self.h.clone());
        hp.run(origin_id, dest_id);
        self.hyperpath_result = hp.get_hyperpath();
    }

    /// Get the hyperpath result as a list of (edge_id, probability) tuples.
    /// The first element is the edge id string, the second is the choice probability.
    #[getter]
    fn hyperpath(&self) -> Vec<(String, f32)> {
        self.hyperpath_result.clone()
    }

    /// Reset algorithm state
    fn recover(&mut self) {
        self.hyperpath_result.clear();
    }
}

/// Describe an edge array: returns [num_vertices, num_edges]
#[pyfunction]
fn describe(arr: &Bound<'_, PyAny>) -> PyResult<Vec<usize>> {
    let m = arr.len()?;
    let mut vertex_set = HashSet::new();
    for i in 0..m {
        let row = arr.get_item(i)?;
        let fid: String = row.get_item(1)?.extract()?;
        let tid: String = row.get_item(2)?.extract()?;
        vertex_set.insert(fid);
        vertex_set.insert(tid);
    }
    Ok(vec![vertex_set.len(), m])
}

/// Make a graph from an edge array.
///
/// arr is m×3 with (eid, fvid, tvid) at each row.
/// n and m are vertex/edge counts (n accepted for API compat).
#[pyfunction]
#[pyo3(signature = (arr, n, m))]
fn make_graph(arr: &Bound<'_, PyAny>, n: usize, m: usize) -> PyResult<PyGraph> {
    let _ = (n, m);
    let mut g = Graph::new();
    let len = arr.len()?;
    for i in 0..len {
        let row = arr.get_item(i)?;
        let eid: String = row.get_item(0)?.extract()?;
        let fid: String = row.get_item(1)?.extract()?;
        let tid: String = row.get_item(2)?.extract()?;
        g.add_edge(&eid, &fid, &tid);
    }
    Ok(PyGraph { inner: g })
}

/// The `dhs` Python module — drop-in replacement for the C++ Boost.Python extension.
#[pymodule]
fn dhs(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<PyVertex>()?;
    m.add_class::<PyEdge>()?;
    m.add_class::<PyGraph>()?;
    m.add_class::<PyDijkstra>()?;
    m.add_class::<PyMa2013>()?;
    m.add_function(wrap_pyfunction!(describe, m)?)?;
    m.add_function(wrap_pyfunction!(make_graph, m)?)?;
    Ok(())
}

// ===========================================================================
// Tests
// ===========================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_graph() {
        let mut g = Graph::new();
        g.add_edge("e1", "v1", "v2");
        g.add_edge("e2", "v2", "v3");

        let wmin = vec![1.0, 1.0];
        let wmax = vec![2.0, 2.0];
        let mut hp = Hyperpath::new(&g, wmin, wmax);
        hp.run("v1", "v3");

        let path = hp.get_hyperpath();
        assert!(!path.is_empty());
    }

    #[test]
    fn test_dijkstra_basic() {
        let mut g = Graph::new();
        g.add_edge("e1", "v1", "v2");
        g.add_edge("e2", "v2", "v3");

        let mut dijk = Dijkstra::new(&g);
        dijk.set_weights(vec![1.0, 2.0]);
        dijk.run("v1");

        let potentials = dijk.get_potentials();
        let v1_idx = *g.vid_to_idx.get("v1").unwrap();
        let v2_idx = *g.vid_to_idx.get("v2").unwrap();
        let v3_idx = *g.vid_to_idx.get("v3").unwrap();

        assert_eq!(potentials[v1_idx], 0.0);
        assert!((potentials[v2_idx] - 1.0).abs() < 1e-6);
        assert!((potentials[v3_idx] - 3.0).abs() < 1e-6);
    }

    #[test]
    fn test_dijkstra_diamond_graph() {
        let mut g = Graph::new();
        g.add_edge("e1", "v1", "v2");
        g.add_edge("e2", "v1", "v3");
        g.add_edge("e3", "v2", "v4");
        g.add_edge("e4", "v3", "v4");

        let mut dijk = Dijkstra::new(&g);
        dijk.set_weights(vec![1.0, 3.0, 2.0, 1.0]);
        dijk.run("v1");

        let potentials = dijk.get_potentials();
        let v4_idx = *g.vid_to_idx.get("v4").unwrap();
        assert!((potentials[v4_idx] - 3.0).abs() < 1e-6);
    }

    #[test]
    fn test_bell2009_dijkstra() {
        let data = get_bell2009();
        assert_eq!(data.len(), 224);

        let mut g = Graph::new();
        let mut w_min = Vec::new();

        for (eid, fv, tv, wm, _wx) in &data {
            g.add_edge(&format!("e{}", eid), &format!("{}", fv), &format!("{}", tv));
            w_min.push(*wm);
        }

        assert_eq!(g.get_vertex_number(), 64);
        assert_eq!(g.get_edge_number(), 224);

        let mut dijk = Dijkstra::new(&g);
        dijk.set_weights(w_min);
        dijk.run("1");

        let potentials = dijk.get_potentials();
        for (i, &p) in potentials.iter().enumerate() {
            assert!(p.is_finite(), "Vertex {} unreachable", g.vertices[i].id);
        }
        let o_idx = *g.vid_to_idx.get("1").unwrap();
        assert!((potentials[o_idx] - 0.0).abs() < 1e-6);
    }

    #[test]
    fn test_bell2009_hyperpath() {
        let data = get_bell2009();

        let mut g = Graph::new();
        let mut w_min = Vec::new();
        let mut w_max = Vec::new();

        for (eid, fv, tv, wm, wx) in &data {
            g.add_edge(&format!("e{}", eid), &format!("{}", fv), &format!("{}", tv));
            w_min.push(*wm);
            w_max.push(*wx);
        }

        let mut dijk = Dijkstra::new(&g);
        dijk.set_weights(w_min.clone());
        dijk.run("1");
        let h = dijk.get_potentials();

        let mut hp = Hyperpath::new(&g, w_min, w_max);
        hp.set_potentials(h);
        hp.run("1", "64");

        let results = hp.get_hyperpath();
        assert!(!results.is_empty());

        for (edge_id, prob) in &results {
            assert!(*prob > 0.0, "Edge {} has non-positive probability", edge_id);
            assert!(*prob <= 1.0 + 1e-5, "Edge {} has probability > 1.0", edge_id);
            assert!(g.eid_to_idx.contains_key(edge_id), "Edge {} not in graph", edge_id);
        }
        assert!(results.len() >= 2);
    }

    #[test]
    fn test_bell2009_multiple_od_pairs() {
        let data = get_bell2009();

        let mut g = Graph::new();
        let mut w_min = Vec::new();
        let mut w_max = Vec::new();

        for (eid, fv, tv, wm, wx) in &data {
            g.add_edge(&format!("e{}", eid), &format!("{}", fv), &format!("{}", tv));
            w_min.push(*wm);
            w_max.push(*wx);
        }

        let od_pairs = [("1", "8"), ("9", "16"), ("17", "32"), ("33", "48")];

        for (origin, dest) in &od_pairs {
            let mut dijk = Dijkstra::new(&g);
            dijk.set_weights(w_min.clone());
            dijk.run(origin);
            let h = dijk.get_potentials();

            let mut hp = Hyperpath::new(&g, w_min.clone(), w_max.clone());
            hp.set_potentials(h);
            hp.run(origin, dest);

            let results = hp.get_hyperpath();
            assert!(!results.is_empty(), "Hyperpath {} -> {} empty", origin, dest);
            for (eid, prob) in &results {
                assert!(*prob > 0.0 && *prob <= 1.0 + 1e-5, "Bad prob {} for {}", prob, eid);
            }
        }
    }

    #[test]
    fn test_graph_reverse() {
        let mut g = Graph::new();
        g.add_edge("e1", "v1", "v2");
        g.add_edge("e2", "v2", "v3");

        let gr = g.make_reverse();
        assert_eq!(gr.get_edge_number(), 2);
        assert_eq!(gr.edges[0].from_vertex, *gr.vid_to_idx.get("v2").unwrap());
        assert_eq!(gr.edges[0].to_vertex, *gr.vid_to_idx.get("v1").unwrap());
    }

    #[test]
    fn test_dijkstra_get_path() {
        let mut g = Graph::new();
        g.add_edge("e1", "v1", "v2");
        g.add_edge("e2", "v2", "v3");
        g.add_edge("e3", "v1", "v3");

        let mut dijk = Dijkstra::new(&g);
        dijk.set_weights(vec![1.0, 1.0, 5.0]);
        dijk.run("v1");

        let path = dijk.get_path("v1", "v3");
        assert_eq!(path, vec!["v1", "v2", "v3"]);
    }
}
