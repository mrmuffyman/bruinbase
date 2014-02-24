/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
//#include "SqlEngine.h"
#include "BTreeNode.h"
#include <iostream>

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
	keyRec get(unsigned int index)
	{
		if (index >= size())
		{
			//Error Handling
			std::cout << "Index out of range" << std::endl;
		}
		return *(keyRec*)(c + index);
	}
	void set(unsigned int index, keyRec val)
	{
		if (index >= size())
		{
			//Error Handling
			std::cout << "Index out of range" << std::endl;
		}
		memcpy((keyRec*)(c + index),&val,sizeof(keyRec));
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
private:
	// Doesn't use any memory. Returns size leaving enough space for PageID
	int size()
	{
		//Size is given by allocating 4 bytes for the last int and the rest for keyRecs
		int remainder = PageFile::PAGE_SIZE - sizeof(int);
		return (int)(remainder / sizeof(keyRec));
	}
	char c[PageFile::PAGE_SIZE];
};
int main()
{
	auto teemo = new BTbuff();
	auto q = teemo->getLast();
	teemo->setLast(9999);
	auto r = teemo->getLast();
	keyRec t;
	t.record.pid = 1;
	t.record.sid = 2;
	t.key = 3;
	keyRec u = teemo->get(0);
	teemo->set(-7, t);
	auto v = teemo->get(0);

  // run the SQL engine taking user commands from standard input (console).
 // SqlEngine::run(stdin);
	BTLeafNode* node = new BTLeafNode();
	RecordId d = {0,0};
	node->insert(3,d);
	node->insert(1,d);
	node->setNextNodePtr(1);
	node->printstats();
//	node->insert(2,d);
								//fname      mode
	PageFile* pf = new PageFile("testpage.test", 'w');
	PageFile* pf2 = new PageFile("testpage.test", 'w');
	node->write(0, *pf);
	node->read(0, *pf);


	BTLeafNode* node2 = new BTLeafNode();
	node2->read(0, *pf);
	node2->printstats();

    return 0;
}
