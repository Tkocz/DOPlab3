/*
 * File: symtab.c
 * --------------
 * This file implements the symbol table abstraction.
 */

#include <stdio.h>
#include "genlib.h"
#include "strlib.h"
#include "symtab.h"

/*
 * Constants
 * ---------
 * NBuckets -- Number of buckets in the hash table
 */

#define NBuckets 101

/*
 * Type: cellT
 * -----------
 * This type defines a linked list cell for the symbol table.
 */

typedef struct cellT {
    string key;
    void *value;
    struct cellT *link;
} cellT;

/*
 * Type: symtabCDT
 * ---------------
 * This type defines the underlying concrete representation for a
 * symtabADT.  These details are not relevant to and therefore
 * not exported to the client.  In this implementation, the
 * underlying structure is a hash table organized as an array of
 * "buckets," in which each bucket is a linked list of elements
 * that share the same hash code.
 */

struct symtabCDT {
    cellT *buckets[NBuckets];
};

/* Private function declarations */

static void FreeBucketChain(cellT *cp);
static cellT *FindCell(cellT *cp, string s);
static int Hash(string s, int nBuckets);

/* Public entries */

symtabADT NewSymbolTable(void)
{
    symtabADT table;
    int i;

    table = New(symtabADT);
    for (i = 0; i < NBuckets; i++) {
        table->buckets[i] = NULL;
    }
    return (table);
}

void FreeSymbolTable(symtabADT table)
{
    int i;

    for (i = 0; i < NBuckets; i++) {
        FreeBucketChain(table->buckets[i]);
    }
    FreeBlock(table);
}

void Enter(symtabADT table, string key, void *value)
{
    int bucket;
    cellT *cp;

    bucket = Hash(key, NBuckets);
    cp = FindCell(table->buckets[bucket], key);
    if (cp == NULL) {
        cp = New(cellT *);
        cp->key = CopyString(key);
        cp->link = table->buckets[bucket];
        table->buckets[bucket] = cp;
    }
    cp->value = value;
}

void *Lookup(symtabADT table, string key)
{
    int bucket;
    cellT *cp;

    bucket = Hash(key, NBuckets);
    cp = FindCell(table->buckets[bucket], key);
    if (cp == NULL) return(UNDEFINED);
    return (cp->value);
}

/*
 * Implementation notes: DeleteSymbol
 * ----------------------------------
 * The implementation of DeleteSymbol must search the hash
 * chain for an entry with a matching key and then delete that
 * entry from the chain.  To delete the cell, the code must have
 * a pointer to the preceding cell, which is maintained in pp.
 * The first cell in the list is a special case because there
 * is no previous cell; this case is identified by having NULL
 * as the value of pp.
 */

void DeleteSymbol(symtabADT table, string key)
{
    int bucket;
    cellT *cp, *pp;

    bucket = Hash(key, NBuckets);
    pp = NULL;
    cp = table->buckets[bucket];
    while (cp != NULL && !StringEqual(cp->key, key)) {
        pp = cp;
        cp = cp->link;
    }
    if (cp == NULL) return;
    if (pp == NULL) {
        table->buckets[bucket] = cp->link;
    } else {
        pp->link = cp->link;
    }
    FreeBlock(cp);
}

void MapSymbolTable(symtabFnT fn, symtabADT table,
                    void *clientData)
{
    int i;
    cellT *cp;

    for (i = 0; i < NBuckets; i++) {
        for (cp = table->buckets[i]; cp != NULL; cp = cp->link) {
            fn(cp->key, cp->value, clientData);
        }
    }
}

/* Private functions */

/*
 * Function: FreeBucketChain
 * Usage: FreeBucketChain(cp);
 * ---------------------------
 * This function takes a chain pointer and frees all the cells
 * in that chain.  Because the package makes copies of the keys,
 * this function must free the string storage as well.
 */

static void FreeBucketChain(cellT *cp)
{
    cellT *next;

    while (cp != NULL) {
        next = cp->link;
        FreeBlock(cp->key);
        FreeBlock(cp);
        cp = next;
    }
}

/*
 * Function: FindCell
 * Usage: cp = FindCell(cp, key);
 * ------------------------------
 * This function finds a cell in the chain beginning at cp that
 * matches key.  If a match is found, a pointer to that cell is
 * returned.  If no match is found, the function returns NULL.
 */

static cellT *FindCell(cellT *cp, string key)
{
    while (cp != NULL && !StringEqual(cp->key, key)) {
        cp = cp->link;
    }
    return (cp);
}

/*
 * Function: Hash
 * Usage: bucket = Hash(key, nBuckets);
 * ------------------------------------
 * This function takes the key and uses it to derive a hash code,
 * which is an integer in the range [0, nBuckets - 1].  The hash
 * code is computed using a method called linear congruence.  The
 * choice of the value for Multiplier can have a significant effect
 * on the performance of the algorithm, but not on its correctness.
 */

#define Multiplier -1664117991L

static int Hash(string s, int nBuckets)
{
    int i;
    unsigned long hashcode;

    hashcode = 0;
    for (i = 0; s[i] != '\0'; i++) {
        hashcode = hashcode * Multiplier + s[i];
    }
    return (hashcode % nBuckets);
}
