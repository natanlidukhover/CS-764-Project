#pragma once

#include "defs.h"
# include<fstream>
typedef uint64_t RowCount;

class Plan
{
	friend class Iterator;
public:
	Plan (ofstream  &outputStream);
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
	 ofstream  &outTrace;
private:
}; // class Plan

class Iterator
{
public:
	Iterator (ofstream  &outputStream);
	virtual ~Iterator ();
	void run ();
	virtual bool next () = 0;
	 ofstream  &outTrace;

private:
	RowCount _count;
}; // class Iterator
