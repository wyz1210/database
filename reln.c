// reln.c ... functions on Relations
// part of signature indexed files
// Written by John Shepherd, March 2019

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "reln.h"
#include "page.h"
#include "tuple.h"
#include "tsig.h"
#include "bits.h"
#include "hash.h"
#include "psig.h"

// open a file with a specified suffix
// - always open for both reading and writing

File openFile(char *name, char *suffix)
{
	char fname[MAXFILENAME];
	sprintf(fname,"%s.%s",name,suffix);
	File f = open(fname,O_RDWR|O_CREAT,0644);
	assert(f >= 0);
	return f;
}

// create a new relation (five files)
// data file has one empty data page

Status newRelation(char *name, Count nattrs, float pF, char sigtype,
                   Count tk, Count tm, Count pm, Count bm)
{
	Reln r = malloc(sizeof(RelnRep));
	RelnParams *p = &(r->params);
	Bits tempBits;
	Page tempPage;
	int i, currPage;
	//int pagenumber;
	assert(r != NULL);
	p->nattrs = nattrs;
	p->pF = pF,
	p->sigtype = sigtype;
	p->tupsize = 28 + 7*(nattrs-2);
	Count available = (PAGESIZE-sizeof(Count));
	p->tupPP = available/p->tupsize;
	p->tk = tk; 
	if (tm%8 > 0) tm += 8-(tm%8); // round up to byte size
	p->tm = tm; p->tsigSize = tm/8; p->tsigPP = available/(tm/8);
	if (pm%8 > 0) pm += 8-(pm%8); // round up to byte size
	p->pm = pm; p->psigSize = pm/8; p->psigPP = available/(pm/8);
	if (p->psigPP < 2) { free(r); return -1; }
	if (bm%8 > 0) bm += 8-(bm%8); // round up to byte size
	p->bm = bm; p->bsigSize = bm/8; p->bsigPP = available/(bm/8);
	if (p->bsigPP < 2) { free(r); return -1; }
	r->infof = openFile(name,"info");
	r->dataf = openFile(name,"data");
	r->tsigf = openFile(name,"tsig");
	r->psigf = openFile(name,"psig");
	r->bsigf = openFile(name,"bsig");
	addPage(r->dataf); p->npages = 1; p->ntups = 0;
	addPage(r->tsigf); p->tsigNpages = 1; p->ntsigs = 0;
	addPage(r->psigf); p->psigNpages = 1; p->npsigs = 0;
	//addPage(r->bsigf); p->bsigNpages = 1; p->nbsigs = 0; // replace this
	// Create a file containing "pm" all-zeroes bit-strings,
    // each of which has length "bm" bits
	//TODO

	/*
	if (pm % p->bsigPP == 0) pagenumber = (pm / p->bsigPP); 
	else pagenumber = (pm / p->bsigPP) +1; 
	p->bsigNpages = 0; p->nbsigs = 0;
	for (currPage=0; currPage< pagenumber; currPage++){
		addPage(r->bsigf);
		p->bsigNpages++;
		tempPage = getPage(r->bsigf, currPage);
		for (i=0; i<p->bsigPP; i++){
			if (p->nbsigs == pm) break;
			tempBits = newBits(bm);
			putBits(tempPage, i, tempBits);
			addOneItem(tempPage);
			p->nbsigs++;					//increment the number of total bsigs
		}
		putPage(r->bsigf, currPage, tempPage);
	}
	*/

	
	addPage(r->bsigf); p->bsigNpages = 1; p->nbsigs = 0;
	currPage = 0;
	tempPage = getPage(r->bsigf, currPage);
	for (i = 0; i<pm; i++){
		if (pageNitems(tempPage) == p->bsigPP){			//add a newpage because filling up a page
			putPage(r->bsigf, currPage, tempPage);		//put the page into the file 
			addPage(r->bsigf);
			p->bsigNpages++;
			currPage++;
			tempPage = getPage(r->bsigf, currPage);		//switch to a new page
		}
		tempBits = newBits(bm);		
		putBits(tempPage, pageNitems(tempPage), tempBits);
		addOneItem(tempPage);
		p->nbsigs++;					//increment the number of total bsigs
	}
	putPage(r->bsigf, currPage, tempPage);		//put the page into the file 
	
	closeRelation(r);
	return 0;
}

// check whether a relation already exists

Bool existsRelation(char *name)
{
	char fname[MAXFILENAME];
	sprintf(fname,"%s.info",name);
	File f = open(fname,O_RDONLY);
	if (f < 0)
		return FALSE;
	else {
		close(f);
		return TRUE;
	}
}

// set up a relation descriptor from relation name
// open files, reads information from rel.info

Reln openRelation(char *name)
{
	Reln r = malloc(sizeof(RelnRep));
	assert(r != NULL);
	r->infof = openFile(name,"info");
	r->dataf = openFile(name,"data");
	r->tsigf = openFile(name,"tsig");
	r->psigf = openFile(name,"psig");
	r->bsigf = openFile(name,"bsig");
	read(r->infof, &(r->params), sizeof(RelnParams));
	return r;
}

// release files and descriptor for an open relation
// copy latest information to .info file
// note: we don't write ChoiceVector since it doesn't change

void closeRelation(Reln r)
{
	// make sure updated global data is put in info file
	lseek(r->infof, 0, SEEK_SET);
	int n = write(r->infof, &(r->params), sizeof(RelnParams));
	assert(n == sizeof(RelnParams));
	close(r->infof); close(r->dataf);
	close(r->tsigf); close(r->psigf); close(r->bsigf);
	free(r);
}

//find the page where a bslice is in 
int findBsilcePage(Reln r, int index, int *pos){
	int i,  count;//,pageId;
	//Page tempPage;
	count = 0;			//count how many bitslice have been iterated
	//pageId = 0;
	for (i=0; i<nBsigPages(r); i++){		
		//tempPage = getPage(f, i);
		count += maxBsigsPP(r);
		//free(tempPage);
		if (count >= index){
			//pageId = i;
			*pos = maxBsigsPP(r) - count + index -1;		//the index in the page
			break;
		}
		//(*pos) -= itemsInLastPage;	
	}
	return i;
}

// insert a new tuple into a relation
// returns page where inserted
// returns NO_PAGE if insert fails completely

PageID addToRelation(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL && strlen(t) == tupSize(r));
	Page p; 
	Page pForTsig;
	PageID pid, pSigId, bSliceID;
	RelnParams *rp = &(r->params);
	Bits Psig, PPsig;		//used for updating psig
	Bits tempBits;
	int i;
	/*special for bitslice*/
	int *indexInBslice = malloc(sizeof(int));

	// add tuple to last page
	pid = rp->npages-1;
	p = getPage(r->dataf, pid);
	// check if room on last page; if not add new page
	if (pageNitems(p) == rp->tupPP) {
		addPage(r->dataf);
		rp->npages++;
		pid++;
		free(p);
		p = newPage();
		if (p == NULL) return NO_PAGE;
	}
	addTupleToPage(r, p, t);
	rp->ntups++;  //written to disk in closeRelation()
	putPage(r->dataf, pid, p);

	

	/*update the psig*/
	Psig = makePageSig(r, t);
	pSigId = rp->psigNpages -1;
	pid = rp->npages -1;
	if (pid >= nPsigs(r)){					//no psig for this page yet
		p = getPage(r->psigf, nPsigPages(r) -1);
		//i = pageNitems(p);
		// check if room on last page; if not add new page
		if (pageNitems(p) == rp->psigPP){
			addPage(r->psigf);
			rp->psigNpages++;
			pSigId++;
			free(p);
			p = newPage();
			if (p == NULL) return NO_PAGE;
			putBits(p, 0, Psig);
			addOneItem(p);
		}
		else {
			putBits(p, pageNitems(p), Psig);
			addOneItem(p);
			
		}
		//putBits(p, pageNitems(p)-1, Psig);
		rp->npsigs++;						//increment the number of page signatures	
		putPage(r->psigf, pSigId, p);
	}
	else {									//this page has a psig already and update it with the current psig
		p = getPage(r->psigf, nPsigPages(r) -1);
		PPsig = newBits(rp->pm);
		getBits(p, pageNitems(p)-1, PPsig);
		orBits(PPsig, Psig);				//merge Psig and PPsig giving a new PPsig
		putBits(p, pageNitems(p)-1, PPsig);	//put it back to the old position
		putPage(r->psigf, pSigId, p);
		freeBits(PPsig);
	}
	

	/* compute tuple signature and add to tsigf */
	tempBits = makeTupleSig(r, t);
	pid = rp->tsigNpages -1;
	pForTsig = getPage(r->tsigf, pid);
	// check if room on last page; if not add new page
	if (pageNitems(pForTsig) == rp->tsigPP) {
		addPage(r->tsigf);
		rp->tsigNpages++;
		pid++;
		free(pForTsig);
		//pForTsig = getPage(r->tsigf, pid);
		pForTsig = newPage();	
		if (p == NULL) return NO_PAGE;
	}
	putBits(pForTsig, pageNitems(pForTsig), tempBits);
	addOneItem(pForTsig);
	rp->ntsigs++;  //written to disk in closeRelation()
	putPage(r->tsigf, pid, pForTsig);	
	freeBits(tempBits);
	// compute page signature and add to psigf
	// finish above
	//TODO
	
	// use page signature to update bit-slices
	pid = rp->npages-1;
	tempBits = newBits(rp->bm);				
	for (i=0; i<rp->pm; i++){
		if (bitIsSet(Psig, i) == TRUE){
			//tempBits = newBits(rp->bm);
			*indexInBslice = 0;						//find the index of bitslice in the page
			//bSliceID = findBsilcePage(r, i, indexInBslice);
			bSliceID = i / rp->bsigPP;
			*indexInBslice = i - bSliceID  * rp->bsigPP;
			assert(*indexInBslice >= 0);
			
			p = getPage(r->bsigf, bSliceID);
			assert(*indexInBslice < pageNitems(p));
			getBits(p, *indexInBslice, tempBits);	//get the bitslice
			setBit(tempBits, pid);
			putBits(p, *indexInBslice, tempBits);
			putPage(r->bsigf, bSliceID, p);
			//freeBits(tempBits);
		}
	}
	freeBits(tempBits);
	freeBits(Psig);
	free(indexInBslice);
	//TODO
	return nPages(r)-1;
}

// displays info about open Reln (for debugging)

void relationStats(Reln r)
{
	RelnParams *p = &(r->params);
	printf("Global Info:\n");
	printf("Dynamic:\n");
    printf("  #items:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
			p->ntups, p->ntsigs, p->npsigs, p->nbsigs);
    printf("  #pages:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
			p->npages, p->tsigNpages, p->psigNpages, p->bsigNpages);
	printf("Static:\n");
    printf("  tups   #attrs: %d  size: %d bytes  max/page: %d\n",
			p->nattrs, p->tupsize, p->tupPP);
	printf("  sigs   %s",
            p->sigtype == 'c' ? "catc" : "simc");
    if (p->sigtype == 's')
	    printf("  bits/attr: %d", p->tk);
    printf("\n");
	printf("  tsigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->tm, p->tsigSize, p->tsigPP);
	printf("  psigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->pm, p->psigSize, p->psigPP);
	printf("  bsigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->bm, p->bsigSize, p->bsigPP);
}
