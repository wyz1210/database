// bsig.c ... functions on Tuple Signatures (bsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"
#include "reln.h"

void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);
	//TODO
	int i;
	PageID pid, lastpid;
	Page tempPage;
	/*special for bitslice*/
	int *indexInBslice = malloc(sizeof(int));
	Bits Qsig = makePageSig(q->rel, q->qstring);
	//printf("this is pagesig"); showBits(Qsig);
	Bits slice = newBits(bsigBits(q->rel));
	setAllBits(q->pages);
	lastpid = -1;
	for (i=0; i<psigBits(q->rel); i++){
		if (bitIsSet(Qsig, i) == TRUE){
			*indexInBslice = i;							//find the index of bitslice in the page
			pid = i / maxBsigsPP(q->rel);
			*indexInBslice = i - pid * maxBsigsPP(q->rel);
			//printf("pid is %d, indexInBslice is %d\n", pid, *indexInBslice);
			tempPage = getPage(q->rel->bsigf, pid);
			if (lastpid != pid){
				printf("the %d page add one\n", i);
				q->nsigpages++;
				lastpid = pid;
			}
			getBits(tempPage, *indexInBslice, slice);	//get the i'th bitslice
			q->nsigs++;
			andBits(q->pages, slice);					//zero bits in Pages which are zero in Slice
			/*
			for(int j=0; j<nPages(q->rel); j++){
				if (!bitIsSet(slice, j)){
					unsetBit(q->pages, j);
				}
			}
			*/
			putPage(q->rel->bsigf, pid, tempPage);	
			//free(tempPage);
		}
	}
	
	//printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

