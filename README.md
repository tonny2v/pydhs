# pydhs for static hyperpath calculation
Note
----
- This package is created by Jiangshan(Tonny) Ma in his Ph.D. work in Tokyo Institute of Technology
- If it happens to be useful to you, please cite my work [Faster hyperpath generating algorithms for vehicle navigation](https://www.tandfonline.com/doi/abs/10.1080/18128602.2012.719165) or my ISTTT paper [A Hyperpath-based Network Generalized Extreme-value Model for Route Choice under Uncertainties](https://www.sciencedirect.com/science/article/pii/S235214651500071X)


```latex
@article{doi:10.1080/18128602.2012.719165,
author = {Jiangshan Ma and Daisuke Fukuda and Jan-Dirk SchmÃ¶cker},
title = {Faster hyperpath generating algorithms for vehicle navigation},
journal = {Transportmetrica A: Transport Science},
volume = {9},
number = {10},
pages = {925-948},
year  = {2013},
publisher = {Taylor & Francis},
doi = {10.1080/18128602.2012.719165},

URL = { 
        https://doi.org/10.1080/18128602.2012.719165
    
},
eprint = { 
        https://doi.org/10.1080/18128602.2012.719165
    
}

}

```





Install
----
You may find the package in Pypi repository and install via pip install pydhs, however, only python2.7 under Mac OS X have been tested. You have to revise the install scripts "setup.py" according to your own OS environment.

docker pull tonny2v/pydhs

Test run
----
docker run -it tonny2v/pydhs python -c "import pydhs;import numpy as np;arr = np.array(pydhs.sample.get_bell2009());sarr =arr[:,:3].astype('int').astype('str');w_min, w_max = arr[:,-2], arr[:,-1];n, m = pydhs.describe(sarr);g = pydhs.make_graph(sarr, n, m);alg = pydhs.Ma2013(g);h = np.zeros(m);alg.set_weights(w_min, w_max);alg.set_potentials(h);alg.run('1','37');print(alg.hyperpath);"

Tutorial
----
```
import numpy as np
import pydhs

# the graph is a string array
arr = np.array(pydhs.sample.get_bell2009())
sarr =arr[:,:3].astype('int').astype('str')

# the last 2 columns are minimum and maximum link weights
w_min, w_max = arr[:,-2], arr[:,-1]

# calculate the number of nodes and links
n, m = pydhs.describe(sarr)

# build the graph topology
g = pydhs.make_graph(sarr, n, m)

# call DHS algorithm in Ma et al. 2013 (http://www.tandfonline.com/doi/abs/10.1080/18128602.2012.719165)
alg = pydhs.Ma2013(g)

# use no node potentials here
h = np.zeros(m)

# set weights and node potentials
alg.set_weights(w_min, w_max)
alg.set_potentials(h)

# search hyperpath from node 1 to 37
alg.run('1','37')

# hyperpath results in terms of link ID and choice possibility
print '------------------------------------'
print 'eid\tvid pair\tpossibility'
print '------------------------------------'
for i in alg.hyperpath:
    edge = g.get_edge(i[0])
    eid, p = i
    print eid, '\t', edge.get_fv().id,'-->', edge.get_tv().id, '\t', round(p, 2)
print '------------------------------------'
```

Contact
----
If you have any questions, please contact tonny.achilles@gmail.com
