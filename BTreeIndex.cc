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
	int midKey = 0;
	PageId splitPid;

	if(rootPid == -1)
	{
		treeHeight = 1;
		rootPid = pf.endPid()+1;
		BTNonLeafNode* root = new BTNonLeafNode();
		root->initializeRoot(NONE, key, pf.endPid()+2); 
		root->write(pf.endPid()+1, pf);
		RC ret = insertHelper(key, rid, pf.endPid()+1, 0, splitkey, splitPid);
		return ret;
	}
	else
	{
		//if require new root, make one
		int midKey = 0;
		RC ret = insertHelper(key, rid, rootPid, 0, midKey, splitPid);
		if(midKey != 0){
			BTNonLeafNode* newRoot = new BTNonLeafNode();
			newRoot->initializeRoot(rootPid, midKey, splitPid);
			newRoot->write(pf.endPid()+1, pf);
		}
    	return ret;
    }
}
// does inserthelper create a new thing?

RC BTreeIndex::insertHelper(int key, const RecordId& rid, PageId pid, int height, int& ifsplit, PageId& splitPid){
	//base case: leaf
	//else: recurse through nonleafs

	RC stillGood; //if any of these calls fail then it'll return the error code

	if(height == treeHeight)
	{

		BTLeafNode* curr = new BTLeafNode();
		if(stillGood = curr->read(pid,pf))
		{		
			return stillGood;
		}
		if(stillGood = curr->insert(key, rid))
		{
			//insert failed because node was full
			int splitKey;
			BTLeafNode* split = new BTLeafNode();
			curr->insertAndSplit(key,rid, *split, splitKey);
			split->setNextNodePtr(curr->getNextNodePtr());
			split->write(pf.endPid()+1, pf);
			curr->setNextNodePtr(pf.endPid());
			ifsplit = splitKey;
			splitPid = pf.endPid();
		}
		if(stillGood = curr->write(pid, pf))
		{
			return stillGood;
		}
		//all calls passed, returns 0
		return stillGood;
	}
	else
	{
		BTNonLeafNode* curr = new BTNonLeafNode();
		PageId t_pid;
		if(stillGood = curr->read(pid,pf))
			return stillGood;
		if(stillGood = curr->locateChildPtr(key, t_pid))
			return stillGood;
		int splitkey = 0;
		PageId splitPid; 
		insertHelper(key, rid, t_pid, height+1, splitkey, splitPid);
		if(splitkey != 0)
		{
			//try to insert else push up new splitkey (Non Leaf Node Overflow)
			if(stillGood = 	curr->insert(splitkey, splitPid))
			{
				int midKey;
				BTNonLeafNode* split = new BTNonLeafNode();
				curr->insertAndSplit(splitkey,splitPid, *split, midKey);
				split->write(pf.endPid()+1, pf);
				ifsplit = midKey;
				splitPid = pf.endPid();
				return stillGood;
			}	
		}
	}

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
