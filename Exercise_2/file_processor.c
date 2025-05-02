#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>

#define LINE_LEN 1024
#define BUF_SIZE 2048

// probably make functions for all edge cases 
void read_option(int data_fd, int results_fd, int start, int end);
void write_option(int data_fd, int offset, char* text);


int main(int argc,char** argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data.txt> <requests.txt>\n", argv[0]);
        exit(1);
    }
    
    int data_fd = open(argv[1], O_RDWR);
    if (data_fd < 0) {
        perror("data file");
        exit(1);
    }

    int requests_fd = open(argv[2], O_RDONLY);
    if (requests_fd < 0) {
        perror("requests file");
        exit(1);
    }

    int results_fd = open("read_results.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (results_fd < 0) {
        perror("read_results.txt");
        exit(1);
    }
    FILE *requests_file = fdopen(requests_fd, "r");
    if (!requests_file) {
        perror("fdopen requests_fd");
        exit(1);
    }

    char line[LINE_LEN];
    while (fgets(line, LINE_LEN, requests_file)) {
        if (line[0] == 'Q') break;

        if (line[0] == 'R') {
            int start, end;
            if (sscanf(line, "R %d %d", &start, &end) == 2) {
                read_option(data_fd, results_fd, start, end);
            }
        } else if (line[0] == 'W') {
            int offset;
            char text[BUF_SIZE];
            if (sscanf(line, "W %d %[^\n]", &offset, text) == 2) {
                write_option(data_fd, offset, text);
            }
        }
    }

    close(data_fd);
    close(results_fd);
    fclose(requests_file);
    return 0;
}

/**
 * check edge cases:
 * ✅ Reading beyond file size → Prevent it with boundary checks, and continue to the next line (dont exit).
 * ✅ Writing beyond file size → Same as reading, if I give you the offset of the end of the file it's ok (but after that it's not)
 *      so is offset 0 (but before that it's not).
 * ✅ Other things → You dont need to worry about integer overflow and `start > end` in the reading.
 * 
 **/

void read_option (int data_fd, int results_fd, int start, int end) {
    int data_size = lseek(data_fd, 0, SEEK_END);
    if (start < 0 || end > data_size)           // Reading beyond file size
        return;
    int len = end - start + 1;
    char *buffer = malloc(len);
    if (!buffer) {
        perror("malloc for buffer");
        exit(1);
    }
    lseek(data_fd, start, SEEK_SET);
    read(data_fd, buffer, len);

    lseek(data_fd, 0, SEEK_END);
    write(results_fd, buffer, len);
    write(results_fd, "\n", 1);

    free(buffer);
}

void write_option (int data_fd, int offset, char* text) { 
    int data_size = lseek(data_fd, 0, SEEK_END);
    if (offset < 0 || offset > data_size)           // Writing beyond file size
        return;

    // shift data from offset to offset + text_len, so that we dont overwrite existing data.
    int rest_size = data_size - offset;      // size of the rest of the file
    char *rest = malloc(rest_size);
    if (!rest) {
        perror("malloc");
        exit(1);
    }
    lseek(data_fd, offset, SEEK_SET);
    read(data_fd, rest, rest_size);

    lseek(data_fd, offset, SEEK_SET);
    write(data_fd, text, strlen(text));
    write(data_fd, rest, rest_size);
}