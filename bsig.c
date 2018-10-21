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
	Bits qsig = makePageSig(q->rel,q->qstring);
	Bits pages = q->pages; 
	Reln r = q->rel;
	Page p = getPage(bsigFile(r),nBsigPages(r) - 1);
	printf("pages bits = "); showBits(pages); printf("\n");
	setAllBits(pages);
	for(int i = 0; i < psigBits(r); i++) {
		if(bitIsSet(qsig,i)) {
			Bits slice = newBits(bsigBits(r));
			getBits(p,i,slice);	
			printf("slice bits = "); showBits(slice); printf("\n");
			for(int j = 0; j < bsigBits(r); j++) {
				printf("checking bit = %d\n",j);
				if(!bitIsSet(slice,j)) {
					unsetBit(pages,j);
				}
			}
		}
	}
}

