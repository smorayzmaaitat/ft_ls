#include "ft_ls.h"

static PathNode *g_head = NULL;
static PathNode *g_tail = NULL;
t_flags flags = {0};

int parse_args(int argc, char **argv, char ***paths, int *path_count) {
    if (!paths || !path_count || !argv) return -1;
    
    *paths = NULL;
    *path_count = 0;
    
    // Allocate extra space for NULL terminator
    char **path_list = malloc(sizeof(char*) * (argc + 1));
    if (!path_list) return -1;
    
    int i = 1;
    while (i < argc) {
        // Check if it's a flag
        if (argv[i][0] == '-' && argv[i][1] != '\0' && argv[i][1] != '-') {
            int j = 1;
            while (argv[i][j]) {
                if (argv[i][j] == 'l') flags.l = 1;
                else if (argv[i][j] == 'a') flags.a = 1;
                else if (argv[i][j] == 'r') flags.r = 1;
                else if (argv[i][j] == 'R') flags.R = 1;
                else if (argv[i][j] == 't') flags.t = 1;
                else {
                    // Invalid flag
                    free(path_list);
                    return -1;
                }
                j++;
            }
        } else {
            // It's a path (not a flag)
            if (*path_count >= argc) {
                free(path_list);
                return -1;
            }
            path_list[*path_count] = argv[i];
            (*path_count)++;
        }
        i++;
    }
    
    // If no paths provided, use current directory
    if (*path_count == 0) {
        path_list[0] = ".";
        *path_count = 1;
    }
    
    // Add NULL terminator
    path_list[*path_count] = NULL;
    
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

static PathNode *create_node(char *path, char *parent_name) {
    if (!path) return NULL;
   
    PathNode *n = malloc(sizeof(PathNode));
    if (!n) return NULL;
    
    n->path = path;
    if (!n->path) {
        free(n);
        return NULL;
    }
    
    n->parent_name = parent_name;

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

int append_path(char *path, char *parent_name) {
    if (!path) return -1;
    PathNode *n = create_node(path, parent_name);
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

long calculate_total(char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;
    
    struct dirent *entry;
    long total_blocks = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (!flags.a && entry->d_name[0] == '.') 
            continue;

        char *tmp;
        if (path[strlen(path) - 1] == '/') {
            tmp = strdup(path);  
        } else {
            tmp = ft_strjoin(path, "/"); 
        }
        if (!tmp) continue;
        
        char *fullpath = ft_strjoin(tmp, entry->d_name);
        free(tmp);
        if (!fullpath) continue;
        
        struct stat st;
        if (lstat(fullpath, &st) == 0) {
            total_blocks += st.st_blocks;
        }
        
        free(fullpath);
    }
    
    closedir(dir);
    return total_blocks;
}


void sort_tmp_by_time(PathNode **head) {
    if (!head || !*head) return;
     
    PathNode *i = *head;
    while (i) {
        PathNode *j = i->next;

        

        while (j) {
            if (i->mtime_sec < j->mtime_sec || 
                (i->mtime_sec == j->mtime_sec && i->mtime_nsec < j->mtime_nsec) || 
                (i->mtime_sec == j->mtime_sec && i->mtime_nsec == j->mtime_nsec && strcmp(i->name, j->name) > 0)) {
              
                char *temp_path = i->path;
                char *temp_name = i->name;
                char *temp_parent = i->parent_name;
                time_t temp_sec = i->mtime_sec;
                long temp_nsec = i->mtime_nsec;
                
                i->path = j->path;
                i->name = j->name;
                i->parent_name = j->parent_name;
                i->mtime_sec = j->mtime_sec;
                i->mtime_nsec = j->mtime_nsec;
                
                j->path = temp_path;
                j->name = temp_name;
                j->parent_name = temp_parent;
                j->mtime_sec = temp_sec;
                j->mtime_nsec = temp_nsec;
            }
            j = j->next;
        }
        i = i->next;
    }
}

void reverse_order(PathNode **head)
{
    if (!head || !*head) return;

    PathNode *prev = NULL;
    PathNode *current = *head;
    PathNode *next = NULL;

    while (current) {
        next = current->next;  
        current->next = prev;  
        prev = current;        
        current = next;       
    }
    *head = prev;  
}


void sort_tmp_by_name(PathNode **head) {
    if (!head || !*head) return;
    
    PathNode *i = *head;
    while (i) {
        PathNode *j = i->next;
        while (j) {
            if (strcmp(i->name, j->name) > 0) {
                // Swap everything
                char *temp_path = i->path;
                char *temp_name = i->name;
                char *temp_parent = i->parent_name;
                time_t temp_sec = i->mtime_sec;
                long temp_nsec = i->mtime_nsec;
                
                i->path = j->path;
                i->name = j->name;
                i->parent_name = j->parent_name;
                i->mtime_sec = j->mtime_sec;
                i->mtime_nsec = j->mtime_nsec;
                
                j->path = temp_path;
                j->name = temp_name;
                j->parent_name = temp_parent;
                j->mtime_sec = temp_sec;
                j->mtime_nsec = temp_nsec;
            }
            j = j->next;
        }
        i = i->next;
    }
}


void add_tmp_to_global(PathNode *tmp_head) {
    PathNode *current = tmp_head;
    while (current) {
        PathNode *next = current->next;
        current->next = NULL;  

        if (!g_tail) {
            g_head = g_tail = current;
        } else {
            g_tail->next = current;
            g_tail = current;
        }
        current = next;
    }
}

void ft_putchar(char c) {
    write(1, &c, 1);
}

void ft_putstr(char *str) {
    if (!str) return;
    while (*str) {
        ft_putchar(*str);
        str++;
    }
}

void ft_putnbr(long long n) {
    if (n < 0) {
        ft_putchar('-');
        n = -n;
    }
    if (n >= 10) {
        ft_putnbr(n / 10);
    }
    ft_putchar((n % 10) + '0');
}

void print_type_suffix(char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        ft_putstr("/");
    } else if (S_ISLNK(st.st_mode)) {
        if (flags.l) {
            char link_target[256];
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len > 0) {
                link_target[len] = '\0';
                ft_putstr("@ -> ");
                ft_putstr(link_target);
            }
        } else {
            ft_putstr("@");
        }
    } else if (S_ISFIFO(st.st_mode)) {
        ft_putstr("|");
    } else if (S_ISSOCK(st.st_mode)) {
        ft_putstr("=");
    } else if (S_ISREG(st.st_mode) &&
               (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
        ft_putstr("*");
    }
}

void print_normal(char *name, char *path) {
    if (!name || !path) return;

    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return;
    }
    ft_putstr(name);
    print_type_suffix(path);
    ft_putstr(" ");
}

void printl(char *path) {
    struct stat st;
    if (lstat(path, &st) < 0) {
        perror(path);
        return;
    }

    if (S_ISREG(st.st_mode)) ft_putchar('-');
    else if (S_ISDIR(st.st_mode)) ft_putchar('d');
    else if (S_ISLNK(st.st_mode)) ft_putchar('l');
    else if (S_ISCHR(st.st_mode)) ft_putchar('c');
    else if (S_ISBLK(st.st_mode)) ft_putchar('b');
    else if (S_ISFIFO(st.st_mode)) ft_putchar('p');
    else if (S_ISSOCK(st.st_mode)) ft_putchar('s');
    else ft_putchar('?');

    // Permissions
    ft_putchar((st.st_mode & S_IRUSR) ? 'r' : '-');
    ft_putchar((st.st_mode & S_IWUSR) ? 'w' : '-');
    ft_putchar((st.st_mode & S_IXUSR) ? 'x' : '-');
    ft_putchar((st.st_mode & S_IRGRP) ? 'r' : '-');
    ft_putchar((st.st_mode & S_IWGRP) ? 'w' : '-');
    ft_putchar((st.st_mode & S_IXGRP) ? 'x' : '-');
    ft_putchar((st.st_mode & S_IROTH) ? 'r' : '-');
    ft_putchar((st.st_mode & S_IWOTH) ? 'w' : '-');
    ft_putchar((st.st_mode & S_IXOTH) ? 'x' : '-');

    // Links count
    ft_putchar(' ');
    ft_putnbr((unsigned long)st.st_nlink);

    // Owner and group
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    
    ft_putchar(' ');
    if (pw) ft_putstr(pw->pw_name);
    else ft_putnbr(st.st_uid);
    
    ft_putchar(' ');
    if (gr) ft_putstr(gr->gr_name);
    else ft_putnbr(st.st_gid);

    // File size
    ft_putchar(' ');
    ft_putnbr((long long)st.st_size);

    // Date and time
    char *time_str = ctime(&st.st_mtime);
    
    ft_putchar(' ');
    // Print month (chars 4-6)
    ft_putchar(time_str[4]);
    ft_putchar(time_str[5]);
    ft_putchar(time_str[6]);
    
    ft_putchar(' ');
    // Print day (chars 8-9)
    ft_putchar(time_str[8]);
    ft_putchar(time_str[9]);
    
    ft_putchar(' ');
    // Print time (chars 11-15)
    ft_putchar(time_str[11]);
    ft_putchar(time_str[12]);
    ft_putchar(':');
    ft_putchar(time_str[14]);
    ft_putchar(time_str[15]);

    // File name (basename)
    char *name = strrchr(path, '/');
    if (name) name++;
    else name = path;
    
    ft_putchar(' ');
    ft_putstr(name);

    print_type_suffix(path);

    ft_putchar('\n');
}

void print_total(char *path) {
    long total = calculate_total(path);
    ft_putstr("total ");
    ft_putnbr(total);
    ft_putchar('\n');
}

void print_path_list(void) {
    PathNode *current = g_head;
    char *current_parent = "";

    while (current) {
        if ((ft_strncmp(current->parent_name, current_parent, ft_strlen(current->parent_name)) != 0 || !current_parent)) {
            if (flags.R && current_parent[0] != '\0') {
                ft_putstr("\n\n");
                ft_putstr(current->parent_name);
                ft_putstr(":\n");
            }
            if (flags.l)
                print_total(current->parent_name);
            current_parent = current->parent_name;
        }

        if (flags.l && strcmp(current->path, "Non") != 0)
            printl(current->path);

        if (!flags.l && strcmp(current->path, "Non") != 0)
            print_normal(current->name, current->path);

        current = current->next;
    }
}

int list_recursive(char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return 1;
    }

    PathNode *tmp_head = NULL;
    PathNode *tmp_tail = NULL;
    int dir_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!flags.a && entry->d_name[0] == '.') 
            continue;

        dir_count++;
        char *tmp;
        char *fullpath;
        if (path[strlen(path) - 1] == '/') {
            tmp = strdup(path);  
        } else {
            tmp = ft_strjoin(path, "/"); 
        }

        if (!tmp) return 1;

        fullpath = ft_strjoin(tmp, entry->d_name);
        free(tmp);
        if (!fullpath) return 1;

        PathNode *node = create_node(fullpath, path);
        if (node) {
            if (!tmp_tail) {
                tmp_head = tmp_tail = node;
            } else {
                tmp_tail->next = node;
                tmp_tail = node;
            }
        }
    }


    if (flags.t) {
        sort_tmp_by_time(&tmp_head);
    } else {
        sort_tmp_by_name(&tmp_head);
    }

    add_tmp_to_global(tmp_head);

    if (dir_count == 0) {
        append_path(strdup("Non"), path);
    }

    closedir(dir);

     

    if (flags.R) {
        PathNode *current = g_head;
        while (current) {
            if (strcmp(current->parent_name, path) == 0) {
                struct stat st;
                if (lstat(current->path, &st) == 0 && S_ISDIR(st.st_mode)) {
                    if (strcmp(current->name, ".") != 0 && strcmp(current->name, "..") != 0) {
                        list_recursive(current->path);
                    }
                }
            }
            current = current->next;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char **paths = NULL;
    int path_count = 0;
    parse_args(argc, argv, &paths, &path_count);

    while (path_count > 0) {
        char *current_path = paths[--path_count];
        list_recursive(current_path);
    }

    print_path_list();
    free_path_list();
    free(paths);

    return 0;
}