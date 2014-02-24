/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"
#include <iostream>

int main()
{
  // run the SQL engine taking user commands from standard input (console).
 // SqlEngine::run(stdin);
	BTLeafNode* node = new BTLeafNode();
	RecordId d = {0,0};
	node->insert(3,d);
	node->insert(1,d);
	node->setNextNodePtr(1);
	node->printstats();
//	node->insert(2,d);

	PageFile* pf = new PageFile("testpage", 'w');
	node->write(0, *pf);

	BTLeafNode* node2 = new BTLeafNode();
	node2->read(0, *pf);
	node2->printstats();

    return 0;
}
