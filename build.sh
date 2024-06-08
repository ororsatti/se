#!/bin/bash

set -xe
if [ ! -d ./dest ]; then
    mkdir ./dest
fi

gcc ./src/lib/hashmap/hashmap.c -o ./dest/hashmap.o -c
gcc ./src/lib/dynarray/dynarray.c -o ./dest/dynarray.o -c
gcc ./src/lib/lib.c -o ./dest/lib.o -c
gcc ./src/main.c -o ./dest/main.o -c


gcc -o ./dest/se ./dest/hashmap.o ./dest/main.o ./dest/lib.o ./dest/dynarray.o
# rm dest/main.o  ./dest/hashmap.o ./dest/lib.o ./dest/dynarray.o
rm dest/main.o  ./dest/lib.o



