#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int rename_file(char *file_path, char *new_name)
{
    // Rename the file using the rename() function
    if (rename(file_path, new_name) != 0)
    {
        fprintf(stderr, "Error renaming file");
        return 0;
    }
    else
    {
        printf("Renamed: %s -> %s\n", file_path, new_name);
        return 1;
    }
}

/* Check if file has correct date time format in name */
int has_correct_date_format(const char *string, const char *format)
{
    struct tm tm;

    if (strptime(string, format, &tm) != NULL)
        return 1;

    return 0;
}

int is_in_range(time_t modification_time, time_t start_time, time_t finish_time)
{
    return (modification_time >= start_time && modification_time <= finish_time);
}

/* Returns last element of string split by given delimeter */
char *get_last_el_of_split(char *path, char *delim)
{
    char *path_copy = strdup(path); // Create a copy of the input string
    char *file_format;
    char *token = strtok(path_copy, delim);
    while (token != NULL)
    {
        file_format = token;
        token = strtok(NULL, delim);
    }
    free(path_copy);
    if (file_format == NULL)
    {
        // Handle the case where no tokens were found
        // (return a default value, print an error, etc.)
    }
    return file_format;
}