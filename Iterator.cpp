#include "Iterator.h"

Plan::Plan () 
{
} // Plan::Plan

Plan::~Plan ()
{
	//TRACE (true);
} // Plan::~Plan

Iterator::Iterator () : _count (0)
{
} // Iterator::Iterator

Iterator::~Iterator ()
{
} // Iterator::~Iterator

void Iterator::run ()
{

	while (next ())  ++ _count;

	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _count);
} // Iterator::run
