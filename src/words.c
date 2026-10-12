#include "words.h"

#define BUF_SIZE 2048

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

int main(int argc, char **argv) {
    char *suffix = ".txt";
    size_t i = 1;
    if (argv[1] == "-s") {
        suffix = argv[2];
        i = 3;
    }
    for (i = 0; i < argc; i++) {
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
            DEBUG_PRINT("Reading file: (%s)...", path);
            read_file(path);
        } else if (S_ISDIR(data.st_mode)) {
            DEBUG_PRINT("Reading directory: (%s)...", path);
            read_dir(path);
        } else {
            DEBUG_PRINT("Reading other file type: (%s)", path);
        }
    }
    return EXIT_SUCCESS;
}

void read_file(char *path) {
    char buf[BUF_SIZE];
    int bytes;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        return EXIT_FAILURE;
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
    DIR *dir_ptr = opendir(path);
    if (!dir_ptr) {
        perror(path);
        return EXIT_FAILURE;
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(dir_ptr))) {
        unsigned file_type = dir_entry->d_type;
        char *path = dir_entry->d_name;
        if (file_type == DT_REG) {
            read_file(path);
        } else if (file_type == DT_DIR) {
            read_dir(path);
        } else {
            DEBUG_PRINT("(read_dir): other file type detected - %s", path);
        }
    }
    closedir(dir_ptr);
}
