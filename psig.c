// psig.c ... functions on page signatures (psig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"

Bits pagecodeword(char *attr_value, int m, int k);

Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	Bits psig = newBits(psigBits(r));
	char **tupVals = tupleVals(r, t);
	for (int i = 0; i < nAttrs(r); i++) {
		char *tupVal = tupVals[i];
		if (strcmp(tupVal, "?") != 0) {
			Bits cw = pagecodeword(tupVal, psigBits(r), codeBits(r));
			orBits(psig, cw);
		}
	}
	return psig;
}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	Reln r = q->rel;
	Bits qsig = makePageSig(r, q->qstring);
	unsetAllBits(q->pages);
	File psigs = psigFile(r);
	for (int i = 0; i < nPsigPages(r); i++) {
		Page p = getPage(psigs, i);
		for (int j = 0; j < pageNitems(p); j++) {
			Bits psig = newBits(psigBits(r));
			getBits(p, j, psig);
			if (isSubset(qsig, psig)) {
				setBit(q->pages, i);
			}
			q->nsigs++;
		}
		q->nsigpages++;
	}
	
	// The printf below is primarily for debugging
	// Remove it before submitting this function
	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

Bits pagecodeword(char *attr_value, int m, int k)
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
