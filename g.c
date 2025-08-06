#include <stdio.h>
#include <grp.h>
#include <unistd.h>

int main() {
    gid_t gid = getgid();  // Get current user's GID
    struct group *grp = getgrgid(1);

    if (grp) {
        printf("Group name: %s\n", grp->gr_name);
        printf("Group ID: %d\n", grp->gr_gid);
    } else {
        perror("getgrgid");
    }

    return 0;
}
