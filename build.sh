#!/bin/bash

set -xe
if [ ! -d ./dest ]; then
    mkdir ./dest
fi

gcc ./src/tokenizer/tokenizer.c -o ./dest/tokenizer.o -c
gcc ./src/tfidf/tfidf.c -o ./dest/tfidf.o -c
gcc ./src/hash_map/hashmap.c -o ./dest/hashmap.o -c
gcc ./src/main.c -o ./dest/main.o -c
gcc ./src/db/db.c -o ./dest/db.o -c
gcc ./src/utils/utils.c -o ./dest/utils.o -c

if [ ! -f ./dest/sqlite3.o ]; then
    gcc ./src/db/sqlite3.c -o ./dest/sqlite3.o -c
fi


gcc -o ./dest/se ./dest/tokenizer.o ./dest/hashmap.o ./dest/main.o ./dest/tfidf.o ./dest/sqlite3.o ./dest/db.o ./dest/utils.o
rm dest/main.o dest/tokenizer.o ./dest/hashmap.o ./dest/tfidf.o ./dest/db.o ./dest/utils.o
