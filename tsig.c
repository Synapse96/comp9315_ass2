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
	for (int i = 0; i < nTsigPages(r); i++) {
		Page p = getPage(tsigs, i);
		for (int j = 0; j < pageNitems(p); j++) {
			Bits tsig = newBits(tsigBits(r));
			getBits(p, j, tsig);
			if (isSubset(qsig, tsig)) {
				int tid = (((i * maxTsigsPP(r)) + j));
				int pid = (tid != 0) ? ((tid-1) / maxTupsPP(r)) : (tid / maxTupsPP(r));
				setBit(q->pages, pid);
			}
			q->nsigs++;
		}
		q->nsigpages++;
	}
	
	// The printf below is primarily for debugging
	// Remove it before submitting this function
	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

Bits codeword(char *attr_value, int m, int k)
{
	int nbits = 0; // count of set bits
	Bits cword = newBits(m); // assuming m <= 32 bits
	srandom(hash_any(attr_value, strlen(attr_value)));
	while (nbits < k) {
		int i = random() % m;
		if (!(bitIsSet(cword, i))) {
	    	setBit(cword, i);
	    	nbits++;
	    }
	}
	return cword; // m-bits with k 1-bits and m-k 0-bits
}
