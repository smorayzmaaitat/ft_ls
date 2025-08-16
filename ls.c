/* simple_ls_with_perms.c
   Minimal changes from your version: prints type + permissions before name.
   Uses ft_strjoin / ft_strncmp from libft and ft_printf for printing name.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include "libft/libft.h"
#include "ft_printf/include/ft_printf.h"
#include <time.h>
typedef struct PathNode {
    char *path;               
    struct PathNode *next;    
} PathNode;

static PathNode *g_head = NULL;
static PathNode *g_tail = NULL;


static PathNode *create_node(const char *path)
{
    const char *use = (path && *path) ? path : ".";
    char *dup = strdup(use);        
    if (!dup) return NULL;

    PathNode *n = malloc(sizeof(PathNode));

    if (!n) {
        free(dup);
        return NULL;
    }

    n->path = dup;
    n->next = NULL;
    return n;
}


int append_path(const char *path)
{
    PathNode *n = create_node(path);
    if (!n) return -1;
    if (g_tail == NULL) {   
        g_head = g_tail = n;
    } else {
        g_tail->next = n;
        g_tail = n;
    }
    return 0;
}

void free_paths(void)
{
    PathNode *cur = g_head;
    while (cur) {
        PathNode *next = cur->next;
        free(cur->path);
        free(cur);
        cur = next;
    }
    g_head = g_tail = NULL;
}


void print_permissions(mode_t mode)
{
    /* owner */
    putchar((mode & S_IRUSR) ? 'r' : '-');
    putchar((mode & S_IWUSR) ? 'w' : '-');
    if (mode & S_ISUID)
        putchar((mode & S_IXUSR) ? 's' : 'S');
    else
        putchar((mode & S_IXUSR) ? 'x' : '-');

    /* group */
    putchar((mode & S_IRGRP) ? 'r' : '-');
    putchar((mode & S_IWGRP) ? 'w' : '-');
    if (mode & S_ISGID)
        putchar((mode & S_IXGRP) ? 's' : 'S');
    else
        putchar((mode & S_IXGRP) ? 'x' : '-');

    /* others */
    putchar((mode & S_IROTH) ? 'r' : '-');
    putchar((mode & S_IWOTH) ? 'w' : '-');
    if (mode & S_ISVTX)
        putchar((mode & S_IXOTH) ? 't' : 'T');
    else
        putchar((mode & S_IXOTH) ? 'x' : '-');
}

void print_hard_links(const char *path) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        perror(path);
         return;
    }

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    
    printf("  %lu", (unsigned long)st.st_nlink);

    if (pw != NULL) {
        printf(" %s", pw->pw_name);

    }    
    if (gr != NULL) {
        printf(" %s", gr->gr_name);
    }

    printf("  %lld ", (long long)st.st_size);
    char *time_str = ctime(&st.st_mtime);
    char month[4];
    month[0] = time_str[4];
    month[1] = time_str[5];
    month[2] = time_str[6];
    month[3] = '\0';
    char day[3];
    day[0] = time_str[8];
    day[1] = time_str[9];
    day[2] = '\0';
    char hour_min[6];
    hour_min[0] = time_str[11];  
    hour_min[1] = time_str[12];  
    hour_min[2] = ':';
    hour_min[3] = time_str[14];  
    hour_min[4] = time_str[15];  
    hour_min[5] = '\0';

    printf(" %s", month); 
    printf(" %s", day);
    printf(" %s", hour_min);


}


void printl(const char *path, const char *name)
{
    if (name == NULL || path == NULL)
        return;

    char *tmp = ft_strjoin(path, "/");
    if (tmp == NULL) {
        perror("ft_strjoin");
        return;
    }

    char *full_path = ft_strjoin(tmp, name);
    free(tmp);
    if (full_path == NULL) {
        perror("ft_strjoin");
        return;
    }

    struct stat fileStat;

    if (lstat(full_path, &fileStat) < 0) {
        perror(full_path);
        free(full_path);
        return;
    }

    /* file type char */
    if (S_ISREG(fileStat.st_mode)) {
        putchar('-');
    } else if (S_ISDIR(fileStat.st_mode)) {
        putchar('d');
    } else if (S_ISLNK(fileStat.st_mode)) {
        putchar('l');
    } else if (S_ISCHR(fileStat.st_mode)) {
        putchar('c');
    } else if (S_ISBLK(fileStat.st_mode)) {
        putchar('b');
    } else if (S_ISFIFO(fileStat.st_mode)) {
        putchar('p');
    } else if (S_ISSOCK(fileStat.st_mode)) {
        putchar('s');
    } else {
        putchar('?');
    }

    /* permissions rwxrwxrwx (with s/S and t/T handling) */
    print_permissions(fileStat.st_mode);
    print_hard_links(full_path);

    printf("%s\n", name);

    free(full_path);
}

int list_recursive(const char *path)
{
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return 1;
    }

    /* first pass: print type+perms and name for each entry */
    while ((entry = readdir(dir)) != NULL) {
        printl(path, entry->d_name);
    }

    closedir(dir);

    dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (ft_strncmp(entry->d_name, ".", 1) == 0)
            continue;
        if (ft_strncmp(entry->d_name, "..", 2) == 0)
            continue;

        int is_dir = 0;
#ifdef DT_DIR
        if (entry->d_type == DT_DIR)
            is_dir = 1;
        else if (entry->d_type == DT_UNKNOWN)
#endif
        {
            /* build path and lstat to check type */
            char *tmp = ft_strjoin(path, "/");
            if (tmp == NULL) {
                perror("ft_strjoin");
                continue;
            }
            char *fullpath = ft_strjoin(tmp, entry->d_name);
            free(tmp);
            if (fullpath == NULL) {
                perror("ft_strjoin");
                continue;
            }
            struct stat st;
            if (lstat(fullpath, &st) == 0 && S_ISDIR(st.st_mode))
                is_dir = 1;
            free(fullpath);
        }

        if (is_dir) {
            /* build full path for recursion */
            char *tmp2 = ft_strjoin(path, "/");
            if (tmp2 == NULL) {
                perror("ft_strjoin");
                continue;
            }
            char *fullpath2 = ft_strjoin(tmp2, entry->d_name);
            free(tmp2);
            if (fullpath2 == NULL) {
                perror("ft_strjoin");
                continue;
            }

            printf("%s:\n", fullpath2);
           // list_recursive(fullpath2);
            free(fullpath2);
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    return list_recursive(path);
}
