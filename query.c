// query.c ... query scan functions
// part of SIMC signature files
// Manage creating and using Query objects
// Written by John Shepherd, September 2018

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q)
{
	if (*q == '\0') return 0;
	char *c;
	int nattr = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nattr++;
	return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs)
{
	Query new = malloc(sizeof(QueryRep));
	assert(new != NULL);
	if (!checkQuery(r,q)) return NULL;
	new->rel = r;
	new->qstring = q;
	new->nsigs = new->nsigpages = 0;
	new->ntuples = new->ntuppages = new->nfalse = 0;
	new->pages = newBits(nPages(r));
	switch (sigs) {
	case 't': findPagesUsingTupSigs(new); break;
	case 'p': findPagesUsingPageSigs(new); break;
	case 'b': findPagesUsingBitSlices(new); break;
	default:  setAllBits(new->pages); break;
	}
	new->curpage = 0;
	return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void scanAndDisplayMatchingTuples(Query q)
{
	assert(q != NULL);
	Bits pages = q->pages;
	Reln r = q->rel;
	File dataF = dataFile(r);
	int matches;
	int n = getNumBits(pages);
	for(int i = 0; i < n; i++) {
		q->curpage = i;
		if(!bitIsSet(pages,i)) {
			continue;
		}
		Page p = getPage(dataF,q->curpage);
		q->ntuppages++;
		for(int j = 0; j < pageNitems(p); j++) {
			q->curtup = j;
			Tuple t = getTupleFromPage(r,p,q->curtup);
			q->ntuples++;
			//check if tuple T == q->qstring
			printf("checking tuple: %d\n",j);
			if(tupleStringcmp(r,t,q->qstring)) {
				showTuple(r,t);
				matches++;
			}
		}
		if(matches == 0) {
			q->nfalse++;
		}
	}
}

Bool tupleStringcmp(Reln r, Tuple t, char *qstring) {
	char **tupVals = tupleVals(r,t);
    char *pt;
    char temp[strlen(qstring)];
    strcpy(temp,qstring);
    int i = 0;
    pt = strtok (temp,",");
    while (pt != NULL) {
    	char *val = tupVals[i];
    	if(strcmp(pt,"?") != 0) {
    		if(strcmp(val,pt) != 0) {
    			return FALSE;
    		}
    	}
        pt = strtok (NULL, ",");
        i++;
    }
	return TRUE;
}


// print statistics on query

void queryStats(Query q)
{
	printf("# signatures read:   %d\n", q->nsigs);
	printf("# sig pages read:    %d\n", q->nsigpages);
	printf("# tuples examined:   %d\n", q->ntuples);
	printf("# data pages read:   %d\n", q->ntuppages);
	printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q->pages);
	free(q);
}

