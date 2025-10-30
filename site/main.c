#include <stdio.h>
#include <dirent.h>

int main() {
    DIR *dirp;
    struct dirent *entry;

    // Open the current directory
    dirp = opendir("."); 

    if (dirp == NULL) {
        perror("Error opening directory");
        return 1;
    }

    // Read entries from the directory
    while ((entry = readdir(dirp)) != NULL) {
        printf("Name: %s, Inode: %ld\n, d_off: %ld\n", entry->d_name, (long)entry->d_ino, entry->d_off);
    }

    // Close the directory stream
    closedir(dirp);

    return 0;
}