#ifndef __UTIL_H__
#define __UTIL_H__


char* change_extension(const char* source_file, const char* new_ext);
char * read_text_file(const char* filename);
char *write_temp_file(const char *content);
const char * get_file_extension(const char * filename);

#endif