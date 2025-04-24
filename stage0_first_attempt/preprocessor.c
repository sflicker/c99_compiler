#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "macros.h"
#include "preprocessor.h"

#define DIRECTIVE_DEFINE "#define"
#define DIRECTIVE_DEFINE_LEN (sizeof(DIRECTIVE_DEFINE) -1 )

#define DIRECTIVE_UNDEF "#undef"
#define DIRECTIVE_UNDEF_LEN (sizeof(DIRECTIVE_UNDEF) -1)

#define DIRECTIVE_INCLUDE "#include"
#define DIRECTIVE_INCLUDE_LEN (sizeof(DIRECTIVE_INCLUDE) - 1)

#define DIRECTIVE_IF "#if"
#define DIRECTIVE_IF_LEN (sizeof(DIRECTIVE_IF) - 1)

#define DIRECTIVE_IFDEF "#ifdef"
#define DIRECTIVE_IFDEF_LEN (sizeof(DIRECTIVE_IFDEF) -1)

#define DIRECTIVE_IFNDEF "#ifndef"
#define DIRECTIVE_IFNDEF_LEN (sizeof(DIRECTIVE_IFNDEF) -1)

#define DIRECTIVE_ELSE "#else"
#define DIRECTIVE_ELSE_LEN (sizeof(DIRECTIVE_ELSE) -1)

#define DIRECTIVE_ELIF "#elif"
#define DIRECTIVE_ELIF_LEN (sizeof(DIRECTIVE_ELIF) - 1)

#define DIRECTIVE_ENDIF "#endif"
#define DIRECTIVE_ENDIF_LEN (sizeof(DIRECTIVE_ENDIF) - 1)


#define MAX_CONDITIONAL_DEPTH 32

bool cond_stack[MAX_CONDITIONAL_DEPTH];
int cond_top = -1;
bool skipping = false;

typedef struct {
    const char* filename;
    const char* contents;
} FakeFile;

FakeFile fake_files[] = {
    {"myheader.h", "#define VALUE 42\n#define MESSAGE \"Hello from header\"\n"}
};

const int fake_file_count = sizeof(fake_files) / sizeof(fake_files[0]);


const char * read_fake_file(const char * filename) {
    for (int i=0;i<fake_file_count; i++) {
        if (strcmp(fake_files[i].filename, filename) == 0) {
            return fake_files[i].contents;
        }
    }
    return NULL;
}

bool is_macro_defined(const char* name) {
    return find_macro(name) != NULL;
}

void handle_include(const char * line, char * output, size_t max_size) {
    const char* start = strchr(line, '"');
    if (!start) return;
    const char * end = strchr(start + 1, '"');
    if (!end) return;

    char filename[128];
    strncpy(filename, start+1, end - start - 1);
    filename[end - start - 1] = '\0';

    const char* file_content = read_fake_file(filename);
    if (file_content) {
        char preprocessed[4096];
        preprocess(file_content, preprocessed, sizeof(preprocessed));
        strncat(output, preprocessed, max_size - strlen(output) -1);
    }

}

void preprocess(const char* input, char * output, size_t max_size) {

    printf("Preprocessor\n");

    const char* p = input;
    char * out = output;
    *out = '\0';

    while(*p) {
        printf("current: %s\n", p);
        // handle #define
        if (*p == '#' && strncmp(p, DIRECTIVE_DEFINE, DIRECTIVE_DEFINE_LEN) == 0 && isspace(p[DIRECTIVE_DEFINE_LEN])) {
            
            if (skipping) { while (*p && *p != '\n') p++; continue; }
            
            p += DIRECTIVE_DEFINE_LEN;

            while(isspace(*p)) p++;

            char name[64], value[256];
            int ni = 0, vi = 0;

            // read macro name. to space
            while (*p && !isspace(*p)) {
                name[ni++] = *p++;
            }
            name[ni] = '\0';
            // skip whitespace
            while (isspace(*p)) p++;

            // read macro value to newline
            while(*p && *p != '\n') {
                value[vi++] = *p++;
            }
            value[vi] = '\0';

            add_macro(name, value);

        } // handle undef
        else if (*p == '#' && strncmp(p, DIRECTIVE_UNDEF, DIRECTIVE_UNDEF_LEN) == 0 && isspace(p[DIRECTIVE_UNDEF_LEN])) {
            if (skipping) { while (*p && *p != '\n') p++; continue; }
            
            p+= DIRECTIVE_UNDEF_LEN;

            while (isspace(*p)) p++;

            char name[64];
            int ni = 0;
            while (*p && !isspace(*p)) name[ni++] = *p++;
            name[ni] = '\0';

            remove_macro(name);

        }
        // handle ifdef
        else if (*p == '#' && strncmp(p, DIRECTIVE_IFDEF, DIRECTIVE_IFDEF_LEN) == 0 && isspace(p[DIRECTIVE_IFDEF_LEN])) {
            p += DIRECTIVE_IFDEF_LEN;
            while (isspace(*p)) p++;

            char name[64];
            int ni = 0;
            while(*p && !isspace(*p) && *p != '\n') name[ni++] = *p++;
            name[ni] = '\0';

            bool include = is_macro_defined(name);
            cond_stack[++cond_top] = include;
            skipping = !include || skipping;
        }
        // handle ifndef
        else if (*p == '#' && strncmp(p, DIRECTIVE_IFNDEF, DIRECTIVE_IFNDEF_LEN) == 0 && isspace(p[DIRECTIVE_IFNDEF_LEN])) {
            p += DIRECTIVE_IFNDEF_LEN;
            while(isspace(*p)) p++;

            char name[64];
            int ni = 0;
            while (*p && !isspace(*p) && *p != '\n') name[ni++] = *p++;
            name[ni] = '\0';

            bool include = !is_macro_defined(name);
            cond_stack[++cond_top] = include;
            skipping = !include || skipping;
        }
        // handle if
        else if (*p == '#' && strncmp(p, DIRECTIVE_IF, DIRECTIVE_IF_LEN) == 0 && isspace(p[DIRECTIVE_IF_LEN])) {
            p += DIRECTIVE_IF_LEN;
            while (isspace(*p)) p++;

            int value = atoi(p);    // simple constant expression
            cond_stack[++cond_top] = value != 0;
            skipping = !value || skipping;
            while (*p && *p != '\n') p++;
        }
        // handle else
        else if (*p == '#' && strncmp(p, DIRECTIVE_ELSE, DIRECTIVE_ELSE_LEN) == 0 && isspace(p[DIRECTIVE_IF_LEN])) {
            if (cond_top > 0) {
                // flip the condition at the top of the stack
                cond_stack[cond_top] = !cond_stack[cond_top];
                skipping = false;
                // search through the stack and step skipping to true if any conditions are false
                for (int i=0;i<=cond_top;i++) {
                    if (!cond_stack[i]) {
                        skipping = true;
                        break;
                    }
                }
            }
            while (*p && *p != '\n') p++;
        }
        // handle endif
        else if (*p == '#' && strncmp(p, DIRECTIVE_ENDIF, DIRECTIVE_ENDIF_LEN) == 0 && isspace(p[DIRECTIVE_ENDIF_LEN])) {
            if (cond_top >= 0) {
                cond_top--;
                skipping = false;
                for (int i=0;i<=cond_top;i++) {
                    if (!cond_stack[i]) {
                        skipping = true;
                        break;
                    }
                }
            }
            while (*p && *p != '\n') p++;
        }
        // handle #include
        else if (*p == '#' && strncmp(p, DIRECTIVE_INCLUDE, DIRECTIVE_INCLUDE_LEN) == 0 && isspace(p[DIRECTIVE_INCLUDE_LEN])) {
            if (skipping) { while(*p && *p != '\n') p++; continue; }

            char filename[256];
            int i = 0;
            while(*p && *p != '\n') filename[i++] = *p++;
            filename[i] = '\0';
            handle_include(filename, output, max_size);
        } // skip other preprocessor symbols. TODO fix
        else if (*p == '#') {
            while (*p && *p != '\n') p++;
        } 
        // handle other. look for MACROS
        else {
            if (skipping) {
                while(*p && *p != '\n') p++;
            }
            else {
                if (isalpha(*p) || *p == '_') {
                    char word[128];
                    int i = 0;
                    while(isalnum(*p) || *p == '_') word[i++] = *p++;
                    word[i] = '\0';

                    const char * replacement = find_macro(word);
                    if (replacement) {
                        strncat(out, replacement, max_size - strlen(out) - 1);
                    }
                    else {
                        strncat(out, word, max_size - strlen(out) - 1);
                    }
                }
                else {
                    strncat(out, (char[]){*p, 0}, max_size - strlen(out) - 1);
                    p++;
                }
            }
        }

        if (*p == '\n') {
            strncat(output, "\n", max_size -strlen(output) -1);
        }
    }
}
