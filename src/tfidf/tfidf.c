#include "tfidf.h"
#include <stdlib.h>
#include <sys/_types/_size_t.h>

uint64_t file_tf_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct FileTf *it = item;
    return hashmap_sip(it->path, strlen(it->path), seed0, seed1);
}

int file_tf_compare(const void *a,const void *b, void *udata) {
    const struct FileTf * item1 = a;
    const struct FileTf * item2 = b;
    return strcmp(item1->path, item2->path);
}

void file_tf_free(void *item) {
    struct FileTf *it = item;
    free(it->path);
    hashmap_free(it->tf);
}
uint64_t term_freq_item_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct TermFreq *it = item;
    return hashmap_sip(it->key, strlen(it->key), seed0, seed1);
}

int term_freq_item_compare(const void *a,const void *b, void *udata) {
    const struct TermFreq * item1 = a;
    const struct TermFreq * item2 = b;
    return strcmp(item1->key, item2->key);
}

void term_freq_item_free(void *item) {
    struct TermFreq *it = item;
    free(it->key);
}

void add_next_token(struct hashmap *map, char *next_token) {
    const struct TermFreq *item = hashmap_get(map, &(struct TermFreq){.key=next_token});

    if(item == NULL) {
        struct TermFreq tmp;
        tmp.count = 1;
        tmp.key = malloc(sizeof(*next_token) *strlen(next_token));
        strcpy(tmp.key,next_token);
        hashmap_set(map, &tmp);
    }
    
    else {
        struct TermFreq tmp;
        tmp.count = item->count + 1;
        tmp.key = item->key;
        hashmap_set(map, &tmp);
    }
}

struct hashmap *tf_file(char *file_path) {
    FILE *fptr = fopen(file_path, "r");
    char *next_token = NULL;
    size_t i = 0;
    void *item;

    if(fptr == NULL) {
        printf("ERROR in 'tf_file(char *file_path)': Could not open file \n");
        return NULL;
    }
    int doc_term_count = 0;
    struct hashmap *map = hashmap_new(sizeof(struct TermFreq), 
                                      0, 0, 0, 
                                      term_freq_item_hash, 
                                      term_freq_item_compare, 
                                      term_freq_item_free, 
                                      NULL);
    // go over all tokens and add them to the hashmap
    do {
        next_token = get_next_word(fptr);
        if(next_token == NULL) {
            break;
        }
        add_next_token(map,next_token);
        doc_term_count++;
        free(next_token);
    } while(next_token != NULL);

    // calculate the tf of each token
    while(hashmap_iter(map,&i,  &item)) {
        struct TermFreq *tf_item = (struct TermFreq*)(item);
        tf_item->freq = (float)tf_item->count / doc_term_count;
    }
    fclose(fptr);
    return map;
}

uint64_t df_item_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct DocFreq *it = item;
    return hashmap_sip(it->term, strlen(it->term), seed0, seed1);
}

int df_item_compare(const void *a,const void *b, void *udata) {
    const struct DocFreq * item1 = a;
    const struct DocFreq * item2 = b;
    return strcmp(item1->term, item2->term);
}

void df_item_free(void *item) {
    struct DocFreq *it = item;
    free(it->term);
}

struct hashmap *df_corpus(struct hashmap *tf_files) {
    struct hashmap *map = hashmap_new(sizeof(struct DocFreq),
                                      0, 0, 0,
                                      df_item_hash,
                                      df_item_compare, 
                                      df_item_free,
                                      NULL);
    size_t i = 0;
    void *item =NULL;
    // iterating over tf map of the files
    while(hashmap_iter(tf_files,&i,  &item)) {
        struct FileTf *file_tf_item = (struct FileTf*)(item);
        size_t j = 0;
        void *term_freq_item;
        // iterating over each file's tf and adding the term count to the doc freq map 
        while(hashmap_iter(file_tf_item->tf, &j,  &term_freq_item)) {
            struct TermFreq *term_freq = (struct TermFreq*)term_freq_item;
            struct DocFreq *doc_freq = (struct DocFreq*)hashmap_get(map, &(struct DocFreq){
                .term = term_freq->key
            });
            if(doc_freq == NULL) {
                struct DocFreq tmp = {
                    .term = malloc(strlen(term_freq->key)*sizeof(char)),
                    .count = 1
                    };
                strcpy(tmp.term,term_freq->key);
                doc_freq = &tmp;
            }else {
                doc_freq->count++;
            }
            hashmap_set(map, doc_freq);
        }
    }
    return map;
}

struct CorpusInfo *get_corpus_info(char *dir_path,struct hashmap *df_files, struct hashmap *tf_files) {
    
    DIR *pDir = opendir(dir_path);
    struct dirent *pDirnet;
    size_t file_count = 0;
    struct CorpusInfo *corpus_info;
    char *next_token, file_name;
    if(pDir == NULL) {
        printf("ERROR in 'idf_dir': failed to open directory\n");
        return NULL;
    }
    corpus_info = malloc(sizeof(struct CorpusInfo));
    
    // iterating over corpus and counting files
    while((pDirnet = readdir(pDir)) != NULL) {
        if(strcmp(pDirnet->d_name,  ".") == 0 
            || strcmp(pDirnet->d_name,  "..") == 0) {
            continue;
        }
        file_count++;
    }
    corpus_info->file_count = file_count;
    corpus_info->df_files = df_files;
    corpus_info->tf_files = tf_files;
    corpus_info->dir_path = dir_path;
    closedir(pDir);   
    // calc the df of each token in each file
    return corpus_info;
}


struct hashmap *calc_tf_for_corpus(char *dir_path) {
    struct hashmap *tf_files;
    DIR *pDir = opendir(dir_path);
    struct dirent *pDirent;
    if(pDir == NULL) {
        printf("ERROR in 'calc_tf_for_corpus': failed to open directory\n");
        return NULL;
    }

    tf_files = hashmap_new(sizeof(struct FileTf), 
                            0, 0, 0,
                            file_tf_hash,
                            file_tf_compare,
                            file_tf_free,
                            NULL);

    while((pDirent = readdir(pDir)) != NULL) {

        struct hashmap *tf_for_file;
        char *file_path;

        if(strcmp(pDirent->d_name,  ".") == 0 
            || strcmp(pDirent->d_name,  "..") == 0) {
            continue;
        }
        // file_path = malloc(sizeof(char) * strlen(pDirent->d_name));
        file_path  = construct_file_path(dir_path, pDirent->d_name);
        tf_for_file = tf_file(file_path);

        if(tf_for_file == NULL) {
            printf("ERROR in 'calc_tf_for_corpus': failed to run ft_file\n");
            free(file_path);
            closedir(pDir);
            return NULL;
        }

        struct FileTf tmp = {
            .tf = tf_for_file,
            .path = file_path,
        };
        hashmap_set(tf_files, (void*)&tmp);
    }
    closedir(pDir);
    return tf_files;
}

double calc_tfidf(size_t doc_freq, double term_freq, size_t doc_count) {
    double term_count = (double)(doc_freq == 0 ? 1: doc_freq);
    return term_freq * log(doc_count/term_count); 
}
