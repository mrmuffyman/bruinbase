#pragma once
#include "RecordFile.h"
struct keyRec{
	int key;
	RecordId record;
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