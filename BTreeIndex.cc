/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include "BTreeIndex.h"
#include "BTreeNode.h"

#define NONE -999

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	RC ret = pf.pf_open(indexname, mode);
	return ret;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC ret = pf.pf_close();
	return ret;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	int midKey = 0; //dummy value unless passed by reference
	PageId splitPid; // <-^

	if (rootPid == -1)
	{
		treeHeight = 1;
		int endpid = pf.endPid();
		rootPid = endpid + 1; //allocate new pagefile for root
		BTNonLeafNode* root = new BTNonLeafNode();
		int newendpid = endpid + 2;
		root->initializeRoot(NONE, key, newendpid);
		root->write(rootPid, pf); // increments endPid() return
		RC ret = insertHelper(key, rid, newendpid, 0, midKey, splitPid);
		return ret;
	}
	// tree exists
	//if require new root, make one
	RC ret = insertHelper(key, rid, rootPid, 0, midKey, splitPid);
	if (midKey != 0){
		BTNonLeafNode* newRoot = new BTNonLeafNode();
		newRoot->initializeRoot(rootPid, midKey, splitPid);
		newRoot->write(pf.endPid() + 1, pf);
		rootPid = pf.endPid();
		treeHeight++;
	}
	return ret;

}
// does inserthelper create a new thing?
/*
	key -> key to insert
	rid -> recordID you want to insert
	pid -> "pointer" PageID of node we are looking at
	height -> height of current node
	ifsplit [OUT] -> set if current node needs to be split: set to a key value
	splitPid [OUT] -> pointer to new node created
	*/
RC BTreeIndex::insertHelper(int key, const RecordId& rid, PageId pid, int height, int& ifsplit, PageId& splitPid){
	//base case: leaf
	//else: recurse through nonleafs
	if (height == treeHeight) // containerNodeent Node is leaf node
	{
		BTLeafNode* containerNode = new BTLeafNode(); //Node container
		int readerr = containerNode->read(pid, pf);
		if (readerr)
		{
			// Read errored out
			return readerr;
		}
		int insertstatus = containerNode->insert(key, rid);
		if (insertstatus)
		{
			//insert found full node: split it
			int splitKey;
			BTLeafNode* split = new BTLeafNode();
			containerNode->insertAndSplit(key, rid, *split, splitKey);
			split->setNextNodePtr(containerNode->getNextNodePtr());
			split->write(pf.endPid() + 1, pf);
			containerNode->setNextNodePtr(pf.endPid());
			ifsplit = splitKey;
			splitPid = pf.endPid();
		}
		int writerr = containerNode->write(pid, pf);
		if (writerr)
		{
			return writerr;
		}
		//all calls passed, returns 0
		return 0;
	}
	// Nonleafs
	BTNonLeafNode* containerNode = new BTNonLeafNode();
	PageId t_pid;
	int readerr = containerNode->read(pid, pf);
	if (readerr)
	{ // couldn't read, return error code
		return readerr;
	}
	int childPtrErr = containerNode->locateChildPtr(key, t_pid);
	if (childPtrErr )
	{	// couldn't follow pid to child node, return error code
		return childPtrErr;
	}
	int propagatedKey = 0;
	PageId propagatedPid;
	insertHelper(key, rid, t_pid, height + 1, propagatedKey, propagatedPid);
	if (propagatedKey != 0)
	{
		//try to insert else push up new splitkey (Non Leaf Node Overflow)
		int nodeFull = containerNode->insert(propagatedKey, propagatedPid);
		if (nodeFull )
		{
			int midKey;
			BTNonLeafNode* split = new BTNonLeafNode();
			containerNode->insertAndSplit(propagatedKey, propagatedPid, *split, midKey);
			containerNode->write(pid, pf);
			split->write(pf.endPid() + 1, pf);
			ifsplit = midKey;
			splitPid = pf.endPid();
		}
	}
	return 0;
}
/*
 * Find the leaf-node index entry whose key value is larger than or
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	locateHelper(searchKey, cursor, rootPid, 0);
	return 0;
}

RC BTreeIndex::locateHelper(int searchKey, IndexCursor& cursor, PageId pid, int height)
{
	if(height == treeHeight)
	{
		BTLeafNode* container = new BTLeafNode();
		container->read(pid, pf);
		int eid;
		RC notFound;
		located = container->locate(searchKey, eid);
		if(notFound){
			if(container.getNextNodePtr() == 0){	//rightmost leaf node
				return RC_NO_SUCH_RECORD;
			}
			//call on right adjacent leaf node
			locateHelper(searchKey, cursor, container.getNextNodePtr(), height);
		}
		else{
			cursor = {pid, eid};
			return 0;
		}
	}
	else
	{
		BTLeafNode* container = new BTNonLeafNode();
		container->read(pid, pf);
		PageId nextPid;
		container->locateChildPtr(searchKey, nextPid);
		locateHelper(searchKey, cursor, nextPid, height+1);
	}
	return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	return 0;
}
