#include "words.h"

#define BUF_SIZE 2048

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

typedef struct {
    char *bytes;
    size_t len;
    size_t cap;
} word;

int main(int argc, char **argv) {
    DEBUG_PRINT("running with %d args\n", argc);
    char *suffix = ".txt";
    size_t i = 1;

    // Check for suffix flag
    if (strcmp(argv[1], "-s") == 0) {
        suffix = argv[2];
        i = 3;
    }

    // Open write file
    int out_fd = open("out.txt", O_WRONLY);
    if (out_fd < 0) {
        perror("out.txt");
        exit(EXIT_FAILURE);
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
            read_file(path, out_fd);
        } else if (S_ISDIR(data.st_mode)) {
            read_dir(path, out_fd, suffix);
        } else {
            DEBUG_PRINT("Reading other file type: (%s)\n", path);
        }
    }

    close(out_fd);

    DEBUG_PRINT("Complete!\n");
    return EXIT_SUCCESS;
}

void read_file(char *path, int out_fd) {
    DEBUG_PRINT("Reading file: %s\n", path);

    char buf[BUF_SIZE];
    int bytes;
    // Open read file
    int in_fd = open(path, O_RDONLY);
    if (in_fd < 0) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    // Process read file
    size_t word_len = 0;
    bytes = read(in_fd, buf, BUF_SIZE);
    while (bytes > 0) {
        for (size_t i = 0; i < bytes; i++) {
            char c = buf[i];
            // Append if word character or hyphen with words surrounding it
            if (is_word_char(c) || (c == '-' && word_len > 0 && is_word_char(buf[i + 1]))) {
                DEBUG_PRINT("%c", c);
                write(out_fd, &c, 1);
                word_len++;
            } else {
                DEBUG_PRINT("\n");
                write(out_fd, "\n", 1);
                word_len = 0;
            }
        }
        bytes = read(in_fd, buf, BUF_SIZE);
    }
    DEBUG_PRINT("\n");
    close(in_fd);
}

void read_dir(char *parent_path, int out_fd, char *suffix) {
    DEBUG_PRINT("Reading directory: %s/\n", parent_path);

    DIR *dir_ptr = opendir(parent_path);
    if (!dir_ptr) {
        perror(parent_path);
        exit(EXIT_FAILURE);
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(dir_ptr))) {
        // Get file type/name
        unsigned file_type = dir_entry->d_type;
        char *file_name = dir_entry->d_name;
        // Build relative path
        size_t path_len = strlen(parent_path) + strlen(file_name) + 2;
        char *full_path = malloc(path_len);
        if (!full_path) {
            perror("malloc failure");
            exit(EXIT_FAILURE);
        }
        snprintf(full_path, path_len, "%s/%s", parent_path, file_name);

        char *dot = strrchr(file_name, '.');
        char *cur_suf = "";
        // Skip hidden files
        if (!dot || file_name[0] == '.') {
            continue;
        }
        // Check suffix match
        cur_suf = dot;
        if (strcmp(cur_suf, suffix) != 0) {
            continue;
        }

        if (file_type == DT_REG) {
            read_file(full_path, out_fd);
        } else if (file_type == DT_DIR) {
            read_dir(full_path, out_fd, suffix);
        } else {
            DEBUG_PRINT("(read_dir): other file type detected - %s", full_path);
        }
    }
    closedir(dir_ptr);
}

int is_word_char(char c) { return isalpha(c) || c == '\''; }