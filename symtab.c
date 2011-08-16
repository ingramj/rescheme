#include "rescheme.h"

#include <assert.h>
#include <string.h>

struct rs_symtab_entry {
	struct rs_symtab_entry *next;
	const char *sym;
	int count;
};

#define _SYMTAB_SIZE 1439
static struct rs_symtab_entry *rs_symtab[_SYMTAB_SIZE];


static struct rs_symtab_entry *rs_symtab_lookup(const char *sym);
static unsigned long rs_symtab_hash(const char *sym);


const char *rs_symtab_insert(const char *sym)
{
	assert(sym != NULL);

	struct rs_symtab_entry *entry = rs_symtab_lookup(sym);
	if (entry != NULL) {
		entry->count++;
		if (entry->count < 0) {
			rs_fatal("refcount overflow for symbol \"%s\"", sym);
		}
		return entry->sym;
	}

	entry = malloc(sizeof(struct rs_symtab_entry));
	if (entry == NULL) {
		rs_fatal("could not allocate symbol table entry:");
	}

	unsigned long hashval = rs_symtab_hash(sym);
#ifdef DEBUG
	if (rs_symtab[hashval] != NULL) {
		TRACE("collision while inserting symbol \"%s\" (%lu)",
		      sym, hashval);
	}
#endif

	entry->sym = strdup(sym);
	if (entry->sym == NULL) {
		rs_fatal("could not copy symbol value:");
	}
	entry->count = 1;
	entry->next = rs_symtab[hashval];
	rs_symtab[hashval] = entry;

	return entry->sym;
}


void rs_symtab_remove(const char *sym)
{
	assert(sym != NULL);

	unsigned long hashval = rs_symtab_hash(sym);
	struct rs_symtab_entry *tmp, *entry = rs_symtab[hashval];
	if (entry == NULL) {
		rs_nonfatal("symbol \"%s\" is not in the table", sym);
		return;
	}

	assert(entry->sym != NULL);
	assert(entry->count > 0);
	if (strcmp(entry->sym, sym) == 0) {
		entry->count--;
		if (entry->count == 0) {
			rs_symtab[hashval] = entry->next;
			free((char *)entry->sym);
			free(entry);
		}
		return;
	}

	while (entry->next != NULL) {
		assert(entry->next->count > 0);
		if (strcmp(entry->next->sym, sym) == 0) {
			entry->next->count--;
			if (entry->next->count == 0) {
				tmp = entry->next;
				entry->next = entry->next->next;
				free((char *)tmp->sym);
				free(tmp);
			}
			return;
		}
		entry = entry->next;
	}
	rs_nonfatal("symbol \"%s\" is not in the table");
}


static struct rs_symtab_entry *rs_symtab_lookup(const char *sym)
{
	assert(sym != NULL);

	struct rs_symtab_entry *entry = rs_symtab[rs_symtab_hash(sym)];

	while (entry != NULL) {
		assert(entry->sym != NULL);
		assert(entry->count > 0);
		if (strcmp(entry->sym, sym) == 0) {
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}


static unsigned long rs_symtab_hash(const char *sym)
{
	assert(sym != NULL);

	unsigned long hashval;

	// DJB2 hash
	for (hashval = 5381; *sym != '\0'; sym++) {
		hashval = ((hashval << 5) + hashval) + (unsigned long)*sym;
	}

	return hashval % _SYMTAB_SIZE;
}
