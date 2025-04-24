#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "file_handler.h"

typedef struct {
    const char* filename;
    const char* contents;
} FakeFile;

FakeFile fake_files[] = {
    { "defs.h", "#define VALUE 42\n" },
    { "main.c",
      "#include \"defs.h\"\n"
      "int x = VALUE;\n"
    }
};

const char* load_fake_file(const char* filename) {
    for (int i = 0; i < sizeof(fake_files)/sizeof(FakeFile); i++) {
        if (strcmp(fake_files[i].filename, filename) == 0)
            return fake_files[i].contents;
    }
    return NULL;
}

const char* load_file(const char* filename) {
    return load_fake_file(filename);
}
