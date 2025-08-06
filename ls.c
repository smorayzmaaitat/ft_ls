#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libft/libft.h"

int list_recursive(char *path)
{
    struct dirent *entry;
    DIR *dir = opendir(path);

    // Check if directory opened successfully
    if (dir == NULL) {
        perror("");
        return 1;
    }

    printf("\n%s:\n", path); // Print the current directory path

    // Iterate through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (ft_strncmp(entry->d_name, ".",1) == 0 || ft_strncmp(entry->d_name, "..",2) == 0) {
            continue; // Skip "." and ".." entries
        }

        printf("%s\n", entry->d_name);

        // If entry is a directory, recursively list its contents
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (stat(full_path, &entry_stat) == 0 && S_ISDIR(entry_stat.st_mode)) {
            list_recursive(full_path);
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[]) {
    
    // Use current directory if no directory is specified
     char *path = (argc > 1) ? argv[1] : ".";

    list_recursive(path);

    return 0;
}


