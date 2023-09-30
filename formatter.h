#ifndef FORMATTER
#define FORMATTER

int is_valid_format(char *path, char *file_format);
char *convert_to_new_format(const char *filename, const char *matched_regex); 
const char *get_datetime_regex_pattern(const char *input);

#endif