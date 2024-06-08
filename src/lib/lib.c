#include "lib.h"
#include <ctype.h>
#include <string.h>

#define CORPUS_EL_SIZE sizeof(struct Document)
#define TERM_EL_SIZE sizeof(struct Term)
#define QUERY_RESULT_SIZE sizeof(struct QueryResult)
#define K 1.2
#define B 0.75
#define DEFAULT_DA_CAPACITY 100

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


void arr_push(struct QueryResults *a, struct QueryResult qr) {
    struct QueryResult *tmp;
    if(a->length == a->capacity) {
        tmp = realloc(a->results,(a->capacity + DEFAULT_DA_CAPACITY) * QUERY_RESULT_SIZE);
        if(tmp == NULL) {
            return;
        }
        a->capacity += DEFAULT_DA_CAPACITY;
        a->results = tmp;
    }
    a->results[a->length] = qr;
    a->length++;
}

struct QueryResults *arr_init() {
    struct QueryResults *a = malloc(sizeof(struct QueryResults));
    a->capacity = DEFAULT_DA_CAPACITY; 
    a->length = 0;
    a->results = malloc(a->capacity * QUERY_RESULT_SIZE);
    return a;
}

void arr_free(struct QueryResults *a) {
    for (size_t i = 0; i < a->length; ++i) {
        free(a->results[i].key);
    }
    free(a->results);
    free(a);
};

void _add_or_update_document(struct hashmap *corpus, char *key, char **terms, size_t term_count) {
    struct Document d = {
        .terms = init_terms_map(),
    };

    copy_string(key, &d.key);

    for (size_t i = 0; i < term_count; ++i) {
        add_term_to_map(d.terms, terms[i]);
    }

    hashmap_set(corpus, &d);
}

void *_add_char_to_string(char *str, char c){
    char *new_str = NULL;
    if(str == NULL) {
        new_str = realloc(str, sizeof(c));
        new_str[0] = c;
        return new_str;
    }

    size_t size = strlen(str); 
    new_str = realloc(str, size + sizeof(c));
    if(new_str == NULL) {
        free(str);
        return NULL;
    }
    new_str[size] = c;
    return new_str;
}

size_t _trim_left(char *content, size_t cursor)
{
    char c;
    size_t len = strlen(content);
    while (cursor < len) {
        c = content[cursor];
        if(isspace(c) || c == EOF)
        {
            cursor++;
            continue;
        }
        break;
    }
    return cursor;
}

char *_get_next_word(char *content,size_t len, size_t *cursor)
{
    char *token = NULL;
    // trimming all the spaces to the left
    *cursor = _trim_left(content, *cursor);

    while(*cursor < len) {
        char c = tolower(content[*cursor]);

        // if we hit a space, if so we finish reading a word
        if(isspace(c) || c == EOF)
        {
            break;
        }
        // when hitting a digit, get all digits and letters
        // example: 2nd, 100th
        if(isdigit(c) == 1) {
            while (*cursor < len) {
                if(isdigit(c) == 1 || isalpha(c) == 1) {
                    token = _add_char_to_string(token, c);
                    *cursor += 1;
                    c = tolower(content[*cursor]);
                } else {
                    return token;
                }
            }
        }
        // when hitting a letter, get all letters
        // example: mom, table
        if(isalpha(c) == 1) {
            while (*cursor < len) {
                if(isalpha(c) == 1) {
                    token = _add_char_to_string(token, c);
                    *cursor += 1;
                    c = tolower(content[*cursor]);
                } else {
                    return token;
                }
            }
        }
        // if special char take till a break:".", ","
        else {
            token = _add_char_to_string(token, c);
            *cursor += 1;
            return token;
        }
    }
    return token;
}

char **tokenize(char *content) {
    size_t cursor = 0;
    size_t len = strlen(content);
    char **tokens = dynarray_create(char*);
    while (cursor < len) {
        char *token = _get_next_word(content, len, &cursor);
        dynarray_push(tokens, token);
    }

    return tokens;
}




// ######## API ############

// initiating the corpus map
struct hashmap *init_corpus() {
    struct hashmap *corpus =
        hashmap_new(CORPUS_EL_SIZE, 0, 0, 0, _hash,_compare, _free, NULL);
    return corpus;
}

// adds a document to the corpus.
void add_or_update_document(struct hashmap *corpus, char *key, char *content) {
    char **tokens = tokenize(content);
    _add_or_update_document(corpus, key,  tokens, dynarray_length(tokens));
    dynarray_destroy(tokens);
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
struct QueryResults *_search_query(struct hashmap *corpus, char **search_terms, size_t st_count) {
    struct QueryResults *best = arr_init();
    size_t i = 0;
    void *item = NULL;

    while(hashmap_iter(corpus, &i, &item)) {
        struct Document *d = item;
        double score = get_bm25_for_doc(corpus, d, search_terms, st_count);
        struct QueryResult qr;
        qr.score = score;
        copy_string(d->key, &qr.key);
        arr_push(best, qr);
    }
    return best;
}

struct QueryResults *search_query(struct hashmap *corpus, char *search_query) {
    char **tokens = tokenize(search_query);
    struct QueryResults *res =  _search_query(corpus, tokens, dynarray_length(tokens));
    dynarray_destroy(tokens);
    return res;
}
