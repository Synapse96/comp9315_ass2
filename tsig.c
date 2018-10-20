// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

Bits codeword(char *attr_value, int m, int k);

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	Bits tsig = newBits(tsigBits(r));
	char **tupVals = tupleVals(r, t);
	for (int i = 0; i < nAttrs(r); i++) {
		char *tupVal = tupVals[i];
		if (strcmp(tupVal, "?") != 0) {
			Bits cw = codeword(tupVal, tsigBits(r), codeBits(r));
			orBits(tsig, cw);
		}
	}
	//printf("tsig = "); showBits(tsig); printf("\n");
	return tsig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	Reln r = q->rel;
	Bits qsig = makeTupleSig(r, q->qstring);
	unsetAllBits(q->pages);
	File tsigs = tsigFile(r);
	//Bits matches = newBits(nPages(r));
	for (int i = 0; i < nTsigPages(r); i++) {
		Page p = getPage(tsigs, i);
		for (int j = 0; j < pageNitems(p); j++) {
			Bits tsig = newBits(tsigBits(r));
			getBits(p, j, tsig);
			//printf("qsig = "); showBits(qsig); printf("\n");
			//printf("tsig = "); showBits(tsig); printf("\n");
			if (isSubset(qsig, tsig)) {
				setBit(q->pages, i);
				//printf("they match!\n");
			}
			q->nsigs++;
		}
		q->nsigpages++;
	}
	
	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

Bits codeword(char *attr_value, int m, int k)
{
	int nbits = 0; // count of set bits
	Bits cword = newBits(m); // assuming m <= 32 bits
	srandom(hash_any(attr_value, k));
	while (nbits < k) {
		int i = random() % m;
		if (!(bitIsSet(cword, i))) {
	    	setBit(cword, i);
	    	nbits++;
	    }
	}
	return cword; // m-bits with k 1-bits and m-k 0-bits
}
