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
#include "BTreeIndex.h"
#include <iostream>

int main()
{
/*	auto BT = new BTreeIndex();
	BT->open("zubat.isabat", 'w');
	BT->insert(1,RecordId(0,0));
	BT->insert(2, RecordId(0, 0));
	BT->insert(3, RecordId(0, 0));
	BT->insert(4, RecordId(0, 0));
	auto n = new BTNonLeafNode();
	n->read(1, BT->pf);
	auto m = new BTLeafNode();
	m->read(2, BT->pf);
	BT->close();*/
/*	auto teemo = new BTbuff();
	auto q = teemo->getLast();
	teemo->setLast(9999);
	auto r = teemo->getLast();
	teemo->setZero();
	auto s = teemo->getLast();
	auto a = teemo->get(0);
	auto b = teemo->get(9);
	keyRec c;
	c.key = 1;
	c.record.pid = 2;
	c.record.sid = 3;
	teemo->set(0, c);
	teemo->set(9, c);
	auto e = teemo->get(0);
	auto f = teemo->get(9);*/
  // run the SQL engine taking user commands from standard input (console).
 // SqlEngine::run(stdin);

//Some LeafNode Testing
	BTLeafNode* node = new BTLeafNode();
	RecordId d = RecordId(0,0);
	node->insert(1,d);
	node->setNextNodePtr(1);
								//fname      mode
	PageFile* pf = new PageFile("testpage.test", 'w');
	node->write(0, *pf);

	BTLeafNode* node2 = new BTLeafNode();
	node2->read(0, *pf);

	int siblingKey;
	BTLeafNode* sibling = new BTLeafNode();
	node->insertAndSplit(4, d, *sibling, siblingKey);
	node->printstats();
	sibling->printstats();


//Some NonLeafNode Testing
	BTNonLeafNode* nade = new BTNonLeafNode();
	nade->insert(4,0);
	nade->insert(1,0);
	nade->insert(2,0);
	nade->insert(5,0);
	PageFile* pfnonleaf = new PageFile("testpage.test", 'w');
	nade->write(0, *pfnonleaf);
	BTNonLeafNode* gre = new BTNonLeafNode();
	int midkey;
	nade->insertAndSplit(3, 0, *gre, midkey);
	nade-> printstats();
	gre->printstats();
    return 0;
}
