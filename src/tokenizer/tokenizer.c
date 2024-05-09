#include "tokenizer.h"
#include <_ctype.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trim_left(FILE *fptr)
{
    char c;
    while (!feof(fptr)) {
        c = fgetc(fptr);
        if(c == ' ' || c == '\n' || c == EOF)
        {
            continue;
        }
        fseek(fptr,-1,SEEK_CUR);
        break;
    }
}

void *add_char_to_string(char *str, char c){
    char *new_str = NULL;
    if(str == NULL) {
        new_str = realloc(str, sizeof(c));
        new_str[0] = c;
        return new_str;
    }

    size_t size = strlen(str); 
    new_str = realloc(str, size + sizeof(c));
    if(new_str == NULL) {
        free(str);
        return NULL;
    }
    new_str[size] = c;
    return new_str;
}

char *get_next_word(FILE *fptr)
{
    char *token = NULL;
    int i = 0, size = 0;
    char c;
    
    while (!feof(fptr)) {

        trim_left(fptr);

        if(feof(fptr)){
            break;
        }

        c = tolower(fgetc(fptr));
        
        // if digit take till a break: "256", "100th", "1st"
        if(isdigit(c) == 1) {
            while (isdigit(c) == 1 || isalpha(c) == 1) {
                token = add_char_to_string(token, c);
                c = tolower(fgetc(fptr));
            }
            // going back once because we advance the char preemtively
            if(!feof(fptr)) {
                fseek(fptr, -1, SEEK_CUR);
            }
            break;
        }
        // if alpha take till a break: "mother", "cool"
        else if(isalpha(c) == 1) {
            while (isalpha(c)) {
                token = add_char_to_string(token, c);
                c = tolower(fgetc(fptr));
            }
            // going back once because we advance the char preemtively
            //
            // if we are alrady at the end of the file we don't want to go back

            if(!feof(fptr)) {
                fseek(fptr, -1, SEEK_CUR);
            }
            break;
        }
        // if special char take till a break:".", ","
        else {
            token = add_char_to_string(token, c);
            break;
        }
    }
    return token;
}

