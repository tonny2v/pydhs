#ifndef RADIXHEAP_H
#define RADIXHEAP_H
#include "heap.h"

//#define RADIXHEAP_DEBUG
/*
 Radix Heaps
 -----------
 
 In general, Dijkstra's algorithm works on directed graphs with non-negative
 real-valued edge costs.  Under this criteria, the most efficient
 implementations of Dijkstra's algorithm, using a Fibonacci heap or similar
 priority queue, achieves a time complexity of O(m + n log n) where n is the
 number of vertices in the graph and m is the number of edges.  This is the
 best-achievable time complexity for any implementation of Dijkstra's algorithm
 based on the comparison and addition of real-valued edge costs.  However, if
 edge costs are limited to moderately sized integers, then the efficiency of
 Dijkstra's algorithm can be improved by using integer-based priority queues.
 The radix heap implementation of Dijkstra's algorithm provides better
 efficiency for many shortest path problems that can be accurately represented
 using only moderately sized integer edge costs.
 
 The radix heap, invented by Ahuja, Melhorn, Orlin, and Tarjan, assumes that all
 edge costs in the graph are integers bounded above by C.  The result is that
 Dijkstra's algorithm can be implemented with a time complexity of
 O(m + n log C).  Furthermore, the radix heap is significantly simpler to
 implement compared to the Fibonacci heap and similar data structures.  The
 radix heap achieves this time complexity by taking advantage of the property
 that shortest path distances fall into a finite range during the computation
 shortest paths by Dijkstra's algorithm.
 
 Recall that Dijkstra's algorithm computes shortest path distances d[v] from
 some source vertex s to all other vertices v.  Initially d[s] = 0.  As vertices
 are explored, they are assigned a tentative value for d[v], which is eventually
 reduced to the final shortest path distance.  Each explored vertex v is
 inserted into the frontier set F, using an insert() operation, and held there
 until the value of d[v] becomes minimum among vertices in F.  For the minimum
 vertex u among those in F, it is known that the value of d[u] cannot improved
 further and is therefore the final shortest path distance.  This minimum vertex
 is removed from F using a delete-min() operation, and then placed permanently
 in the solution set S.  Further vertices v are then explored through outgoing
 edges of u.  Any explored vertex v that is not in S will be placed in F, using
 an insert() operation, and be assigned a tentative distance value d[v], unless
 it is already being held there.  If an explored vertex is already held in F,
 then its tentative distance value d[v] may be improved by a decrease-key()
 operation.  Dijkstra's algorithm repeats this delete-min() process until all
 vertices have been moved to S, with their corresponding shortest path distance
 assigned in d[v].
 
 The radix heap relies on the following properties of Dijkstra's algorithm:
 
 1.  The value of d[v] is limited to the range [0..nC].
 
 2.  The value of d[v] for any vertex v in F lies in the range [d[u]..d[u]+C],
	where u is the minimum vertex most recently removed from F.
 
 Property 1 is seen by noting that no path can contain more than n edges, each
 of which has cost no larger than C.  Property 2 arises because any value d[v],
 for v in F, extends from some d[w] for w in S.  Thus, with d[w]<=d[u] and edge
 cost at most C, the upper bound on any d[v] is d[u]+C, and the lower bound is
 d[u] (the minimum distance in F).
 
 Let d[u] be denoted as dmin for this description.  At any time during
 Dijkstra's algorithm, the range [dmin..dmin+C] consists of C+1 distinct values.
 The radix heap divides the C+1 values in this range into B buckets, where
 B = log2(C+1) + 2  (rounded up to the nearest integer).  Buckets are numbered
 from 1 to B.  Each bucket i has an associated size size(i), which is the number
 of distinct values it covers.  The first bucket covers only the minimum value
 dmin.  The second bucket covers the next value, and remaining buckets cover
 twice the number of values as the bucket before them, except for the last
 bucket which is allowed to cover nC+1 distinct values (a practically unlimited
 number).  This defines size(i) as follows:
 
 size(1) = 1
 size(2) = 1
 size(i) = 2*size(i-1)  (3 <= i <= B-1)
 size(B) = nC+1
 
 Equivalently:
 size(1) = 1
 size(i) = 2^(i-2)  (2 <= i <= B-1)
 size(B) = nC+1
 
 Conceptually, the interval [dmin..dmin+C] is partitioned, with each bucket i
 covering a range of values, denoted as range(i), according to size(i).  For
 example:
 
 i    size(i)   range(i)
 
 1    1         dmin
 2    1         dmin+1
 3    2         dmin+2,...,dmin+3
 4    4         dmin+4,...,dmin+7
 5    8         dmin+8,...,dmin+15
 6    16        dmin+16,...,dmin+31
 7    32        dmin+32,...,dmin+63
 etc...
 
 The range of values covered by a bucket is managed by specifying an upper bound
 u[i] for each bucket i.  In general, bucket i can hold values in the range
 [u[i-1]+1,...,u[i]]; assuming the convention u[0]=dmin-1.  The vertices in F
 are stored in these buckets according to their distance value d[v], with vertex
 v in bucket i if d[v] lies in range(i).  As the computation proceeds, the upper
 bounds u[i] are updated, altering the range of buckets.  We see later that the
 value of upper bounds u[i] only increases as the computation proceeds.  The
 inequality |range(i)|<=size(i) is always maintained.
 
 The buckets in the radix heap are implemented using doubly-linked lists.  This
 is to allow efficient (constant time) removal of vertices from lists.
 Initially, when dmin = 0, the upper bounds of buckets are assigned as
 u[i] = 2^(i-1)-1 for 1<=i<=B-1 and u(B)=nC+1.  Dijkstra's algorithm uses the
 radix heap through the following three operations:
 
 - delete-min() for deleting and returns the minimum vertex from F.
 - insert(v,k) for inserting a vertex v into the F with a key k corresponding
	to the value of d[v].
 - decrease-key(v,k) for decreasing the key of a vertex in F to a new value
	k corresponding to value of d[v].
 
 The insert(v,k) operation is implemented as follows:  First, visit buckets i in
 decreasing order, starting with i = B, until locating the largest i for which
 u[i]<k.  Then insert v into bucket i+1.
 
 The decrease-key(v,k) operation is similar to insert().  First, remove $v$ from
 its current bucket, denoted as bucket j.  Then reinsert vertex v as for
 insert(), but starting with i=j-1.
 
 The most complicated operation is delete-min() since this adjusts the range of
 buckets.  If bucket 1 is non-empty, then remove and return any node in bucket
 1 as the minimum, since bucket 1 only holds vertices v for which d[v]=dmin.
 Otherwise, locate the first non-empty bucket j by visiting buckets in
 increasing order of index.  Then locate the vertex v with minimum key value by
 scanning all vertices in bucket j.  Remove v from bucket j and save v as the
 result to be returned.  Next, adjust the range of buckets 1 to j-1 by assigning
 u(0)=d[v]-1, u(1)=d[v], and u(i)=min(u(i-1)+size(i),u(j)) for i from 2 to j-1.
 Finally, remove all vertices from bucket j, redistributing them as in
 decrease-key(), and return v as the minimum.  Note that, the assigned ranges of
 buckets 1 to j-1 accommodate any vertex that was previously in bucket j.  As a
 result all vertices move to a bucket of strictly smaller index.
 
 Each insert or decrease-key operation spends O(1) time plus time attributed to
 the movement of vertices to lower indexed buckets.  With O(log C) buckets, each
 vertex makes at most O(log C) movements during its lifetime.  With n vertices
 inserted, vertex movement accounts for at worst O(n log C) time.  Thus, the
 total time for m insert and n decrease-key operations during Dijkstra's
 algorithm is n x O(1) + m x O(1) + O(n log C) = O(m + n log C).
 
 Each delete-min operation spends at most O(log C) time when locating a
 non-empty bucket, plus time attributed to the movement of vertices to lower
 indexed buckets by delete-min operations.  Again the total time for such vertex
 movement is at worst O(log C) per vertex.  Thus, the time for n delete-min
 operations during a run of Dijkstra's algorithm is O(n log C).  Combined with
 the time for insert and decrease-key operations, Dijkstra's algorithm has a
 total running time of O(m + n log C) when using a radix heap.
 
 Radix Heap Implementation Details
 ---------------------------------
 
 A C++ implementation of the radix heap is provided in the source files
 radixheap.cpp and radixheap.h.  The RadixHeap class defined in radixheap.h is
 derived from the abstract base class Heap defined in heap.h.  The public
 methods deleteMin(), insert() and decreaseKey() provide the respective heap
 operations.  Additionally, nItems() returns the number of items in the heap.
 The method nComps(), which returns the number of comparison operations
 performed, is not implemented in the radix heap.  The method dump() prints out
 a text representation of the heap.  The radix heap constructor RadixHeap()
 takes a parameter n specifying the maximum number of items the radix heap is to
 hold.  The private constant MaxKey defines the maximum integer size that the
 heap is to work with.
 
 The RadixHeap class itself includes the following private member variables:
 nodes     - an array of pointers to radix heap nodes.
 bucketHeaders - an array of radix heap bucket 'header' nodes.  A radix heap
 bucket is implemented as a circularly doubly-linked list
 containing a header-node that points to the first and last
 nodes of the list.
 u         - an array where entry u[i] holds the upper-bound of bucket i.
 nBuckets  - the number of buckets.
 dMin      - the minimum distance (key of the most recently removed
 minimum node).
 itemCount - the number of items in the heap.
 compCount - number of key comparisons performed (not currently implemented)
 Several private member functions are used for manipulating the buckets of the
 radix heap.  These are described later.
 
 The nodes of the radix heap are provided by the class RadixHeapNode.  A
 RadixHeapNode includes the following member variables:
 item       - the number of the vertex that the node represents.
 key        - a key corresponding to the distance of the vertex.
 bucket     - the index of the bucket that the node currently belongs to.
 next, prev - pointers to the next and previous nodes in the bucket.
 
 As seen in radixheap.cpp, the RadixHeap() constructor performs the necessary
 allocation and initialisation when a new RadixHeap object is created.  The
 supplied parameter n, along with the value of maxKey determines the number of
 buckets nBuckets in the heap.  Each bucket in the radix heap is represented by
 allocating an array of header-nodes.  Each header-node corresponds to a
 separate circular doubly-linked list of nodes.  A header-node is just a
 place-holder in the list.  The actual nodes of a bucket are accessed by
 following the next and prev pointers from the header node. Initially, all lists
 are empty, with the next and prev pointers of the header node pointing to
 itself.  This kind of circular list implementation offers better efficiency
 since there is no need to check for the occurrence of empty-list null-pointers.
 The radix heap also allocates and initialises a node lookup array.  This is
 indexed by item (vertex) number and later used to obtain a pointer to the node
 of any item (vertex) in the heap.  The constructor also allocates and
 initialises the array of upper bounds u.  The Radix heap destructor
 ~RadixHeap() frees all the allocated arrays when the radix heap is destroyed.
 
 The insert() method creates a new RadixHeapNode object newNode, assigning it
 the supplied item and key.  Then the corresponding entry nodes[item] of the
 node lookup array is assigned a pointer to the newNode.  Starting from the last
 bucket (bucket number nBuckets), the placeNode() method then places the node in
 the appropriate bucket; refer to the general description of insert().  Finally,
 the number of items in the heap is incremented.
 
 To decrease the key of an item in the heap, the decreaseKey() method first
 obtains a pointer to the corresponding node by examining the entry nodes[item]
 in the node lookup array.  The node is then removed from its list using the
 removeNode() method, and its key assigned the new value.  Starting from the
 nodes last bucket (bucket number node->bucket), the placeNode() method then
 places the node back in the appropriate bucket of the heap, according to the
 nodes new key value; refer to the general description of decrease-key().
 
 As described deleteMin() is the most complicated operation.  If bucket 1 is
 empty (that is, if the header of bucket 1 points to itself), then any node can
 be removed from bucket 1 and its item returned.  In this case, the node
 obtained by header->next is removed from bucket 1 using the removeNode()
 method.  Then minItem is assigned the nodes item (vertex number) before
 deleting the node and erasing its entry in the node lookup array.  Finally, the
 number of items in the heap is decreased by one and minItem returned.  In cases
 where bucket 1 is empty, the first non-empty bucket (bucket i) is searched for
 (by locating a bucket header that does not point to itself).  The circular
 linked-list representing the non-empty bucket is then scanned to determine the
 minimum node in the bucket.  This is done by updating the minNode pointer and
 minKey value whenever a node with a key smaller than minKey is found.  With the
 minimum node located, the upper bound u[j] of lower indexed buckets j < i is
 updated according to the value of minKey.  Starting from bucket i-1, the
 placedNode() function is then used to reinsert the nodes of bucket i into lower
 indexed buckets, according to their new upper bounds.  The pointers of the
 header node of bucket i are then reset to reflect that bucket i is now empty.
 Then minItem is assigned the nodes item (vertex number) before, deleting the
 node and erasing its entry in the node lookup array.  Finally, the number of
 items in the heap is decreased by one and minItem returned.
 
 As described, the placeNode() method is used when manipulating the heap to
 place a node into its appropriate bucket.  The parameter node is a pointer to
 the node, and the parameter startBucket specifies that the node belongs to a
 bucket i <= startBucket.  The variable, key, is assigned the nodes key.  By
 decreasing i, the first bucket i <= startBucket, that violates u[i] >= key is
 located.  Finally. the node is inserted into bucket i+1 by calling
 insertNode().
 
 Given a pointer to a node, and a bucket index i, the insertNode() method
 inserts the node into the circular doubly-linked list corresponding bucket i.
 First, the node's bucket number is updated.  Then the node is inserted between
 the last actual node in the list (prevNode) and the header node (tailNode) by
 updating the pointers node->next, tailNode->prev, node->prev, and
 prevNode->next.
 
 The removeNode() function is used to remove a node from its corresponding
 bucket when manipulating the heap.  The single parameter, node, is a pointer to
 the node.  The node is removed simply by updating the next and prev pointers of
 neighbouring nodes (node->prev and node->next) in the linked list.
 */

class RadixHeapNode {
public:
    int item;
    double key;
    int bucket;
    RadixHeapNode *next, *prev;
};

class RadixHeap: public Heap {
public:
    RadixHeap(int n);
    ~RadixHeap();
    
    int deleteMin();
    void insert(int item, double k);
    void decreaseKey(int item, double newValue);
    int nItems() const { return itemCount; }
    
    double nComps() const { return compCount; }
    void dump() const;
    
private:
    void placeNode(int startBucket, RadixHeapNode *node);
    void insertNode(int i, RadixHeapNode *node);
    void removeNode(RadixHeapNode *node);
    
    static const int MaxKey = 500000;
    RadixHeapNode **nodes;
    RadixHeapNode *bucketHeaders;
    double *u;
    
    int nBuckets;
    int dMin;
    
    int itemCount;
    int compCount;
};

#endif
