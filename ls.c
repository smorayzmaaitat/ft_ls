#included "ft_ls.h"

static PathNode *g_head = NULL;
static PathNode *g_tail = NULL;



int parse_args(int argc, char **argv, t_flags *flags, char ***paths, int *path_count) {
    if (!flags || !paths || !path_count) return -1;
    
    flags->l = 0;
    flags->a = 0;
    flags->r = 0;
    flags->R = 0;
    flags->t = 0;
    
    *paths = NULL;
    *path_count = 0;
    
    char **path_list = malloc(sizeof(char*) * argc);
    if (!path_list) return -1;
    
    int i = 1;
    while (i < argc) {
        if (argv[i][0] == '-' && argv[i][1]) {

            int j = 1;
            while (argv[i][j]) {
                if (argv[i][j] == 'l') flags->l = 1;
                else if (argv[i][j] == 'a') flags->a = 1;
                else if (argv[i][j] == 'r') flags->r = 1;
                else if (argv[i][j] == 'R') flags->R = 1;
                else if (argv[i][j] == 't') flags->t = 1;
                else {
                    free(path_list);
                    return -1;
                }
                j++;
            }
        } else {
            path_list[*path_count] = argv[i];
            (*path_count)++;
        }
        i++;
    }
    
    if (*path_count == 0) {
        path_list[0] = ".";
        *path_count = 1;
    }
    
    *paths = path_list;
    return 0;
}


static const char *get_basename(const char *path) {
    if (!path || !*path) return NULL;
    
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        return last_slash + 1;
    }
    
    return path; 
}

static PathNode *create_node(const char *path) {
    if (!path) return NULL;
    
    PathNode *n = malloc(sizeof(PathNode));
    if (!n) return NULL;
    
    n->path = strdup(path);
    if (!n->path) {
        free(n);
        return NULL;
    }
    

    n->name = strdup(get_basename(path));
    if (!n->name) {
        free(n->path);
        free(n);
        return NULL;
    }

    struct stat st;
    if (lstat(path, &st) == 0) {
        n->mtime_sec = st.st_mtime;
        #ifdef __APPLE__
            n->mtime_nsec = st.st_mtimespec.tv_nsec;
        #elif defined(__linux__)
            n->mtime_nsec = st.st_mtim.tv_nsec;
        #else
            n->mtime_nsec = 0;
        #endif
    } else {
        n->mtime_sec = 0;
        n->mtime_nsec = 0;
    }
    
    n->next = NULL;
    return n;
}



int append_path(const char *path) {
    PathNode *n = create_node(path);
    if (!n) return -1;
    
    if (!g_tail) {
        g_head = g_tail = n;
    } else {
        g_tail->next = n;
        g_tail = n;
    }
    return 0;
}


static void free_node(PathNode *node) {
    if (node) {

        free(node->path);
        free(node->name);
        free(node);
    }
}



void free_path_list(void) {
    PathNode *current = g_head;
    while (current) {
        PathNode *next = current->next;
        free_node(current);
        current = next;
    }
    g_head = g_tail = NULL;
}


void print_path_list(void) {
    PathNode *current = g_head;
    printf("Path List:\n");
    while (current) {
        printf("  Path: %s\n", current->path);
        printf("  Name: %s\n", current->name);
        printf("  Modified: %ld.%09ld\n", current->mtime_sec, current->mtime_nsec);
        printf("  ---\n");
        current = current->next;
    }
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
