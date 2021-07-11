// bits.c ... functions on bit-strings
// part of signature indexed files
// Bit-strings are arbitrarily long byte arrays
// Least significant bits (LSB) are in array[0]
// Most significant bits (MSB) are in array[nbytes-1]

// Written by John Shepherd, March 2019

#include <assert.h>
#include "defs.h"
#include "bits.h"
#include "page.h"

typedef struct _BitsRep {
	Count  nbits;		  // how many bits
	Count  nbytes;		  // how many bytes in array
	Byte   bitstring[1];  // array of bytes to hold bits
	                      // actual array size is nbytes
} BitsRep;

void convert_bytes_to_bits(Bits b, char *string);
void left_shift(Bits b, int n);

//left shift bitstring
void left_shift(Bits b, int n){
	/*
	int i, len, base, sum, pos_in_string;
	char *temp = malloc(sizeof(char) * (b->nbytes * 8 + 1));
	convert_bytes_to_bits(b, temp);
	len = strlen(temp);
	i = b->nbytes * 8 -b->nbits;			//ignore the padding digits
	for (;i<len; i++){
		if (i + n >= b->nbytes * 8){		//use 0 to pad
			temp[i] = '0';
		}
		else{
			temp[i] = temp[i+n];			//left shift
		}
	}
	temp[i] = '\0';
	//printf("temp %s\n",temp);
	base = 128;
	sum = 0;
	pos_in_string = b->nbytes -1;		
	for (i=0; i<len; i++){
		if (temp[i] == '1'){
			sum+=base;
		}
		//printf("i is %d, sum is%d, base is %d\n",i, sum, base);
		base >>= 1;
		
		if ((i+1) % 8 == 0){
			base = 128; 						//reset the base to 128
			b->bitstring[pos_in_string] = sum;
			//printf("i is %d, sum is%d, pos is %d\n",i, sum, pos_in_string);
			sum = 0;
			pos_in_string--;
		}
	}
	//b->bitstring[pos_in_string] = sum;			//for the last byte
	free(temp);	
	*/
	int i;
	for (i = b->nbits -1; i>= n; i--){
		if (bitIsSet(b, i-n) == TRUE){
			setBit(b, i);
		}
		else unsetBit(b, i);
	}
	for (i=0; i<n; i++){
		unsetBit(b, i);
	}
}

//right shift bitstring
void right_shift(Bits b, int n){
	/*
	int i, len, base, sum, pos_in_string;
	char *temp = malloc(sizeof(char) * (b->nbytes * 8 + 1));
	convert_bytes_to_bits(b, temp);
	len = strlen(temp);
	i = len - 1;			//start from right
	for (;i >=0; i--){
		if (i - n < 0){		//use 0 to pad
			temp[i] = '0';
		}
		else{
			temp[i] = temp[i-n];			
		}
	}
	//printf("temp %s\n",temp);
	base = 128;
	sum = 0;
	pos_in_string = b->nbytes -1;		
	for (i=0; i<len; i++){
		if (temp[i] == '1'){
			sum+=base;
		}
		base >>= 1;
		
		if ((i+1) % 8 == 0){
			base = 128; 						//reset the base to 128
			b->bitstring[pos_in_string] = sum;
			//printf("i is %d, sum is%d, pos is %d\n",i, sum, pos_in_string);
			sum = 0;
			pos_in_string--;
		}
	}
	free(temp);	
	*/
	int i;
	for (i = 0; i< b->nbits - n; i++){
		if (bitIsSet(b, i+n) == TRUE){
			setBit(b, i);
		}
		else unsetBit(b, i);
	}
	for (i= b->nbits - 1; i>= b->nbits - n; i--){
		unsetBit(b, i);
	}	
}

//convert bytestring to bitstring
void convert_bytes_to_bits(Bits b, char *string){
	int i, j, k;
	char mask;
	for(i = b->nbytes -1; i>=0; i--){
		for(j=7; j>=0; j--){
			mask = (1<<j);
			if (b->bitstring[i] & mask){
				string[k] = '1';
			}
			else{
				string[k] = '0';
			}
			k++;
		}
	}
	string[k] = '\0';
}

// create a new Bits object

Bits newBits(int nbits)
{
	Count nbytes = iceil(nbits,8);
	Bits new = malloc(2*sizeof(Count) + nbytes);
	new->nbits = nbits;
	new->nbytes = nbytes;
	memset(&(new->bitstring[0]), 0, nbytes);
	return new;
}

// release memory associated with a Bits object

void freeBits(Bits b)
{
	//free(b->bitstring);
	free(b);
}

// check if the bit at position is 1

Bool bitIsSet(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	/*
	int real_index;
	char *temp = malloc(sizeof(char) * (b->nbytes * 8 + 1));
	convert_bytes_to_bits(b, temp);
	real_index = b->nbytes * 8 - 1 - position;
	if (temp[real_index] == '1'){
		free(temp);
		return TRUE;
	}
	free(temp);
	*/
	int firstIndex, secondIndex;
	firstIndex = position / 8;
	secondIndex = position - firstIndex * 8;
	Byte number = b->bitstring[firstIndex];
	int mask = 1 << secondIndex;
	if (number & mask) {
		return TRUE;
	}
	return FALSE;
}

// check whether one Bits b1 is a subset of Bits b2

Bool isSubset(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	int i;
	Byte a, b;
	/*
	char *temp1, *temp2;
	temp1 = malloc(sizeof(char) * (b1->nbytes * 8 + 1));
	temp2 = malloc(sizeof(char) * (b2->nbytes * 8 + 1));
	convert_bytes_to_bits(b1, temp1);
	convert_bytes_to_bits(b2, temp2);
	for(i = 0; i < b1->nbytes * 8; i++){
		if (temp1[i] == '1'){
			if (temp2[i] != '1'){
				free(temp1);
				free(temp2);
				return FALSE;
			}
		}
	}
	free(temp1);
	free(temp2);
	return TRUE;
	*/
	/*reference from https://stackoverflow.com/questions/8639396/verify-if-a-binary-array-is-a-subset-of-another-one-in-c */
	for (i=0; i<b1->nbytes; i++){
		a = b1->bitstring[i];
		b = b2->bitstring[i];
		if ((a|b) != b) return FALSE;
	}
	return TRUE;
}

// set the bit at position to 1

void setBit(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	int pos_in_string, which_digit;
	//position += b->nbytes * 8 - b->nbits;						//padding the offset of 64 digit
	if (position == 0){
		b->bitstring[0] |= 1;
		return;
	}
	pos_in_string = position / 8;
	which_digit = position % 8;
	b->bitstring[pos_in_string] |= (1 << which_digit);		//use bitwise OR operator
}

// set all bits to 1

void setAllBits(Bits b)
{
	assert(b != NULL);
	int i;
	/*special for the bitstring[b->nbytes - 1]*/
	//int last_byte;
	//i = b->nbytes * 8 - b->nbits;								//padding the offset of 64 digit
	//last_byte = 255 >> i;

	for (i=0; i<b->nbytes; i++){
		b->bitstring[i] |= 255;
	}

	//b->bitstring[b->nbytes -1] |= last_byte;
}

// set the bit at position to 0

void unsetBit(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	int pos_in_string, which_digit;
	/*
	if (position == 0){
		b->bitstring[0] &= 254;
		return;
	}
	*/
	pos_in_string = position / 8;
	which_digit = position % 8;
	/* reference from https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit*/
	b->bitstring[pos_in_string] &= ~(1 << which_digit);		//invert the bit string and bitwise NOT operator
}

// set all bits to 0

void unsetAllBits(Bits b)
{
	assert(b != NULL);
	int i;
	for (i=0; i<b->nbytes; i++){
		b->bitstring[i] &= 0;
	}
}

// bitwise AND ... b1 = b1 & b2

void andBits(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	int i, temp;
	for (i=0; i<b1->nbytes; i++){
		temp = b2->bitstring[i];
		b1->bitstring[i] &= temp;
	}
}

// bitwise OR ... b1 = b1 | b2

void orBits(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	int i, temp;
	for (i=0; i<b1->nbytes; i++){
		temp = b2->bitstring[i];
		b1->bitstring[i] |= temp;
	}
}

// left-shift ... b1 = b1 << n
// negative n gives right shift

void shiftBits(Bits b, int n)
{
    //int i;
	if (n == 0) return;
	else if (n > 0){
		/*
		for(i=0; i<8; i++){
			printf("%d ",b->bitstring[i]);
		}
		printf("\n");
		*/
		left_shift(b, n);
		/*
		for(i=0; i<8; i++){
			printf("%d ",b->bitstring[i]);
		}
		printf("\n");	
		*/	
	}
	else{
		n *= -1;
		right_shift(b, n);
	}
}

// get a bit-string (of length b->nbytes)
// from specified position in Page buffer
// and place it in a BitsRep structure

void getBits(Page p, Offset pos, Bits b)
{
	//int i;
	Count number;
	Byte *string;
	number = pageNitems(p);
	//string = malloc(b->nbytes +1);
	assert(pos <= number);			//index can't be larger than nitems in page
	assert(pos >= 0);				//index should be larger than 0
	string = addrInPage(p, pos, b->nbytes);					//get the item from page
	memcpy(b->bitstring, string, b->nbytes);
	/*
	for (i=0; i<b->nbytes; i++){
		b->bitstring[i] = *(string+i);
	}
	*/
}

// copy the bit-string array in a BitsRep
// structure to specified position in Page buffer

void putBits(Page p, Offset pos, Bits b)
{
	Byte *string;
	assert(pos >=0);
	//assert((pos + 1) * b->nbytes <= PAGESIZE - sizeof(int));
	string = addrInPage(p, pos, b->nbytes);
	memcpy(string, b->bitstring, b->nbytes);
	/*
	for (i=0; i<b->nbytes; i++){
		*(string + i) = b->bitstring[i];
	}
	*/
	//addOneItem(p);
}

// show Bits on stdout
// display in order MSB to LSB
// do not append '\n'

void showBits(Bits b)
{
	assert(b != NULL);
    //printf("(%d,%d)",b->nbits,b->nbytes);
	for (int i = b->nbytes-1; i >= 0; i--) {
		for (int j = 7; j >= 0; j--) {
			Byte mask = (1 << j);
			if (b->bitstring[i] & mask)
				putchar('1');
			else
				putchar('0');
		}
	}
}
