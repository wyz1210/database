// Test Bits ADT

#include <stdio.h>
#include "defs.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"

int main(int argc, char **argv)
{
	Bits b = newBits(60);
	printf("t=0: "); showBits(b); printf("\n");
	setBit(b, 5);
	printf("t=1: "); showBits(b); printf("\n");
	setBit(b, 0);
	setBit(b, 50);
	setBit(b, 59);
	Bits a = newBits(60);
	//setBit(a, 40);
	//if (bitIsSet(a,50)) printf("Bit a 50 is set\n");
	setAllBits(a);
	printf("a=0: "); showBits(a); printf("\n");
	unsetBit(a, 0);
	printf("a=1: "); showBits(a); printf("\n");
	if (bitIsSet(a,0)) printf("Bit a 0 is set\n");
	setBit(a, 5);
	if (bitIsSet(a,5)) printf("Bit a 5 is set\n");
	//printf("a=0: "); showBits(a); printf("\n");
	if (isSubset(a,b)) printf("Bit a is subset of b\n");
	setBit(b,59);
	setBit(b,3);
	printf("t=2: "); showBits(b); printf("\n");
	
	shiftBits(b, -3);
	printf("t=3: "); showBits(b); printf("\n");
	if (bitIsSet(b,5)) printf("Bit 5 is set\n");
	if (bitIsSet(b,10)) printf("Bit 10 is set\n");
	setAllBits(b);
	printf("t=3: "); showBits(b); printf("\n");
	unsetBit(b, 40);
	
	printf("t=4: "); showBits(b); printf("\n");
	//setBit(a, 40);
	//printf("a=1: "); showBits(a); printf("\n");
	if (bitIsSet(b,20)) printf("Bit 20 is set\n");
	if (bitIsSet(b,40)) printf("Bit 40 is set\n");	
	if (bitIsSet(b,50)) printf("Bit 50 is set\n");
	unsetAllBits(b);
	/*
	printf("t=5: "); showBits(b); printf("\n");
	Bits a = newBits(60);
	unsetBit(a,40);
	orBits(b,a);
	printf("t=6: "); showBits(b); printf("\n");
	setAllBits(a);
	unsetBit(a,40);
	printf("t=8: "); showBits(a); printf("\n");
	shiftBits(a,19);
	printf("t=9: "); showBits(a); printf("\n");
	
	printf("t=5: "); showBits(b); printf("\n");
	setBit(b,0);
	setBit(b,1);
	printf("t=6: "); showBits(b); printf("\n");
	shiftBits(b,58);
	printf("t=7: "); showBits(b); printf("\n");
	*/
	return 0;
}
