/**
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

void backup_file(const char* source, const char* destination);
void create_backup_dir(const char* path);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        exit(1);
    }

    char *source_dir = argv[1];
    char *backup_dir = argv[2];

    // Create backup directory if it doesn't exist
    create_backup_dir(backup_dir);
    
    // Perform backup
    backup_file(source_dir, backup_dir);

    return 0;
}

void create_backup_dir(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}

void backup_file(const char* source, const char* destination) {
    DIR *dir;
    struct dirent *entry;
    char src_path[1024];
    char dst_path[1024];
    
    dir = opendir(source);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // If regular file
            snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, entry->d_name);
            
            // Copy file
            FILE *src = fopen(src_path, "rb");
            FILE *dst = fopen(dst_path, "wb");
            
            if (src && dst) {
                char buffer[4096];
                size_t bytes;
                
                while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                    fwrite(buffer, 1, bytes, dst);
                }
                
                fclose(src);
                fclose(dst);
            }
        }
    }
    
    closedir(dir);
}
**/