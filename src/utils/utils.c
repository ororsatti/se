#include "utils.h"
char *construct_file_path(char *dir_path, char *file_name) {
    int dir_path_size = strlen(dir_path), 
        file_name_size = strlen(file_name);

    if(dir_path[dir_path_size-1] == '/') {
        dir_path[dir_path_size-1] = 0;
        dir_path_size--;
    }
    
    char *file_path = (char*)malloc((file_name_size + dir_path_size + 1) * sizeof(char));
    strcat(file_path, dir_path);
    strcat(file_path, "/");
    strcat(file_path, file_name);
    
    return file_path;
}

