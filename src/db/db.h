#include <sqlite3.h>
#include <stdlib.h>
#include "../hash_map/hashmap.h"

#define FILE_TABLE_INIT "CREATE TABLE IF NOT EXISTS files(path TEXT NOT NULL UNIQUE PRIMARY KEY, created_at INTEGER DEFAULT (unixepoch('now')), updated_at INTEGER DEFAULT (unixepoch('now')));" 

#define TF_TABLE_INIT "CREATE TABLE IF NOT EXISTS tf(path Text NOT NULL, term TEXT, count INTEGER DEFAULT 0, freq REAL DEFAULT 0.0,  PRIMARY KEY(path, term), FOREIGN KEY(path) REFERENCES files(path));"

#define DF_TABLE_INIT "CREATE TABLE IF NOT EXISTS df(path TEXT NOT NULL, term TEXT NOT NULL , count INTEGER DEFAULT 0, PRIMARY KEY(path, term), FOREIGN KEY(path) REFERENCES files(path));"
#define DB_NAME "info.db"
#define CONCAT(t1,t2,t3) t1 t2 t3

sqlite3 *init_db(char *path);
void insert_to_df_table(sqlite3 *db, struct hashmap df_files, char **files_to_update, size_t len);
void insert_to_tf_table(sqlite3 *db, struct hashmap *tf_files,char **files_to_update, size_t len);
void insert_to_files_table(sqlite3 *db, char **paths, size_t paths_count);
