#pragma once
#include "RecordFile.h"
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