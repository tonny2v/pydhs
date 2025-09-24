import numpy as np

import pydhs


def test_describe_counts_match_sample():
    data = np.array(pydhs.sample.get_bell2009(), dtype=object)
    edges = data[:, :3].astype(int).astype(str)
    edge_rows = edges.tolist()

    n, m = pydhs.describe(edge_rows)

    expected_vertices = {row[1] for row in edge_rows}
    expected_vertices.update(row[2] for row in edge_rows)

    assert int(n) == len(expected_vertices)
    assert int(m) == edges.shape[0]

    graph = pydhs.make_graph(edge_rows, int(n), int(m))
    assert graph.vertex_num == int(n)
    assert graph.edge_num == int(m)


def test_hyperpath_produces_probabilities():
    data = np.array(pydhs.sample.get_bell2009(), dtype=object)
    edges = data[:, :3].astype(int).astype(str)
    edge_rows = edges.tolist()
    w_min = data[:, 3].astype(float)
    w_max = data[:, 4].astype(float)

    n, m = map(int, pydhs.describe(edge_rows))
    graph = pydhs.make_graph(edge_rows, n, m)

    alg = pydhs.Ma2013(graph)
    alg.set_weights(w_min, w_max)
    alg.set_potentials(np.zeros(m))
    alg.run("1", "37")

    hyperpath = list(alg.hyperpath)
    assert hyperpath, "hyperpath should not be empty"
    for edge_id, probability in hyperpath:
        assert isinstance(edge_id, str)
        assert 0.0 <= probability <= 1.0
