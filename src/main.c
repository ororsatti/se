#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include "dynamic_array/dynamic_array.h"
#include "tokenizer/tokenizer.h"
#include "hash_map/hashmap.h"
#include "db/db.h"

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

void save_files(struct hashmap *tf_files,struct hashmap *df_files, sqlite3 *db) {
    void *item;
    size_t i = 0, index = 0,
            count = hashmap_count(tf_files);
    char **pathes = malloc(count * sizeof(char**));
    while(hashmap_iter(tf_files, &i, &item)) {
        struct FileTf *file_tf = (struct FileTf*)item;
        pathes[index] = file_tf->path;
        index++;
    }

    // add new files
    // delete missing files
    // alter existing files
    insert_to_files_table(db, pathes ,count);
    insert_to_tf_table(db, tf_files, NULL, 0);
    free(pathes);
}


void noop_free(void *str) {return;}
void update_db(char *dir_path, sqlite3 *db) {
    DIR *pDir = opendir(dir_path);
    struct array *saved_files = load_files_from_db(db),
        *current_files = arr_init(sizeof(char*)),
        *altered_files = arr_init(sizeof(char*));
    struct dirent *pDirent;
    if(pDir == NULL) {
        printf("ERROR in 'update_db': failed to open directory\n");
        return;
    }
    while((pDirent = readdir(pDir)) != NULL) {
        char *file_path;

        if(strcmp(pDirent->d_name,  ".") == 0 
            || strcmp(pDirent->d_name,  "..") == 0) {
            continue;
        }
        // getting the full path
        file_path  = construct_file_path(dir_path, pDirent->d_name);
        struct stat result;
        // evaluating the created at and updated at of a file
        stat(file_path, &result);
        unsigned long updated_at = result.st_mtimespec.tv_sec;
        unsigned long crated_at = result.st_birthtimespec.tv_sec;
        printf("loading %s \n",file_path);
        unsigned long stored_updated_at = get_updated_at_from_db(db,file_path);
        // if stored_updated_at is 0 we have an error / the file is not found so ignoring it
        if(stored_updated_at < updated_at && stored_updated_at != 0) {
            arr_push(altered_files,file_path);
        }

        // pushing the file into the list of current file
        arr_push(current_files, file_path);
    }

    struct array *new_files = arr_diff(current_files,saved_files);
    struct array *files_to_remove = arr_diff(saved_files, current_files);

    printf("::::current files:::\n");
    for (int i = 0; i < current_files->len;i++) {
        printf("    file: %s \n", (char*)current_files->items[i]);
    }
    printf("::::saved files:::\n");
    for (int i = 0; i < saved_files->len;i++) {
        printf("    file: %s \n", (char*)saved_files->items[i]);
    }



    printf("::::altered files::::\n");
    for (int i = 0; i < altered_files->len;i++) {
        printf("    new file: %s \n", (char*)altered_files->items[i]);
    }
    printf("::::files to remove::::\n");
    for (int i = 0; i < files_to_remove->len;i++) {
        printf("    file: %s \n", (char*)files_to_remove->items[i]);
        remove_file(db, files_to_remove->items[i]);
    }
    printf("::::files to add::::\n");
    for (int i = 0; i < new_files->len;i++) {
        printf("    file: %s \n", (char*)new_files->items[i]);
    }


    arr_free(current_files, free_string);
    arr_free(saved_files, free_string);
    arr_free(altered_files, noop_free);
    arr_free(new_files, free_string);
    arr_free(files_to_remove, free_string);
}

int main(int argc, char **argv){

    char *dir_path = argv[1];
    struct CorpusInfo *corpus_info;
    struct hashmap *df_files, *tf_files;
    sqlite3 *db;
    tf_files = calc_tf_for_corpus(dir_path);
    df_files = df_corpus(tf_files);
    corpus_info = get_corpus_info(dir_path,df_files, tf_files);
    db = init_db(dir_path);
    update_db(dir_path, db);
    // save_files(tf_files,df_files, db);
    // load_files_from_db(db);
    hashmap_free(df_files);
    hashmap_free(tf_files);
    sqlite3_close(db);
    free(corpus_info);
    return 0;
}
