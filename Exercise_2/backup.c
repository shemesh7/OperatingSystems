#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

void backup(const char *source, const char *destination);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        exit(1);
    }

    char *source_dir = argv[1];
    char *backup_dir = argv[2];

    backup(source_dir, backup_dir);

    return 0;
}

void backup(const char *source, const char *destination) {
    DIR *dir = opendir(source);
    if (!dir) {
        perror("Failed to open source directory");
        return;
    }

    // Create the destination directory if it doesn't exist
    if (mkdir(destination, 0644) != 0 && errno != EEXIST) {
        perror("Failed to create destination directory");
        closedir(dir);
        return;
    }

    struct dirent *entry;
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip current and parent directory
        }

        snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Recursively call backup for subdirectories
            backup(src_path, dst_path);
        } else if (entry->d_type == DT_REG) {
            // Copy regular files
            FILE *src_file = fopen(src_path, "rb");
            if (!src_file) {
                perror("Failed to open source file");
                continue;
            }

            FILE *dst_file = fopen(dst_path, "wb");
            if (!dst_file) {
                perror("Failed to create destination file");
                fclose(src_file);
                continue;
            }

            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
                fwrite(buffer, 1, bytes, dst_file);
            }

            fclose(src_file);
            fclose(dst_file);
        } else if (entry->d_type == DT_LNK) {
            // Copy symbolic links
            char link_target[PATH_MAX];
            ssize_t len = readlink(src_path, link_target, sizeof(link_target) - 1);
            if (len == -1) {
                perror("Failed to read symbolic link");
                continue;
            }
            link_target[len] = '\0';

            if (symlink(link_target, dst_path) != 0) {
                perror("Failed to create symbolic link");
            }
        }
    }

    closedir(dir);
}