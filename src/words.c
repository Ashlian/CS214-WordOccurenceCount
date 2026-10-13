#include "words.h"

#define BUF_SIZE 2048

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

int main(int argc, char **argv) {
    DEBUG_PRINT("running with %d args\n", argc);
    char *suffix = ".txt";
    size_t i = 1;

    // Check for suffix flag
    if (strcmp(argv[1], "-s") == 0) {
        suffix = argv[2];
        i = 3;
    }

    // Iterate through each file
    for (; i < argc; i++) {
        char *path = argv[i];
        struct stat data;

        // Get stat of path
        int res = stat(path, &data);
        if (res < 0) {
            perror(path);
            return EXIT_FAILURE;
        }

        // Check if path links to regular file or directory
        if (S_ISREG(data.st_mode)) {
            read_file(path);
        } else if (S_ISDIR(data.st_mode)) {
            read_dir(path, suffix);
        } else {
            DEBUG_PRINT("Reading other file type: (%s)\n", path);
        }
    }
    return EXIT_SUCCESS;
}

void read_file(char *path) {
    DEBUG_PRINT("Reading file: (%s)...\n", path);

    char buf[BUF_SIZE];
    int bytes;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    bytes = read(fd, buf, BUF_SIZE);
    while (bytes > 0) {
        for (size_t i = 0; i < bytes; i++) {
            /* Process bytes - To be implemented... */
        }

        read(fd, buf, BUF_SIZE);
    }
    close(fd);
}

void read_dir(char *path, char *suffix) {
    DEBUG_PRINT("Reading directory: (%s)...\n", path);

    DIR *dir_ptr = opendir(path);
    if (!dir_ptr) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(dir_ptr))) {
        unsigned file_type = dir_entry->d_type;
        char *file_name = dir_entry->d_name;
        char *first_dot = strrchr(file_name, '.');
        char *cur_suf = "";
        // Skip hidden files
        if (!first_dot || first_dot == file_name) {
            continue;
        }
        // Check suffix match
        cur_suf = first_dot;
        if (strcmp(cur_suf, suffix) != 0) {
            continue;
        }

        // Build relative path
        snprintf(path, strlen(path) + strlen(file_name) + 2, "%s/%s", path, file_name);

        if (file_type == DT_REG) {
            read_file(path);
        } else if (file_type == DT_DIR) {
            read_dir(path, suffix);
        } else {
            DEBUG_PRINT("(read_dir): other file type detected - %s", path);
        }
    }
    closedir(dir_ptr);
}
