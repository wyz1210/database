// psig.c ... functions on page signatures (psig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "tsig.h"


Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
	Bits result = newBits(psigBits(r));
	Bits temp;
	int m, k;
	int shiftbit;
	/*special for the first attribute of CATC*/
	int bitsForFisrtAtrri;
	char **v = tupleVals(r, t);
	int i, n = nAttrs(r);
	m = psigBits(r);
	k = codeBits(r);	
	if (sigType(r) == 's'){			//SIMC
		for (i=0; i<n; i++){
			if (v[i][0] == '?') temp = newBits(m);
			else temp = codeword(v[i], m, k, 1, m);
			orBits(result, temp);
			freeBits(temp);
		}
	}
	else if (sigType(r) == 'c'){	//CATC	
		assert(n>0);
		bitsForFisrtAtrri = m / n + m % n;
		shiftbit = 0;
		k = maxTupsPP(r) *2;
		if (v[0][0] == '?') {
			temp = newBits(m);
		}
		else temp = codeword(v[0], bitsForFisrtAtrri, bitsForFisrtAtrri/k, 0, m);
		shiftbit += bitsForFisrtAtrri;
		orBits(result, temp);
		freeBits(temp);
		for (i=1; i<n; i++){
			if (v[i][0] == '?') temp = newBits(m);
			else temp = codeword(v[i], m/n, (m/n)/k, 0, m);
			//if (i ==1) shiftBits(temp, bitsForFisrtAtrri);
			//else shiftBits(temp, m/n);
			shiftBits(temp, shiftbit);
			shiftbit += m/n;
			orBits(result, temp);
			freeBits(temp);
		}

	}
	freeVals(v, n);
	return result;
}

//find the page where a psig is in 
int findPsigPage(Reln r, int index){
	File f = dataFile(r);
	int i, pageId, count;
	Page tempPage;
	count = 0;			//count how many pids have been iterated
	pageId = 0;
	for (i=0; i<nPsigPages(r); i++){		
		tempPage = getPage(f, i);
		count += pageNitems(tempPage);
		free(tempPage);
		if (count >= index){
			pageId = i;
			break;
		}
	}
	return pageId;
}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	//TODO
	Reln r = q->rel;
	Bits pageSig = newBits(psigBits(r));
	Bits querysig;
	int i, j, psigIndex, PageId;
	Page tempPage;
	psigIndex = 0;
	querysig = makePageSig(r, q->qstring);
	unsetAllBits(q->pages);
	for (i=0; i<nPsigPages(r); i++){
		tempPage = getPage(psigFile(r), i);
		q->nsigpages++;
		for (j=0; j<pageNitems(tempPage); j++){
			getBits(tempPage, j, pageSig);
			q->nsigs++;
			if (isSubset(querysig, pageSig) == TRUE){
				//PageId = findPsigPage(r, psigIndex);
				PageId = psigIndex;
				//printf("page id is %d, i is %d, j is %d, psigIndex is %d\n",PageId, i , j,psigIndex);
				setBit(q->pages, PageId);
			}
			psigIndex++;
		}
		free(tempPage);
	}	
	//printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

