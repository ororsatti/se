#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_TABLE_INIT "CREATE TABLE IF NOT EXISTS files(path TEXT NOT NULL UNIQUE PRIMARY KEY, created_at INTEGER DEFAULT (unixepoch('now')), updated_at INTEGER DEFAULT (unixepoch('now')));" 
#define TF_TABLE_INIT "CREATE TABLE IF NOT EXISTS tf(term TEXT NOT NULL PRIMARY KEY, count INTEGER DEFAULT 0, freq REAL DEFAULT 0.0);"
#define DF_TABLE_INIT "CREATE TABLE IF NOT EXISTS df(path TEXT NOT NULL, term TEXT NOT NULL , count INTEGER DEFAULT 0, PRIMARY KEY(path, term), FOREIGN KEY(path) REFERENCES files(path));"
#define DB_NAME "info.db"

#define CONCAT(t1,t2,t3) t1 t2 t3
int cb(void *not_used, int argc, char **argv, char **az_col_name) {
    int i;
    for (i=0; i<argc; i++) {
        printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void handle_sqlite_error(sqlite3 *db, char *err_msg){
    fprintf(stderr,"SQL error: %s \n",err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
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
    rc = sqlite3_exec(db, CONCAT(FILE_TABLE_INIT, TF_TABLE_INIT, DF_TABLE_INIT), cb, 0, &z_error_msg);
    if(rc != SQLITE_OK) {
        handle_sqlite_error(db,z_error_msg);
        return NULL;
    }
    return db;
}

