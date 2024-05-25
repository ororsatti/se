#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../hash_map/hashmap.h"
#include "../tokenizer/tokenizer.h"
#include "../utils/utils.h"

uint64_t term_freq_item_hash(const void *item, uint64_t seed0, uint64_t seed1);
int term_freq_item_compare(const void *a,const void *b, void *udata);
void term_freq_item_free(void *item);

uint64_t file_tf_hash(const void *item, uint64_t seed0, uint64_t seed1);
int file_tf_compare(const void *a,const void *b, void *udata);
void file_tf_free(void *item);

struct hashmap *tf_file(char *file_path);
struct hashmap *calc_tf_for_corpus(char*);
struct hashmap *df_corpus(struct hashmap *tf_files);
struct CorpusInfo *get_corpus_info(char *dir_path,struct hashmap *df_files, struct hashmap *tf_files);
double calc_tfidf(size_t ,double , size_t);


struct TermFreq {
    char *key;
    int count;
    float freq;
};

struct FileTf {
    char *path;
    struct hashmap *tf;
};

struct DocFreq {
    char *term;
    size_t count;
};

struct CorpusInfo {
    char *dir_path;
    size_t file_count;
    struct hashmap *df_files;
    struct hashmap *tf_files;
};
