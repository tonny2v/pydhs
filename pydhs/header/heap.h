#ifndef HEAP_H
#define HEAP_H
/* File heap.h - Abstract Base Class for Heaps
 * ----------------------------------------------------------------------------
 *  Shane Saunders
 */

/* --- Heap ---
 * This is an abstract base class from which specific heap classes can be
 * derived.  Different heaps derived from this abstract base class can be used
 * interchangeably by algorithms that were written using the universal
 * interface it provides.
 *
 * This heap stores integer items, and associates with each item a long integer
 * key.  Any derived heap heap must provide the following methods:
 *
 * deleteMin()    - removes the item with the minimum key from the heap, and
 *                  returns it.
 * insert()       - inserts an item 'item' with key 'key' into the heap.
 * decreaseKey()  - decreases the key of item 'item' to the new value newKey.
 * nItems()       - returns the number of items currently in the heap.
 * nComps()       - returns the number of key comparison operations.
 * dump()         - prints a text representation of the heap to the standard
 *                  output.
 */
class Heap {
public:
    virtual ~Heap() { };
    virtual int deleteMin() = 0;
    virtual void insert(int item, double key) = 0;
    virtual void decreaseKey(int item, double newKey) = 0;
    virtual int nItems() const = 0;
    virtual double nComps() const = 0;
    virtual void dump() const = 0;
};

class HeapDesc {
public:
    virtual ~HeapDesc() { };
    virtual Heap *newInstance(int n) const = 0;
};

template <class T>
class HeapD: public HeapDesc {
public:
    Heap *newInstance(int n) const { return new T(n); };
};

#endif
