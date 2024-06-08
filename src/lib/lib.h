#ifndef SEARCH_LIB
#define SEARCH_LIB

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "../hash_map/hashmap.h"
#include "../dynarray/dynarray.h"

struct QueryResult {
    char *key;
    double score;
};

struct QueryResults {
    size_t length;
    size_t capacity;
    struct QueryResult *results;
};


struct Term {
    size_t count;
    char *key;
};

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
void free_corpus(struct hashmap *corpus);

void _add_or_update_document(struct hashmap *corpus, char *key, char **terms, size_t term_count);
void add_or_update_document(struct hashmap *corpus, char *key, char *content);
bool remove_document(struct hashmap *corpus, char *key);
// void update_document(struct hashmap *corpus, char *key, char **terms, size_t term_count);
size_t get_corpus_size(struct hashmap *corpus);
void copy_string(char *src, char **dest);
size_t get_doc_freq_for_term(struct hashmap *corpus, char *term);
size_t get_tfidf_for_term(struct hashmap *corpus, char *term);

/*
 *  searching a string inside a given corpus.
 *  returning the n most relevant documents
 */
struct QueryResults *search_query(struct hashmap *corpus, char *search_query);

void arr_free(struct QueryResults *a);
struct QueryResults *arr_init();
void arr_push(struct QueryResults *a, struct QueryResult qr);
#endif
