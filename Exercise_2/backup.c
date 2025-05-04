#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

void backup(const char* source, const char* destination);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        exit(1);
    }

    char *source_dir = argv[1];
    char *backup_dir = argv[2];

    // create backup directory if it doesn't exist
    struct stat st = {0};
    if (stat(backup_dir, &st) == -1) {
        mkdir(backup_dir, 0744);
    }

    backup(source_dir, backup_dir);

    return 0;
}

void backup(const char* source, const char* destination) {
    DIR *dir;
    struct dirent *entry;
    char src_path[1024];
    char dst_path[1024];
    
    dir = opendir(source);
    if (dir == NULL) {
        perror("Unable to open source directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // if source is regular file
            snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, entry->d_name);
            
            // create hard link
            if (link(src_path, dst_path) != 0) {
                perror("Failed to create hard link");
            }
        }
        else if (entry->d_type == DT_DIR) {
            // go deeper into the directory
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue; // skip current and parent directory
            }

            snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, entry->d_name);
            // call backup_file recursively
            backup(src_path, dst_path);
        }
        else if (entry->d_type == DT_LNK) {
            // if there is a symbolic link, create a symbolic link in the backup directory
            snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, entry->d_name);
            if (symlink(src_path, dst_path) != 0) {
                perror("Failed to create symbolic link");
            }
        }
    }
    
    closedir(dir);
}