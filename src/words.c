#include "words.h"

#define BUF_SIZE 2048
#define TEMP_FILE "temp.txt"

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

    // Open write file
    int out_fd = open(TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (out_fd < 0) {
        perror("open temp file failed in main");
        exit(EXIT_FAILURE);
    }
    Word *word = word_init(BUF_SIZE); // Init out_buffer

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
            read_file(path, out_fd, word);
        } else if (S_ISDIR(data.st_mode)) {
            read_dir(path, out_fd, word, suffix);
        } else {
            DEBUG_PRINT("Reading other file type: (%s)\n", path);
        }
    }

    close(out_fd);
    word_free(word);
    sort_and_count();
    DEBUG_PRINT("\nComplete!\n");
    return EXIT_SUCCESS;
}

void read_file(const char *path, int out_fd, Word *word) {
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
    bytes = read(in_fd, buf, BUF_SIZE);
    while (bytes > 0) {
        for (size_t i = 0; i < bytes; i++) {
            char c = buf[i];
            char next_c = '\0';
            if (i + 1 < bytes) {
                next_c = buf[i + 1];
            }

            if (isalpha(c)) {
                // 1. Regular alphabet
                word_append(word, c);
            } else if (c == '\'' && (word->len > 0 || isalpha(next_c))) {
                // 2. Apostrophe within/around word
                word_append(word, c);
            } else if (c == '-' && word->len > 0 && isalpha(next_c)) {
                // 3. Dash within word
                word_append(word, c);
            } else {
                // 4. Endpoints
                word_write(word, out_fd);
            }
        }
        bytes = read(in_fd, buf, BUF_SIZE);
    }
    DEBUG_PRINT("\n");
    close(in_fd);
}

void read_dir(const char *parent_path, int out_fd, Word *word, const char *suffix) {
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
            read_file(full_path, out_fd, word);
        } else if (file_type == DT_DIR) {
            read_dir(full_path, out_fd, word, suffix);
        } else {
            DEBUG_PRINT("(read_dir): other file type detected - %s", full_path);
        }
        free(full_path);
    }
    closedir(dir_ptr);
}

void sort_and_count() {
    int p1[2], p2[2], p3[2];
    if (pipe(p1) < 0 || pipe(p2) < 0 || pipe(p3) < 0) {
        perror("error: pipe creation failed");
        exit(EXIT_FAILURE);
    }
    // 1. Sort the file to prepare for uniq -c
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("error: fork 1 failed");
        exit(EXIT_FAILURE);
    }
    if (pid1 == 0) {
        int in_fd = open(TEMP_FILE, O_RDONLY);
        if (in_fd < 0) {
            perror("open temp file failed in child 1");
            exit(EXIT_FAILURE);
        }
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
        dup2(p1[1], STDOUT_FILENO);
        close_pipes(p1, p2, p3);
        execlp("sort", "sort", NULL);
        abort();
    }
    // 2. Count num occurences of each word
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("error: fork 2 failed");
        exit(EXIT_FAILURE);
    }
    if (pid2 == 0) {
        dup2(p1[0], STDIN_FILENO);
        dup2(p2[1], STDOUT_FILENO);
        close_pipes(p1, p2, p3);
        execlp("uniq", "uniq", "-c", NULL);
        abort();
    }
    // 3. Sort by count then by word
    pid_t pid3 = fork();
    if (pid3 < 0) {
        perror("error: fork 3 failed");
        exit(EXIT_FAILURE);
    }
    if (pid3 == 0) {
        dup2(p2[0], STDIN_FILENO);
        dup2(p3[1], STDOUT_FILENO);
        close_pipes(p1, p2, p3);
        setenv("LC_ALL", "C", 1);
        execlp("sort", "sort", "-k1,1nr", "-k2,2", NULL);
        abort();
    }
    // 4. Switch word and counte columns
    pid_t pid4 = fork();
    if (pid4 < 0) {
        perror("error: fork 4 failed");
        exit(EXIT_FAILURE);
    }
    if (pid4 == 0) {
        dup2(p3[0], STDIN_FILENO);
        close_pipes(p1, p2, p3);
        execlp("awk", "awk", "{print $2, $1}", NULL);
        abort();
    }
    close_pipes(p1, p2, p3);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    waitpid(pid4, NULL, 0);
}

void close_pipes(int p1[2], int p2[2], int p3[2]) {
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    close(p3[0]);
    close(p3[1]);
}

Word *word_init(size_t cap) {
    Word *word = malloc(sizeof(Word));
    if (!word) {
        perror("error: word_init failed");
        exit(EXIT_FAILURE);
    }

    word->len = 0;
    word->cap = cap;
    word->bytes = malloc(cap);
    if (!word->bytes) {
        perror("error: word_init failed");
        free(word);
        exit(EXIT_FAILURE);
    }

    return word;
}

void word_append(Word *word, char c) {
    if (word->len + 1 >= word->cap) {
        word->cap = word->cap * 2;

        char *new_bytes = realloc(word->bytes, word->cap);
        if (!new_bytes) {
            perror("error: resizing word failed in word_append()");
            exit(EXIT_FAILURE);
        }
        word->bytes = new_bytes;
    }
    word->bytes[word->len++] = c;
}

void word_write(Word *word, int out_fd) {
    if (word->len == 0)
        return;
    // Append new line before write
    word_append(word, '\n');
    // Write to TEMP_FILE
    write(out_fd, word->bytes, word->len);
    word->len = 0; // Reset word length
}

void word_free(Word *word) {
    if (!word) {
        perror("error: free_word() called with null pointer");
        exit(EXIT_FAILURE);
    }
    free(word->bytes);
    free(word);
}