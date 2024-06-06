#include <sqlite3.h>
#include <stdlib.h>
#include "../hash_map/hashmap.h"
#include "../utils/utils.h"
#include "../tfidf/tfidf.h"
#include "../dynamic_array/dynamic_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define FILE_TABLE_INIT "CREATE TABLE IF NOT EXISTS files(path TEXT NOT NULL UNIQUE PRIMARY KEY, created_at INTEGER DEFAULT (unixepoch('now')), updated_at INTEGER DEFAULT (unixepoch('now')));" 

#define TF_TABLE_INIT "CREATE TABLE IF NOT EXISTS tf(path Text NOT NULL, term TEXT, count INTEGER DEFAULT 0, freq REAL DEFAULT 0.0,  PRIMARY KEY(path, term), FOREIGN KEY(path) REFERENCES files(path));"

// #define DF_TABLE_INIT "CREATE TABLE IF NOT EXISTS df(term TEXT NOT NULL UNIQUE PRIMARY KEY, count INTEGER DEFAULT 0);"
#define DB_NAME "info.db"
#define CONCAT(t1,t2) t1 t2

sqlite3 *init_db(char *path);
void insert_to_df_table(sqlite3 *db, struct hashmap *df_files, char **files_to_update, size_t len);
void insert_to_tf_table(sqlite3 *db, struct hashmap *tf_files,char **files_to_update, size_t len);
void insert_to_files_table(sqlite3 *db, char **paths, size_t paths_count);
struct array *load_files_from_db(sqlite3 *db);
unsigned long get_updated_at_from_db(sqlite3 *db, char *path);
void update_file(sqlite3 *db,struct FileTf ftf);
void add_file(sqlite3 *db, struct FileTf ftf);
void remove_file(sqlite3 *db, char *file_path);
