#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/dirent.h>
#include "lib/lib.h"
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


char **get_search_terms(int argc, char **argv) {
    int search_term_index = 0;
    char **search_terms = malloc((argc-2)*sizeof(char*));
    for (int i = 2; i<argc; i++ ) {
        search_terms[search_term_index] = argv[i];
        search_term_index++;
    }
    return search_terms;
} 


int main2(int argc, char **argv){
    if(argc == 1){
        printf("ERROR in 'main': missing file_path \n");
        return 1;
    }
    char *dir_path = argv[1];
    char *search_term = argv[2];
    char **search_terms = get_search_terms(argc, argv);
    struct CorpusInfo *corpus_info;
    struct hashmap *df_files, *tf_files;
    printf("search term %s in %s \n", search_term, dir_path);
    tf_files = calc_tf_for_corpus(dir_path);
    df_files = df_corpus(tf_files);
    corpus_info = get_corpus_info(dir_path,df_files, tf_files);
    search_term_in_corpus(search_term, corpus_info);
    
    hashmap_free(corpus_info->df_files);
    hashmap_free(tf_files);
    free(search_terms);
    free(corpus_info);
    return 0;
}


int main(int argc, char **argv) {
    struct hashmap *c = init_corpus();
    char *doc1[] = {
        "is", 
        "is", 
        "is", 
        "is", 
        "is", 
        "is", 
        "is", 
        "red", "yellow", "red", "blue", "gray"
    };
    char *doc2[] = {
        "red", "yellow", "magenta", "black", "magenta", "pink"
    };
    char *doc1edit[] = {
        "term1doc1", "term2doc1", "3termdocrerdsfghjasdgh", "4term4doc2", "term1doc2"
    };

    char *query[] = {
        "red", "pink", "is"
    };

    add_or_update_document(c, "doc1",doc1, 12);
    add_or_update_document(c, "doc2",doc2, 6);
    // add_or_update_document(c, "doc1",doc1edit, 5);
    // remove_document(c,"doc2");
    size_t i = 0;
    void *corpus_item = NULL;
    while (hashmap_iter(c, &i, &corpus_item)) {
        struct Document *doc = corpus_item;
        size_t j = 0;
        void *doc_item = NULL;
        printf("doc key %s doc terms p: %p\n", doc->key, doc->terms);
        while (hashmap_iter(doc->terms ,&j, &doc_item)) {
            struct Term *term = doc_item;
            printf("term: %s, count: %zu \n", term->key, term->count);
        }
    }
    printf("the term term1 occures in %zu docs \n", get_doc_freq_for_term(c, "term1"));
    printf("%s is the most relevant for the query: red is pink \n", search_query(c,query,2, 0));
    hashmap_free(c);
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
