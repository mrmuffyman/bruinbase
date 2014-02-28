/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 5/28/2008
 */

#ifndef BTNODE_H
#define BTNODE_H

#include "RecordFile.h"
#include "PageFile.h"
#include <vector>
#include <iostream>

template <class T>
class BTbuff
{
public:
    /*
    Public interface:
    Get -> get element at index
    Set -> set element at index
    getLast -> get pageID
    setLast -> set pageID
    */
    //Get
    T get(unsigned int index)
    {
        if (index >= size())
        {
            //Error Handling
            std::cout << "Index out of range" << std::endl;
            return *(T*) c;
        }
        return *(T*)(c + index * sizeof(T));
    }
    void set(unsigned int index, T val)
    {
        if (index >= size())
        {
            //Error Handling
            std::cout << "Index out of range" << std::endl;
            return;
        }
        memcpy((T*)(c + index * sizeof(T) ), &val, sizeof(T));
    }
    // Get and set the PageID int at the end
    int getLast()
    { // casting voodoo to interperet last 4 bytes as an integer
        return *(int*)(c + (PageFile::PAGE_SIZE - sizeof(PageId)));
    }
    void setLast(int val)
    {
        memcpy(&c[PageFile::PAGE_SIZE - sizeof(int)], &val, sizeof(int));
    }
    // Doesn't use any memory. Returns size leaving enough space for PageID
    int size()
    {
        //Size is given by allocating 4 bytes for the last int and the rest for keyRecs
        int remainder = PageFile::PAGE_SIZE - sizeof(int);
        return (int)(remainder / sizeof(T));
    }
    void setZero()
    {
        memset(c, 0, PageFile::PAGE_SIZE);
    }
private:
    char c[PageFile::PAGE_SIZE];
};

struct keyRec{
    RecordId record;
    int key;
    keyRec()
    {
        key = 0;
        record.pid = 0;
        record.sid = 0;
    }
    keyRec(int k, RecordId r)
    {
        key = k;
        record = r;
    }
};

struct keyPid{
    int key;
    PageId pid;
    keyPid()
    {
        key = 0;
        pid = 0;
    }
    keyPid(int k, PageId p)
    {
        key = k;
        pid = p;
    }
};


/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
  public:
	  BTLeafNode()
	  {
		  //empty constructor for now
	  }
   /**
    * Insert the (key, rid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param rid[IN] the RecordId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, const RecordId& rid);

   /**
    * Insert the (key, rid) pair to the node
    * and split the node half and half with sibling.
    * The first key of the sibling node is returned in siblingKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param siblingKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey);

   /**
    * Find the index entry whose key value is larger than or equal to searchKey
    * and output the eid (entry id) whose key value &gt;= searchKey.
    * Remember that keys inside a B+tree node are sorted.
    * @param searchKey[IN] the key to search for.
    * @param eid[OUT] the entry number that contains a key larger              
    *                 than or equalty to searchKey.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locate(int searchKey, int& eid);

   /**
    * Read the (key, rid) pair from the eid entry.
    * @param eid[IN] the entry number to read the (key, rid) pair from
    * @param key[OUT] the key from the slot
    * @param rid[OUT] the RecordId from the slot
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC readEntry(int eid, int& key, RecordId& rid);

   /**
    * Return the pid of the next slibling node.
    * @return the PageId of the next sibling node 
    */
    PageId getNextNodePtr();


   /**
    * Set the next slibling node PageId.
    * @param pid[IN] the PageId of the next sibling node 
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC setNextNodePtr(PageId pid);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
 
   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);

    void printstats();

  private:
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    std::vector<keyRec> mymap;
    PageId nextpage;
	BTbuff<keyRec> buffer;
}; 


/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
  public:
   /**
    * Insert a (key, pid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, PageId pid);

   /**
    * Insert the (key, pid) pair to the node
    * and split the node half and half with sibling.
    * The sibling node MUST be empty when this function is called.
    * The middle key after the split is returned in midKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
    * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey);

   /**
    * Given the searchKey, find the child-node pointer to follow and
    * output it in pid.
    * Remember that the keys inside a B+tree node are sorted.
    * @param searchKey[IN] the searchKey that is being looked up.
    * @param pid[OUT] the pointer to the child node to follow.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locateChildPtr(int searchKey, PageId& pid);

   /**
    * Initialize the root node with (pid1, key, pid2).
    * @param pid1[IN] the first PageId to insert
    * @param key[IN] the key that should be inserted between the two PageIds
    * @param pid2[IN] the PageId to insert behind the key
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC initializeRoot(PageId pid1, int key, PageId pid2);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();

   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);

    void printstats();

  private:
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    std::vector<keyPid> mymap;
    PageId firstpage;
    BTbuff<keyPid> buffer;
}; 

#endif /* BTNODE_H */
