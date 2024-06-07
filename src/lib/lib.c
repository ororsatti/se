#include "lib.h"
#include <stdlib.h>
#include <string.h>

#define CORPUS_EL_SIZE sizeof(struct Document)
#define TERM_EL_SIZE sizeof(struct Term)

int _compare(const void *a, const void *b, void *udata) {
    const struct Document *da = a;
    const struct Document *db = b;
    return strcmp(da->key, db->key);
}

uint64_t _hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct Document *d = item;
    return hashmap_sip(d->key, strlen(d->key), seed0, seed1);
}

void _free(void *item) {
    struct Document *d = item;
    free(d->key);
    hashmap_free(d->terms);
}


void copy_string(char *src, char **dest) {
    char *tmp = malloc(strlen(src));
    strcpy(tmp, src);
    *dest = tmp;
}


int term_compare(const void *a, const void *b, void *udata) {
    const struct Term *ta = a;
    const struct Term *tb = b;
    return strcmp(ta->key, tb->key);
}

uint64_t term_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct Term *t = item;
    return hashmap_sip(t->key, strlen(t->key), seed0, seed1);
}

void term_free(void *item) {
    struct Term *t = item;
    free(t->key);
}

struct hashmap *init_terms_map() {
    struct hashmap *terms_map =
        hashmap_new(TERM_EL_SIZE, 0, 0, 0, term_hash,term_compare, term_free, NULL);
    return terms_map;
}

// add or update the term in the term map
// if you pass it a term that already exists in
// the map, it will increase the freq variable by one
// and update the map
void add_term_to_map(struct hashmap *term_map, char *t) {
    const struct Term * term = hashmap_get(term_map, &(struct Term){ .key = t });
    struct Term tmp = {
        .key = term ? term->key : malloc(strlen(t)),
        .freq = term ? term->freq + 1 : 1
    };
    if(!term) {
        strcpy(tmp.key, t);
    }
    hashmap_set(term_map, &tmp);
}


// ######## API ############



// initiating the corpus map
struct hashmap *init_corpus() {
    struct hashmap *corpus =
        hashmap_new(CORPUS_EL_SIZE, 0, 0, 0, _hash,_compare, _free, NULL);
    return corpus;
}

void add_document(struct hashmap *corpus, char *key, char **terms, size_t term_count) {
    struct Document d = {
        .terms = init_terms_map(),
        .key = malloc(strlen(key))
    };

    copy_string(key, &d.key);

    for (size_t i = 0; i < term_count; ++i) {
        add_term_to_map(d.terms, terms[i]);
    }

    hashmap_set(corpus, &d);
}
void remove_document(struct hashmap *corpus, char *key);
void update_document(struct hashmap *corpus, struct Document new_doc);
size_t get_corpus_size(struct hashmap *corpus);

size_t get_doc_freq_for_term(struct hashmap *corpus, char *term);
size_t get_tfidf_for_term(struct hashmap *corpus, char *term);

/*
 *  searching a string inside a given corpus.
 *  returning the n most relevant documents
 */
struct Document *search_query(struct hashmap **corpus, char **search_terms, int n);

