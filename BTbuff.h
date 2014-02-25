#pragma once
#include <iostream>
#include "keyRec.h"
template <class T>
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
	T get(unsigned int index)
	{
		if (index >= size())
		{
			//Error Handling
			std::cout << "Index out of range" << std::endl;
			return *(T*) c;
		}
		return *(T*)(c + index * sizeof(T));
	}
	void set(unsigned int index, T val)
	{
		if (index >= size())
		{
			//Error Handling
			std::cout << "Index out of range" << std::endl;
			return;
		}
		memcpy((T*)(c + index * sizeof(T) ), &val, sizeof(T));
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
	// Doesn't use any memory. Returns size leaving enough space for PageID
	int size()
	{
		//Size is given by allocating 4 bytes for the last int and the rest for keyRecs
		int remainder = PageFile::PAGE_SIZE - sizeof(int);
		return (int)(remainder / sizeof(T));
	}
	void setZero()
	{
		memset(c, 0, PageFile::PAGE_SIZE);
	}
private:
	char c[PageFile::PAGE_SIZE];
};
