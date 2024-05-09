#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/dirent.h>
#include "tokenizer/tokenizer.h"
#include "hash_map/hashmap.h"
#include "tfidf/tfidf.h"

void search_term_in_corpus(char *term,struct CorpusInfo *ci) {
    double max_tfidf = 0;
    char *relavent_file_path = NULL;
    size_t i = 0;
    void *item;
    struct DocFreq *term_doc_freq = (struct DocFreq*)hashmap_get(ci->df_files, &(struct DocFreq){ .term = term });
    while(hashmap_iter(ci->tf_files, &i, &item)) {
        struct FileTf *file_tf = (struct FileTf*)item;
        struct TermFreq *tf = (struct TermFreq*)hashmap_get(file_tf->tf, &(struct TermFreq){ .key = term });
        if(tf == NULL) {
            continue;
        }
        double tfidf = calc_tfidf(term_doc_freq->count,tf->freq, ci->file_count);
        if(tfidf > max_tfidf) {
            max_tfidf = tfidf;
            relavent_file_path = file_tf->path;
        }
    }

    printf("%s \n", relavent_file_path);

}

int main(int argc, char **argv){

    if(argc == 1){

        printf("ERROR in 'main': missing file_path \n");
        return 1;
    }
    char *dir_path = argv[1];
    char *search_term = argv[2];
    struct CorpusInfo *corpus_info;
    struct hashmap *tf_files;
    printf("search term %s in %s \n", search_term, dir_path);
    tf_files = calc_tf_for_corpus(dir_path);
    corpus_info = get_corpus_info(dir_path, tf_files);
    search_term_in_corpus(search_term, corpus_info);
    
    hashmap_free(corpus_info->df_files);
    hashmap_free(tf_files);
    free(corpus_info);
    return 0;
}


// while(hashmap_iter(tf_files,&i,  &item)) {
//     struct FileTf *file_tf_item = (struct FileTf*)(item);
//     size_t j = 0;
//     void *tf;
//     printf("-----------------------\n");
//     printf("Iterating over: %s \n", file_tf_item->path);
//     printf("-----------------------\n");
//     while(hashmap_iter(file_tf_item->tf,&j,  &tf)) {
//         struct TermFreq *tf_as_tf = (struct TermFreq*)tf;
//         struct DocFreq tmp = { .term = tf_as_tf->key};
//         struct DocFreq* df = (struct DocFreq*)hashmap_get(corpus_info->df_files, &tmp);
//
//         double tfidf = calc_tfidf(df->count, tf_as_tf->freq, corpus_info->file_count); 
//         printf("    -----------------------\n");
//         printf("    df: %s: tfidf:  %f\n", df->term, tfidf);
//         printf("    -----------------------\n");
//     }
// }
