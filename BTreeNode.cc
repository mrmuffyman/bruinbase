#include "BTreeNode.h"
#include <iostream>

using namespace std;


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
	//keyRec* iter = buffer; 
	int length = buffer.size();
	for(int i = 0; i < length; i++)
	{ 
		keyRec t = buffer.get(i);
		if(t.key == 0){
			break;
		}
		else{
			mymap.push_back(t);
		}
	} 
	//set PageId
	nextpage = buffer.getLast(); //* (int*)  (buffer + (sizeof(buffer)-sizeof(PageId)));
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
	//keyRec* curr = buffer;	//clear the buffer

	//reconstruct buffer 
 	//memset(buffer, 0, sizeof(buffer));
	buffer.setZero();
	for(int i = 0; i < mymap.size(); i++){
		buffer.set(i, mymap[i]);
		//memcpy(curr, &mymap[i], sizeof(keyRec));
		//curr += sizeof(keyRec);
	}
	//curr = buffer + (sizeof(buffer)-sizeof(PageId));
	//Add next page
	buffer.setLast(nextpage);
	//memcpy(curr, &nextpage, sizeof(nextpage));	//add next page
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

}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	if(mymap.size() == buffer.size()){
			return RC_NODE_FULL;
	}
	int eid;
	if(locate(key, eid) != 0){	//if locate couldn't find a key
		mymap.push_back(keyRec(key,rid));
	}
	else{
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
	//code from BTLeafNode::insert, minus checking if buffer is full
	int eid;
	if(locate(key, eid) != 0){	//if locate couldn't find a key
		mymap.push_back(keyRec(key,rid));
	}
	else{
		std::vector<keyRec>::iterator it;
		mymap.insert(mymap.begin()+eid, keyRec(key,rid));	
	}

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
	RC ret = pf.pf_read(pid, &buffer);
	int length = buffer.size();
	for(int i = 0; i < length; i++)
	{ 
		keyPid t = buffer.get(i);
		if(t.key == 0){	//this is an assumption to keep in mind
			break;
		}
		else{
			mymap.push_back(t);
		}
	} 

	//set PageId
	nextpage = buffer.getLast();
	return ret; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	buffer.setZero();
	for(int i = 0; i < mymap.size(); i++){
		buffer.set(i, mymap[i]);
	}
	buffer.setLast(nextpage);
	RC ret = pf.pf_write(pid, &buffer);
	return ret; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 

	return mymap.size(); 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	//check if node is full
	if(mymap.size() == buffer.size()){
			return RC_NODE_FULL;
	}

	//if you find a key that's greater than the entry key, insert the entry key right before
	for(int i = 0; i < mymap.size(); i++){
		if(mymap[i].key > key){
			mymap.insert(mymap.begin()+i, keyPid(key,pid));
			return 0;	
		}
	}
	//else, entry key is the greatest, so push it to the back
	mymap.push_back(keyPid(key,pid));
	return 0; 
}

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
{ 
	//code from insert, minus buffer size check
	bool inserted = false;
	for(int i = 0; i < mymap.size(); i++){
		if(mymap[i].key > key){
			mymap.insert(mymap.begin()+i, keyPid(key,pid));
			inserted = true;
		}
	}
	if(!inserted){
		mymap.push_back(keyPid(key,pid));
	}

	int mid = mymap.size()/2;
	int count = 0;
	std::vector<keyPid>::iterator it;

	//iterate to right after the middle
	for(it = mymap.end(); it != mymap.begin()+mid+1; it--){
		sibling.mymap.push_back(*it);
		count++;		//count = number of elements to pop off
	}
	//at the end of iteration it points to middle
	midKey = (*it).key;
	for(int i = 0; i < count+1; i++){
		mymap.pop_back(); 	//pop off count elements + 1 (middle element)
	}

	return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 	
	//if node is empty
	if(mymap.size() == 0){
		return RC_NO_SUCH_RECORD;
	}
	//once a key is found greater than the search key, return its pointer
	for(int i = 0; i < buffer.size(); i++){
		if(mymap[i].key > searchKey){
			pid = mymap[i].pid;
			return 0;
		}
	}
	//else the search key is greater than all keys, so return the rightmost poitner
	pid = nextpage;
	return 0; 
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
	insert(key, pid1);
	nextpage = pid2;
	return 0; 
}
