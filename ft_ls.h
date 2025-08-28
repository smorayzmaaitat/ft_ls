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
    char *name;
    char *parent_name;
    time_t mtime_sec;
    long   mtime_nsec;
    struct PathNode *next;
} PathNode;


typedef struct s_flags {
    int l;
    int a;
    int r;
    int R;
    int t;
}   t_flags;


static PathNode *g_head = NULL;
static PathNode *g_tail = NULL;
t_flags flags = {0};