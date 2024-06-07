#include "lib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h>

#define CORPUS_EL_SIZE sizeof(struct Document)
#define TERM_EL_SIZE sizeof(struct Term)
#define K 1.2
#define B 0.75

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
        // allocating memory for the key
        .key = term ? term->key : malloc(strlen(t)),
        // setting the count
        .count = term ? term->count + 1 : 1
    };
    if(!term) {
        strcpy(tmp.key, t);
    }
    hashmap_set(term_map, &tmp);
}

size_t get_doc_size(struct Document *d) {
    return hashmap_count(d->terms);
}

double get_corpus_avg_doc_size(struct hashmap *corpus) {
    size_t i = 0;
    double acc = 0,
        corpus_size = get_corpus_size(corpus);
        
    void *item = NULL;

    while (hashmap_iter(corpus, &i, &item)) {
        struct Document *doc = item;
        size_t size = get_doc_size(doc);
        acc += size;
    }

    return acc / corpus_size;
}

size_t get_doc_freq_for_term(struct hashmap *corpus, char *term) {
    size_t i = 0, count = 0;
    void *item = NULL;
    while (hashmap_iter(corpus, &i, &item)) {
        struct Document *doc = item;
        const struct Term *t = hashmap_get(doc->terms, &(struct Term){ .key = term });
        t && count++;
    }
    return count;
}

/*
 * IDF(t) = ln (((N - DF + 0.5) / (DF + 0.5)) + 1)
 * N - total # of documents
 * DF - how many documents containing the term t;
 * ln - natural log
 * */
double calc_idf(struct hashmap *corpus, char *term) {
    size_t docs_count = get_corpus_size(corpus);
    size_t df = get_doc_freq_for_term(corpus, term);
    double numerator = docs_count - df + 0.5;
    double denominator = df + 0.5;

    return log(1 + (numerator / denominator));
}

/*
 *  BM25(D,Q) = sum(IDF(q[i]) * TF(D, q[i]) * (K + 1)
 *  / 
 *  TF(D, q[i]) + K * (1 - b + b * Dlen/avgD))
 *
 *  D - document
 *  Q - query (list of terms)
 *  K, B - constants
 *  IDF - the inverse document frequency
 *  TF - how many instances of a term exist in D
 *  Dlen - the length of D
 *  avgD - the average length of a document
 * */
double get_bm25_for_doc(struct hashmap *corpus, struct Document *d, char **terms, size_t terms_count) {
    double score = 0;
    double doc_size = get_doc_size(d);
    for (size_t i = 0; i < terms_count; ++i) {
        const struct Term *t = hashmap_get(d->terms, &(struct Term){ .key = terms[i] });

        if(!t) continue;

        double idf = calc_idf(corpus, terms[i]);
        double inner = doc_size / get_corpus_avg_doc_size(corpus);
        double numinator = t->count * (K + 1);
        double denominator = t->count + K * (1 - B + B * inner);
        score += idf * (numinator / denominator);
    }
    return score;
}


// ######## API ############

// initiating the corpus map
struct hashmap *init_corpus() {
    struct hashmap *corpus =
        hashmap_new(CORPUS_EL_SIZE, 0, 0, 0, _hash,_compare, _free, NULL);
    return corpus;
}

// adds a document to the corpus.
void add_or_update_document(struct hashmap *corpus, char *key, char **terms, size_t term_count) {
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

// remove a document from the corpus.
// returns true if succeed, false if item not found
bool remove_document(struct hashmap *corpus, char *key) {
    return (bool)hashmap_delete(corpus, &(struct Document){ .key = key });
}

size_t get_corpus_size(struct hashmap *corpus) {
    return hashmap_count(corpus);
}

/*
 *  searching a string inside a given corpus.
 *  returning the n most relevant documents
 */
char *search_query(struct hashmap *corpus, char **search_terms, size_t st_count, int n) {
    // char *best[n];
    // memset(best, NULL, n);
    size_t i = 0;
    void *item = NULL;
    char *best = NULL;
    double max = 0;

    while(hashmap_iter(corpus, &i, &item)) {
        struct Document *d = item;
        double score = get_bm25_for_doc(corpus, d, search_terms, st_count);
        if(max < score) { 
            best = d->key;
            max = score;
        }
    }
    return best;
}
