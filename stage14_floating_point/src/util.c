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


void print_with_label(const char *label, const char *text) {
    const char *p = text;
    printf("%s] \n", label);
    int line_number = 0;
    while (*p) {
        line_number++;
        // print until next \n or \0
        const char *line_end = strchr(p, '\n');
        if (line_end && *line_end == '\n') {
            printf("[%03d]  %.*s", line_number, (int)(line_end - p + 1), p);   // uses \n from text
        } else {
            printf("[%03d]  %.*s\n", line_number, (int)(line_end - p), p);
        }
        // print label spaces for next line including space after label
        //printf("%*s", (int)strlen(label) + 1 + 1, " ");
        p = line_end + 1;

    }
    printf("\n");
}

void show_first_mismatch(const char *exp, const char *act) {
    size_t i = 0, line = 1, col = 1;
    for (; exp[i] && act[i] && exp[i] == act[i]; ++i) {
        if (exp[i] == '\n') { line++; col = 1; } else { col++; }
    }
    if (exp[i] == act[i]) return; // identical

    fprintf(stdout, "Mismatch at line %zu, col %zu\n", line, col);

    // Print the whole differing lines with a caret
    const char *ls = exp + i;
    while (ls > exp && ls[-1] != '\n') ls--;
    const char *le = ls; while (*le && *le != '\n') le++;

    const char *rs = act + i;
    while (rs > act && rs[-1] != '\n') rs--;
    const char *re = rs; while (*re && *re != '\n') re++;

    fprintf(stdout, "expected: %.*s\n", (int)(le - ls), ls);
    fprintf(stdout, "actual  : %.*s\n", (int)(re - rs), rs);

    // caret under column
    fprintf(stdout, "          ");
    for (size_t k = 1; k < col; ++k) fputc(' ', stdout);
    fputc('^', stdout);
    fputc('\n', stdout);
}
