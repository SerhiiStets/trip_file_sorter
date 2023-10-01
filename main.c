#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>

#include "utils.h"
#include "formatter.h"

#define MAX_LEN 256

typedef struct RenamedFile
{
    char path[MAX_LEN];
    char new_name[MAX_LEN];
} RenamedFile;

// Define the function to free the allocated memory for RenamedFile structs
void free_renamed_files(RenamedFile *renamed_files)
{
    free(renamed_files); // Free the allocated memory
}

void resolve_duplicate_names(RenamedFile *renamed_files, int num_files)
{
    for (int i = 0; i < num_files - 1; ++i)
    {
        for (int j = i + 1; j < num_files; ++j)
        {
            if (strcmp(renamed_files[i].new_name, renamed_files[j].new_name) == 0)
            {
                // Found a duplicate new_name entry
                // Rename the duplicate entry
                int counter = 1;
                char base_name[MAX_LEN];
                char file_format[MAX_LEN];
                // Split the new_name into base_name and file_format before the dot
                sscanf(renamed_files[j].new_name, "%[^.].%s", base_name, file_format);
                char new_name_with_counter[MAX_LEN];
                while (1)
                {
                    snprintf(new_name_with_counter, sizeof(new_name_with_counter), "%s_%d.%s", base_name, counter, file_format);
                    int is_unique = 1;
                    // Check if the new name with the counter is unique
                    for (int k = 0; k < num_files; ++k)
                    {
                        if (k != j && strcmp(renamed_files[k].new_name, new_name_with_counter) == 0)
                        {
                            is_unique = 0;
                            break;
                        }
                    }
                    if (is_unique)
                    {
                        snprintf(renamed_files[j].new_name, sizeof(renamed_files[j].new_name), "%s", new_name_with_counter);
                        printf("Renamed duplicate %s\n", renamed_files[j].new_name);
                        break;
                    }
                    ++counter;
                }
            }
        }
    }
}

void rename_file_and_add_entry(const char *dir_path, const char *file_path, const char *formatted_time, const char *file_format, RenamedFile **renamed_files, int *num_files)
{
    char new_name[MAX_LEN];
    snprintf(new_name, sizeof(new_name), "%s/%s.%s", dir_path, formatted_time, file_format);
    printf("%s -> %s\n", file_path, new_name);

    RenamedFile new_entry;
    snprintf(new_entry.path, sizeof(new_entry.path), "%s", file_path);
    snprintf(new_entry.new_name, sizeof(new_entry.new_name), "%s.%s", formatted_time, file_format);

    *renamed_files = realloc(*renamed_files, (*num_files + 1) * sizeof(RenamedFile));
    if (*renamed_files == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    (*renamed_files)[*num_files] = new_entry;
    (*num_files)++;
}

RenamedFile *process_directory(const char *dir_path, const time_t start_time, const time_t end_time, int *num_renamed_files)
{
    DIR *dir = opendir(dir_path);
    struct dirent *ent;

    if (dir == NULL)
    {
        fprintf(stderr, "Unable to open directory: %s\n", dir_path);
        return NULL;
    }

    RenamedFile *renamed_files = NULL;
    int num_files = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        char file_path[MAX_LEN];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, ent->d_name);

        char *file_format = get_last_el_of_split(file_path, ".");

        // Check if file has correct format
        if (is_valid_format(file_path, file_format) == 0)
            continue;

        const char *datetime_matched_pattern = get_datetime_regex_pattern(ent->d_name);

        if (datetime_matched_pattern != NULL)
        {
            // Check if file already has a correct %Y%m%d_%H%M%S format
            if (has_correct_date_format(ent->d_name, "%Y%m%d_%H%M%S"))
                continue;

            // Convert incorrect datetime format to correct one
            char *formatted_time = convert_to_new_format(ent->d_name, datetime_matched_pattern);

            // For some reason unkown to me if you don't rerun file_format function
            // some file_path formats would be corrupted after convert_to_new-format call
            // TODO: solve why and remove this duplicated call
            file_format = get_last_el_of_split(file_path, ".");

            if (formatted_time != NULL)
            {
                rename_file_and_add_entry(dir_path, file_path, formatted_time, file_format, &renamed_files, &num_files);
            }
            free(formatted_time);
        }
        else
        {
            struct stat file_stat;

            if (stat(file_path, &file_stat) != 0)
                continue;

            time_t modification_time = file_stat.st_mtime;

            if (is_in_range(modification_time, start_time, end_time))
            {
                // File modified time is within the specified range
                struct tm *timeinfo = localtime(&modification_time);
                char *formatted_time = malloc(18); // Allocate memory for the formatted time (YYYYMMDD_HHMMSS\0)
                // For some reason unkown to me if you don't rerun file_format function
                // some file_path formats would be corrupted after convert_to_new-format call
                // TODO: solve why and remove this duplicated call
                file_format = get_last_el_of_split(file_path, ".");

                if (formatted_time != NULL)
                {
                    strftime(formatted_time, 18, "%Y%m%d_%H%M%S", timeinfo);
                    rename_file_and_add_entry(dir_path, file_path, formatted_time, file_format, &renamed_files, &num_files);
                    free(formatted_time);
                }
                else
                    fprintf(stderr, "Memory allocation error\n");
            }
            else
            {
                printf("NOT In range: %s - %s\n", file_path, ctime(&modification_time));
            }
        }
    }
    closedir(dir);
    *num_renamed_files = num_files;

    resolve_duplicate_names(renamed_files, num_files);

    return renamed_files;
}

int main()
{
    return 0;
}