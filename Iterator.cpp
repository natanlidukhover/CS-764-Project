#include "Iterator.h"

Plan::Plan (ofstream &outputStream) : outTrace(outputStream)
{
	TRACE (true, outTrace);
} // Plan::Plan

Plan::~Plan ()
{
	//TRACE (true);
} // Plan::~Plan

Iterator::Iterator (ofstream  &outputStream) : _count (0),outTrace(outputStream)
{
	TRACE (true,outTrace);
} // Iterator::Iterator

Iterator::~Iterator ()
{
	TRACE (true,outTrace);
} // Iterator::~Iterator

void Iterator::run ()
{
	TRACE (true, outTrace);

	while (next ())  ++ _count;

	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _count);
} // Iterator::run
