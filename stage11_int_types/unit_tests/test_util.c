#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "util.h"
#include "test_assert.h"

const char * program = 
    "int main() {\n"
    "    return 42;\n"
    "}";



void test_read_text_file() {
    char * filename = write_temp_file(program);
    //char * filename = get_temp_filename();
    // FILE* file = fopen(filename, "w");
    // fprintf(file, "%s", program);
    // fclose(file);

    char * program_data = read_text_file(filename);
    TEST_ASSERT("Verifying data was correctly read", strcmp(program_data, program) == 0);
}

void test_change_extension() {
    char * original_filename = "c_source.c";
    char * new_filename = change_extension(original_filename, ".s");
    TEST_ASSERT("Verifying filename has correct new extension", strcmp("s", get_file_extension(new_filename))==0);
}

int main() {
    printf("starting test_util\n");
    test_read_text_file();
    test_change_extension();
}