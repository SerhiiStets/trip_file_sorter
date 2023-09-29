#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>

#define MAX_LEN 256
#define len(x) (sizeof(x) / sizeof(x[0]))

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

/* Check if file has correct date time format in name */
int has_correct_date_format(const char *file_name)
{
    struct tm tm;

    if (strptime(file_name, "%Y%m%d_%H%M%S", &tm) != NULL)
        return 1;

    return 0;
}

int has_datetime_format(const char *filename)
{
    regex_t regex;
    if (regcomp(&regex, ".*_([0-9]{4})-([0-9]{2})-([0-9]{2})_([0-9]{2})-([0-9]{2})-([0-9]{2})", REG_EXTENDED) != 0)
    {
        perror("Failed to compile regular expression");
        return 0;
    }

    if (regexec(&regex, filename, 0, NULL, 0) == 0)
    {
        regfree(&regex);
        return 1; // Match found
    }

    regfree(&regex);
    return 0; // No match found
}

char *convert_to_new_format(const char *filename)
{
    regex_t regex;
    regmatch_t matches[7];

    if (regcomp(&regex, ".*_([0-9]{4})-([0-9]{2})-([0-9]{2})_([0-9]{2})-([0-9]{2})-([0-9]{2})\\.jpg", REG_EXTENDED) != 0)
    {
        perror("Failed to compile regular expression");
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
            perror("Memory allocation error");
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
    }
    else
    {
        printf("Renamed: %s -> %s\n", file_path, new_name);
    }
}

int process_directory(const char *dir_path)
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
        char *file_format;

        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, ent->d_name);
        file_format = get_last_el_of_split(file_path, ".");

        // Check if 'ent' represents a regular file (not a directory)
        if (is_valid_format(file_path, file_format) == 0)
            continue;

        if (has_datetime_format(ent->d_name))
        {
            // Check if file already has a correct %Y%m%d_%H%M%S format
            if (has_correct_date_format(ent->d_name))
                continue;

            // Convert incorrect datetime format to correct one
            char *formatted_time = convert_to_new_format(ent->d_name);
            if (formatted_time == NULL)
                continue;
            char new_name[MAX_LEN];
            snprintf(new_name, sizeof(new_name), "%s/%s.%s", dir_path, formatted_time, file_format);
            printf("%s -> %s\n", ent->d_name, new_name);
            // rename_file(file_path, new_name);
            free(formatted_time);
        }
        else
        {
            // TODO: check modification time and rename using this parameter
            struct stat file_stat;

            if (stat(file_path, &file_stat) != 0)
            {
                continue;
            }

            time_t modification_time = file_stat.st_mtime;
            printf("Last modification time of %s: %s", file_path, ctime(&modification_time));
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("The correct format is : %s <file path>", argv[0]);
        return EXIT_FAILURE;
    }

    process_directory(argv[1]);

    return EXIT_SUCCESS;
}