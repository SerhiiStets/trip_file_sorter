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

#define MAX_LEN 256
#define len(x) (sizeof(x) / sizeof(x[0]))

/* Check if given path is a file of correct format */
int is_valid_format(char *path, char *file_format)
{
    char allowed_formats[][MAX_LEN] = {"JPG", "jpg", "MP4", "HEIC", "mp4", "MOV"};
    // Chech if the path is a file
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
        return 0; // Error occurred, treat as not a regular file

    if (S_ISREG(path_stat.st_mode) == 0)
        return 0; // Not a file

    // Check if file is of selected format
    for (int i = 0; i < len(allowed_formats); i++)
    {
        if (strcmp(allowed_formats[i], file_format) == 0)
            return 1;
    }

    printf("Wrong format:\n%s\n\n", file_format);
    return 0;
}

char *convert_to_new_format(const char *filename, const char *matched_regex)
{
    regex_t regex;
    regmatch_t matches[7];

    if (regcomp(&regex, matched_regex, REG_EXTENDED) != 0)
    {
        fprintf(stderr, "Failed to compile regular expression\n");
        return NULL;
    }

    if (regexec(&regex, filename, 7, matches, 0) == 0)
    {
        char year[5], month[3], day[3], hour[3], minute[3], second[3];
        strncpy(year, filename + matches[1].rm_so, 4);
        strncpy(month, filename + matches[2].rm_so, 2);
        strncpy(day, filename + matches[3].rm_so, 2);
        strncpy(hour, filename + matches[4].rm_so, 2);
        strncpy(minute, filename + matches[5].rm_so, 2);
        strncpy(second, filename + matches[6].rm_so, 2);

        year[4] = month[2] = day[2] = hour[2] = minute[2] = second[2] = '\0';

        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(struct tm));
        timeinfo.tm_year = atoi(year) - 1900;
        timeinfo.tm_mon = atoi(month) - 1;
        timeinfo.tm_mday = atoi(day);
        timeinfo.tm_hour = atoi(hour);
        timeinfo.tm_min = atoi(minute);
        timeinfo.tm_sec = atoi(second);

        char *formatted_time = malloc(16);
        if (formatted_time == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            regfree(&regex);
            return NULL;
        }

        strftime(formatted_time, 16, "%Y%m%d_%H%M%S", &timeinfo);
        regfree(&regex);
        return formatted_time;
    }
    regfree(&regex);
    return NULL; // No match found
}

int rename_file(char *file_path, char *new_name)
{
    // Rename the file using the rename() function
    if (rename(file_path, new_name) != 0)
    {
        printf("Error renaming file");
        return 0;
    }
    else
    {
        printf("Renamed: %s -> %s\n", file_path, new_name);
        return 1;
    }
}

const char *get_datetime_regex_pattern(const char *input)
{
    const char *regex_patterns[3] = {
        ".*([0-9]{4})-([0-9]{2})-([0-9]{2})_([0-9]{2})-([0-9]{2})-([0-9]{2})",
        ".*([0-9]{4})([0-9]{2})([0-9]{2})_([0-9]{2})([0-9]{2})([0-9]{2})",
        ".*([0-9]{4})-([0-9]{2})-([0-9]{2})-([0-9]{2})h([0-9]{2})m([0-9]{2})s",
    };

    regex_t regex;
    for (int i = 0; i < 3; i++)
    {
        int ret = regcomp(&regex, regex_patterns[i], REG_EXTENDED);
        if (ret != 0)
        {
            char error_message[MAX_LEN];
            regerror(ret, &regex, error_message, MAX_LEN);
            fprintf(stderr, "Failed to compile regular expression: %s\n", error_message);
            continue;
        }

        if (regexec(&regex, input, 0, NULL, 0) == 0)
        {
            regfree(&regex);
            return regex_patterns[i]; // Match found, return the regex pattern
        }
        regfree(&regex);
    }

    return NULL; // No match found, return NULL
}

int process_directory(const char *dir_path, const time_t start_time, const time_t end_time)
{
    DIR *dir;
    struct dirent *ent;

    dir = opendir(dir_path);
    if (dir == NULL)
    {
        printf("Unable to open directory: %s\n", dir_path);
        return 1;
    }
    printf("Folder found!\n");

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
                char new_name[MAX_LEN];
                snprintf(new_name, sizeof(new_name), "%s/%s.%s", dir_path, formatted_time, file_format);
                printf("%s -> %s\n", ent->d_name, new_name);
                // rename_file(file_path, new_name);
            }
            free(formatted_time);
        }
        else
        {
            struct stat file_stat;

            if (stat(file_path, &file_stat) == 0)
            {
                time_t modification_time = file_stat.st_mtime;
                // printf("Last modification time of %s: %s", file_path, ctime(&modification_time));
                if (is_in_range(modification_time, start_time, end_time))
                {
                    printf("In range: %s\n", file_path);
                }
                else
                {
                    printf("NOT In range: %s - %s\n", file_path, ctime(&modification_time));
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("The correct format is : %s <file path> <start_date> <end_date>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct tm start_tm, end_tm;
    memset(&start_tm, 0, sizeof(struct tm));
    memset(&end_tm, 0, sizeof(struct tm));

    if (has_correct_date_format(argv[2], "%Y%m%d") == 0 || has_correct_date_format(argv[3], "%Y%m%d") == 0)
    {
        printf("Invalid date format. Please use YYYYMMDD format.\n");
        return EXIT_FAILURE;
    }

    time_t start_time = mktime(&start_tm);
    time_t end_time = mktime(&end_tm);

    process_directory(argv[1], start_time, end_time);

    return EXIT_SUCCESS;
}
