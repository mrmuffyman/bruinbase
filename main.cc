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
#include "BTbuff.h"

int main()
{
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
	BTLeafNode* node = new BTLeafNode();
	RecordId d = {0,0};
	node->insert(3,d);
	node->insert(1,d);
	node->setNextNodePtr(1);
	node->printstats();
//	node->insert(2,d);
								//fname      mode
	PageFile* pf = new PageFile("testpage.test", 'w');
	PageFile* pf2 = new PageFile("booty.test", 'r');
	node->write(0, *pf);



	BTLeafNode* node2 = new BTLeafNode();
	node2->read(0, *pf);
	node2->printstats();

    return 0;
}
