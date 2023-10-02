#ifndef UTILS
#define UTILS
#include <time.h>

int rename_file(char *file_path, char *new_name);

int has_correct_date_format(const char *string, const char *format);

int is_in_range(time_t modification_time, time_t start_time, time_t finish_time);

char *get_last_el_of_split(char *path, char *delim);

#endif