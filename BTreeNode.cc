#include "BTreeNode.h"
#include <iostream>

using namespace std;


#define MAX_ELEMENTS (sizeof(buffer)-sizeof(PageId))/sizeof(keyRec)
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	//read into char buffer
	RC ret = pf.pf_read(pid, &buffer);

	//populate vector with elements in buffer
	size_t index = sizeof(keyRec);
	keyRec* iter = buffer; 
	//while(*iter != 0 && sizeof(iter-buffer) <= sizeof(buffer) - sizeof(PageId)){
	//	RecordId r = * (RecordId*) iter;
	//	iter += sizeof(RecordId);
	//	int k = * (int*) iter;
	//	mymap.push_back(keyRec(k, r));
	//	iter += sizeof(int);
	//}

	//set PageId
	nextpage = * (int*)  (buffer + (sizeof(buffer)-sizeof(PageId)));
	return ret; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	keyRec* curr = buffer;	//clear the buffer

	//reconstruct buffer 
 	memset(curr, 0, sizeof(buffer));
	for(int i = 0; i < mymap.size(); i++){
		memcpy(curr, &mymap[i], sizeof(keyRec));
		curr += sizeof(keyRec);
	}
	curr = buffer + (sizeof(buffer)-sizeof(PageId));
	memcpy(curr, &nextpage, sizeof(nextpage));	//add next page
	RC ret = pf.pf_write(pid, &buffer);
	return ret; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 

	return mymap.size();

/*	Older implementation

	char* iter = buffer;
	//based on fact that unused characters are padded as 0's
	while(*iter != 0    &&     sizeof(iter-buffer) <= sizeof(buffer) - sizeof(PageId)){
		iter++;
	}
	size_t len = sizeof(iter-buffer);
	//size of buffer - size of pageID
	size_t recKeyLen = sizeof(RecordId) + sizeof(int);
	int keyCount = len / recKeyLen;*/ 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	if(mymap.size() == MAX_ELEMENTS){
			return RC_NODE_FULL;
	}
	int eid;
	if(locate(key, eid) != 0){	//if locate couldn't find a key
		mymap.push_back(keyRec(key,rid));
	}
	else{
		std::vector<keyRec>::iterator it;
		mymap.insert(mymap.begin()+eid, keyRec(key,rid));	
	}
	return 0; 
}

void BTLeafNode::printstats(){
	for(int i = 0; i < mymap.size(); i++){
		cout<< mymap[i].key << " " << mymap[i].record.pid << "," << mymap[i].record.sid << "\n";
	}
	cout << "nextpage: " << nextpage << "\n";
}
/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
	int mid = mymap.size()/2;
	int count = 0;
	std::vector<keyRec>::iterator it;
	for(it = mymap.end(); it != mymap.begin()+mid; it--){
		sibling.mymap.push_back(*it);
		count++;
	}
	for(int i = 0; i < count; i++){
		mymap.pop_back();
	}

	siblingKey = sibling.mymap[0].key;

	return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	RC ret;
	int tempKey;
	RecordId tempId;
	if(mymap.size() == 0){
		return RC_NO_SUCH_RECORD;
	}
	for(int i = 0; i <= mymap.size(); i++){
		ret = readEntry(i, tempKey, tempId);
		if (ret != 0){
			break;
		}
		else if(mymap[i].key >= searchKey){
				eid = i;
				break;
		}
	}

	return ret; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
	if(eid >= mymap.size()){
		return RC_FILE_SEEK_FAILED;
	}

	key = mymap[eid].key;
	rid = mymap[eid].record;
	return 0;

}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return nextpage;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	nextpage = pid;
	return 0; 
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 

	return 0; }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return 0; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
