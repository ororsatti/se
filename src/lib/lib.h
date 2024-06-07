#ifndef SEARCH_LIB
#define SEARCH_LIB

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../hash_map/hashmap.h"

struct Term {
    size_t freq;
    char *key;
};

struct Document{
    // char *name;
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

void add_document(struct hashmap *corpus, char *key, char **terms, size_t term_count);
void remove_document(struct hashmap *corpus, char *key);
void update_document(struct hashmap *corpus, struct Document new_doc);
size_t get_corpus_size(struct hashmap *corpus);
void copy_string(char *src, char **dest);
size_t get_doc_freq_for_term(struct hashmap *corpus, char *term);
size_t get_tfidf_for_term(struct hashmap *corpus, char *term);

/*
 *  searching a string inside a given corpus.
 *  returning the n most relevant documents
 */
struct Document *search_query(struct hashmap **corpus, char **search_terms, int n);

#endif
