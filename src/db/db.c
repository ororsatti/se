#include "db.h"
#include <stdio.h>

int cb(void *not_used, int argc, char **argv, char **az_col_name) {
    int i;
    for (i=0; i<argc; i++) {
        printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int file_names_cb(void *v, int argc, char **argv, char **az_col_name) {
    int i;
    for (i=0; i<argc; i++) {
        if(strcmp(az_col_name[i],"path") == 0) {
            char *name = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(name, argv[i]);
            arr_push(v, name);
        }
    }
    return 0;
}

void handle_sqlite_error(sqlite3 *db, char *err_msg) {
    fprintf(stderr,"SQL error: %s \n",err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
}

void handle_quitely_sqlite_error(sqlite3 *db, char *err_msg){
    fprintf(stderr,"SQL error: %s \n",err_msg);
}

sqlite3 *init_db(char *path) {
    char *db_path, *z_error_msg = NULL;
    int rc;
    sqlite3 *db;
    rc = sqlite3_open(DB_NAME, &db);
    if(rc) {
        fprintf(stderr, "Can't open db %s \n", DB_NAME);
        return NULL;
    }
    rc = sqlite3_exec(db, CONCAT(FILE_TABLE_INIT, TF_TABLE_INIT), cb, 0, &z_error_msg);
    if(rc != SQLITE_OK) {
        handle_sqlite_error(db,z_error_msg);
        return NULL;
    }
    return db;
}

void colonize_insert_files_stmt(sqlite3_stmt *stmt, char *path) {
    struct stat result;
    stat(path, &result);
    unsigned long updated_at = result.st_mtimespec.tv_sec;
    unsigned long crated_at = result.st_birthtimespec.tv_sec;

    size_t l = strlen(path);
    sqlite3_bind_text(stmt,1,path,l, SQLITE_STATIC);
    sqlite3_bind_int64(stmt,2,crated_at);
    sqlite3_bind_int64(stmt,3,updated_at);
}

void insert_to_files_table(sqlite3 *db, char **paths, size_t paths_count) {
    int err;
    sqlite3_stmt *stmt;
    char *inst_stmt = "INSERT INTO files VALUES (?,?,?) ON CONFLICT(path) DO UPDATE SET updated_at=excluded.updated_at, created_at=excluded.created_at";

    err = sqlite3_prepare_v2(db,inst_stmt,-1,&stmt, NULL);
    if(err != SQLITE_OK) {
        printf("Failed to create prepared statement, ERROR CODE: %d \n", sqlite3_errcode(db));
        sqlite3_finalize(stmt);
    }

    for (size_t i = 0;i < paths_count ; i++) {
        colonize_insert_files_stmt(stmt, paths[i]);
        err = sqlite3_step(stmt);
        
        if(err != SQLITE_DONE) {
            printf("failed to insert %s to files, ERROR CODE: %d\n", paths[i], sqlite3_errcode(db));
            sqlite3_finalize(stmt);
        }

        sqlite3_reset(stmt);
    }
}

void colonize_tf_stmt(sqlite3_stmt *stmt, char *path, struct TermFreq tf) {
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, tf.key, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, tf.count);
    sqlite3_bind_double(stmt, 4, (double)tf.freq);
}

void insert_to_tf_table(sqlite3 *db, struct hashmap *tf_files,char **files_to_update, size_t len) {
    void *item;
    size_t hashmap_index = 0;
    int err;
    while (hashmap_iter(tf_files, &hashmap_index, &item)) {
        void *tf_item;
        size_t tf_hashmap_index = 0;
        
        struct FileTf *ftf = item;
        char *path = ftf->path;
        char *stmt_str = "INSERT OR REPLACE INTO tf VALUES (?,?,?,?)";
        sqlite3_stmt *stmt;

        err = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);

        if(err != SQLITE_OK) {
            printf("Failed to create prepared statement, ERROR CODE: %d \n", sqlite3_errcode(db));
            sqlite3_finalize(stmt);
        }

        while (hashmap_iter(ftf->tf, &tf_hashmap_index, &tf_item)) {
            struct TermFreq *tf = tf_item;
            colonize_tf_stmt(stmt, path, *tf);
            err = sqlite3_step(stmt);

            if(err != SQLITE_DONE) {
                printf("Failed to insert %s:%s to tf table. ERROR CODE: %d \n",path, tf->key, sqlite3_errcode(db));
                printf("%s\n",sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
            }
            sqlite3_reset(stmt);
        }
    }
}
struct array *load_files_from_db(sqlite3 *db) {
    char *sql = "SELECT * FROM files;";
    char *err_msg;
    struct array *file_names = arr_init(sizeof(char*)); sqlite3_exec(db, sql, file_names_cb ,file_names, &err_msg);
    return file_names;
}

unsigned long get_updated_at_from_db(sqlite3 *db, char *path) {
    char *sql = "SELECT updated_at FROM files WHERE files.path=?";
    sqlite3_stmt *stmt; 
    int err;
    err = sqlite3_prepare_v2(db, sql, -1,&stmt, NULL); 
    if(err != SQLITE_OK) {
        printf("Failed to create prepared statement, ERROR CODE: %d \n", sqlite3_errcode(db));
        sqlite3_finalize(stmt);
    }
    
    sqlite3_bind_text(stmt, 1, path, -1,SQLITE_STATIC);
    err = sqlite3_step(stmt);

    if(err != SQLITE_ROW) {
        printf("An error as occured: %d %s \n",err, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }
    unsigned long updated_at = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);

    return updated_at;
}

void update_file(sqlite3 *db,struct FileTf ftf) {}
void add_file(sqlite3 *db, struct FileTf ftf) {}
void remove_file(sqlite3 *db, char *file_path) {
    const char *sql_files = "DELETE FROM files WHERE files.path=?",
        *sql_tf = "DELETE FROM tf WHERE tf.path=?";
    sqlite3_stmt *stmt;
    int err;

    err = sqlite3_prepare_v2(db, sql_tf, -1,&stmt, NULL); 
    if(err != SQLITE_OK) {
        printf("Failed to create prepared statement, ERROR CODE: %d \n", sqlite3_errcode(db));
        sqlite3_finalize(stmt);
    }
    sqlite3_bind_text(stmt, 1, file_path, -1, SQLITE_STATIC);
    err = sqlite3_step(stmt);
    // TODO: finish this function and test
    if(err != SQLITE_DONE) {
        printf("An error as occured: %d %s \n",err, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    err = sqlite3_prepare_v2(db, sql_files, -1,&stmt, NULL); 
    if(err != SQLITE_OK) {
        printf("Failed to create prepared statement, ERROR CODE: %d \n", sqlite3_errcode(db));
        sqlite3_finalize(stmt);
    }
    sqlite3_bind_text(stmt, 1, file_path, -1, SQLITE_STATIC);

    err = sqlite3_step(stmt);

    if(err != SQLITE_DONE) {
        printf("An error as occured: %d %s \n",err, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}
