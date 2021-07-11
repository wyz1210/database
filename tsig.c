// tsig.c ... functions on Tuple Signatures (tsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

//make a codeword for a attribute

Bits codeword(char *attr_value, int m, int k, int label, int length){
   int  nbits = 0;   					// count of set bits
   Bits cword = newBits(length);   
   srandom(hash_any(attr_value, strlen(attr_value)));
   if (label == 1){						// SIMC
	while (nbits < k) {
		int i = random() % m;
		if (bitIsSet(cword, i) == FALSE) {
			setBit(cword, i);
			nbits++;
		}
	}
   }
   else if (label == 0){				//CATC
	while (nbits < k) {
		int i = random() % m ;
		if (bitIsSet(cword, i) == FALSE) {
			setBit(cword, i);
			nbits++;
		}
	}	
   }
   return cword;  // m-bits with k 1-bits and m-k 0-bits
}

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
	Bits result = newBits(tsigBits(r));
	Bits temp;
	int m, k;
	int shiftbit;
	/*special for the first attribute of CATC*/
	int bitsForFisrtAtrri;
	char **v = tupleVals(r, t);
	int i, n = nAttrs(r);
	m = tsigBits(r);
	k = codeBits(r);	
	if (sigType(r) == 's'){			//SIMC
		for (i=0; i<n; i++){
			if (v[i][0] == '?'){
				temp  = newBits(tsigBits(r));
			}
			else temp = codeword(v[i], m, k, 1, m);
			orBits(result, temp);
			freeBits(temp);
		}
	}
	else if (sigType(r) == 'c'){	//CATC	
		assert(n>0);
		bitsForFisrtAtrri = m / n + m % n;
		shiftbit = 0;
		if (v[0][0] == '?') temp = newBits(m);
		else temp = codeword(v[0], bitsForFisrtAtrri, bitsForFisrtAtrri/2, 0, m);
		shiftbit += bitsForFisrtAtrri;
		orBits(result, temp);
		freeBits(temp);
		for (i=1; i<n; i++){
			if (v[i][0] == '?') temp = newBits(m);
			else temp = codeword(v[i], m/n, (m/n)/2, 0, m);
			//if (i ==1) shiftBits(temp, bitsForFisrtAtrri);
			//else shiftBits(temp, (m/n) * (i+1));
			shiftBits(temp, shiftbit);
			shiftbit += m/n;
			orBits(result, temp);
			freeBits(temp);
		}

	}
	freeVals(v, n);
	return result;
}

//find the page where a tuple is in 
int findTuplePage(Reln r, int index){
	File f = dataFile(r);
	int i, pageId, count;
	Page tempPage;
	count = 0;			//count how many tuples have been iterated
	pageId = 0;
	for (i=0; i<nPages(r); i++){		
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

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	//TODO
	int i, j, tsigIndex, tuplePageId;
	Bits QuerySig;
	Page temp;
	Bits tsig;
	tsigIndex = 0;
	tsig = newBits(tsigBits(q->rel));
	QuerySig = makeTupleSig(q->rel, q->qstring);
	unsetAllBits(q->pages);
	for (i=0; i<nTsigPages(q->rel); i++){
		temp = getPage(tsigFile(q->rel), i);
		q->nsigpages++;
		for (j=0; j<pageNitems(temp); j++){
			getBits(temp, j, tsig); 
			q->nsigs++;
			if (isSubset(QuerySig, tsig) == TRUE){
				tuplePageId = tsigIndex / maxTupsPP(q->rel);
				setBit(q->pages, tuplePageId);
			}
			tsigIndex++;
		}
		free(temp);
	} 

	// The printf below is primarily for debugging
	// Remove it before submitting this function
	//printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
