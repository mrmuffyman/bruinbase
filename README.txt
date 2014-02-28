Names: Kevin Lin, Trevor Anderson
For Leaf Node, last 4 bytes are reserved for the next page.
For NonLeafNodes, last 4 bytes are reserved for last pageID.

Size of storage is 1020/12 = 85 keys for leaf nodes
Size of storage is 1020/8 = 127 keys for non-leaf nodes.

Implementation uses a class as a buffer to read/write, and a vector to sort key's. 