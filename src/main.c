#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/dirent.h>
#include "dynarray/dynarray.h"
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
    char *query = "bomber pilot tower";
    char *doc1 = "The Tower Realty Company was approved to build on the site in 1929. By May 1929 the site was cleared by razing buildings which included the State Savings and Trust and the Franklin buildings.[1] There was also the four-story building of the Starr Piano Company on the site which had to be purchased and razed. The piano store's 99-year lease also had to be purchased by the Tower Realty Company";

    char *doc2 = "William Ellis Newton, VC (8 June 1919 – 29 March 1943) was an Australian recipient of the Victoria Cross, the highest decoration for gallantry in the face of the enemy that can be awarded to a member of the British and Commonwealth armed forces. He was honoured for his actions as a bomber pilot in Papua New Guinea during March 1943 when, despite intense anti-aircraft fire, he pressed home a series of attacks on the Salamaua Isthmus, the last of which saw him forced to ditch his aircraft in the sea. Newton was still officially posted as missing when the award was made in October 1943. It later emerged that he had been taken captive by the Japanese, and executed by beheading on 29 March.";

    add_or_update_document(c, "doc1", doc1);
    add_or_update_document(c, "doc2", doc2);
    size_t i = 0;
    void *corpus_item = NULL;
    while (hashmap_iter(c, &i, &corpus_item)) {
        struct Document *doc = corpus_item;
        size_t j = 0;
        void *doc_item = NULL;
        while (hashmap_iter(doc->terms ,&j, &doc_item)) {
            struct Term *term = doc_item;
        }
    }
    struct QueryResults *ans = search_query(c,query);
    for (int x = 0; x < ans->length; ++x) {
        printf("result %s has score of %f \n", ans->results[x].key, ans->results[x].score);
    }
    arr_free(ans);
    hashmap_free(c);
}
