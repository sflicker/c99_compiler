#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>

#include "util.h"
#include "token.h"
#include "ast.h"

char * read_text_file(const char* filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char * buffer = malloc(filesize + 1);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        exit(1);
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if (read_size != filesize) {
        fprintf(stderr, "fread failed\n");
        fclose(file);
        free(buffer);
        exit(1);
    }

    buffer[filesize] = '\0';
    fclose(file);
    return buffer;

}

// Writes `content` to a safe temp file and returns a malloc'd filename
char *write_temp_file(const char *content) {
    char template[] = "/tmp/tmp_test_file_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        exit(1);
    }

    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        remove(template);
        exit(1);
    }

    fputs(content, f);
    fclose(f);  // also closes fd

    // Return a heap copy of the filename so the caller can manage it
    return strdup(template);
}

char* change_extension(const char* source_file, const char* new_ext) {
    const char* dot = strrchr(source_file, '.'); // find last dot
    size_t base_length = (dot) ? (size_t)(dot - source_file) : strlen(source_file);

    size_t new_length = base_length + strlen(new_ext);
    char* out_file = malloc(new_length + 1); // +1 for '\0'
    if (!out_file) {
        perror("malloc");
        exit(1);
    }

    memcpy(out_file, source_file, base_length);
    strcpy(out_file + base_length, new_ext);

    return out_file;
}

const char * get_file_extension(const char * filename) {
    const char * dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void print_indent(int indent) {
    for (int i=0;i<indent;i++) printf("  ");
}



