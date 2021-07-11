// query.c ... query scan functions
// part of signature indexed files
// Manage creating and using Query objects
// Written by John Shepherd, March 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q)
{
	if (*q == '\0') return 0;
	char *c;
	int nattr = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nattr++;
	return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs)
{
	Query new = malloc(sizeof(QueryRep));
	assert(new != NULL);
	if (!checkQuery(r,q)) return NULL;
	new->rel = r;
	new->qstring = q;
	new->nsigs = new->nsigpages = 0;
	new->ntuples = new->ntuppages = new->nfalse = 0;
	//new->pages = newBits(nPages(r));
	new->pages = newBits(bsigBits(r));
	switch (sigs) {
	case 't': findPagesUsingTupSigs(new); break;
	case 'p': findPagesUsingPageSigs(new); break;
	case 'b': findPagesUsingBitSlices(new); break;
	default:  setAllBits(new->pages); break;
	}
	new->curpage = 0;
	return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void scanAndDisplayMatchingTuples(Query q)
{
	assert(q != NULL);
	int i, j, find;
	Page temp;
	Tuple cur_tup;			//tuple from the page
	Tuple qur_tup;			//tuple in the query
	qur_tup = q->qstring;
	for (i=0; i<nPages(q->rel); i++){
		if (bitIsSet(q->pages,i) == FALSE) continue;		//PID is not set in MatchingPages
		temp = getPage(dataFile(q->rel), i);
		//q->nsigpages++;										
		q->curpage = i;
		//q->nsigs = i;
		q->ntuppages++;
		find = 0;
		for (j=0; j<pageNitems(temp); j++){					//iterate all tuples in that page
			cur_tup = getTupleFromPage(q->rel, temp, j);
			//printf("%s\n", cur_tup);
			q->ntuples++;
			q->curtup = j;
			if (tupleMatch(q->rel, cur_tup, qur_tup) == TRUE){
				showTuple(q->rel, cur_tup);
				find = 1;
			}
		}
		if (find == 0){			//a false match page
			q->nfalse++;
		}
		free(temp);
	}

}

// print statistics on query

void queryStats(Query q)
{
	printf("# sig pages read:    %d\n", q->nsigpages);
	printf("# signatures read:   %d\n", q->nsigs);
	printf("# data pages read:   %d\n", q->ntuppages);
	printf("# tuples examined:   %d\n", q->ntuples);
	printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q->pages);
	free(q);
}

