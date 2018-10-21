// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);
	Reln r = q->rel;
	Bits qsig = makePageSig(r, q->qstring);
	setAllBits(q->pages);
	for (int i = 0; i < psigBits(r); i++) {
		if (bitIsSet(qsig, i)) {
			PageID pid = (i != 0) ? ((i-1) / maxBsigsPP(r)) : (i / maxBsigsPP(r));
			Page p = getPage(r->bsigf, pid);
			Bits bsig = newBits(bsigBits(r));
			int b_i = (i != 0) ? ((i-1) % maxBsigsPP(r)) : (i % maxBsigsPP(r));
			getBits(p, b_i, bsig);
			for (int j = 0; j < nPsigs(r); j++) {
				if (!bitIsSet(bsig, j)) {
					unsetBit(q->pages, j);
				}
			}
		}
	}
}
