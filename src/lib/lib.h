#ifndef SEARCH_LIB
#define SEARCH_LIB

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "hashmap/hashmap.h"
#include "dynarray/dynarray.h"

// Contains the score and the key of a given Document
struct QueryResult {
    char *key;
    double score;
};

// Contains the count of how many
// times this term exists inside a given Document
struct Term {
    size_t count;
    char *key;
};

// Containes all the terms in its content
// and the key as a refrence to the original
// content ID.
struct Document{
    char *key;
    // map of struct Term
    struct hashmap *terms;
};

/*
 * struct Corpus = hashmap<Document>
 * allocating a hashmap for the corpus
*/
struct hashmap *init_corpus();

// free the memory of a given corpus
void free_corpus(struct hashmap *corpus);

// add or update a given document. 
// in the future we will have a more efficient update function
void add_or_update_document(struct hashmap *corpus, char *key, char *content);

// removes a document from a given corpus
bool remove_document(struct hashmap *corpus, char *key);

// gets the corpus size (count of the documents in it)
size_t get_corpus_size(struct hashmap *corpus);

// searches a given query inside the corpus.
// return an array of QueryResult in an unordered fasion.
// will only return keys of relevant documents, a document
// that is irrelevant to the search_query will be left out.
struct QueryResult *search_query(struct hashmap *corpus, char *search_query);

// free the query result list
void free_query_results(struct QueryResult *qrs, size_t len);
#endif
