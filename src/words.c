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

    char out_file[] = "output_XXXXXX";
    int out_fd = -1;
    Word *word = NULL;
    int status = EXIT_FAILURE;

    // Check for suffix flag
    if (argc > 1 && strncmp(argv[1], "-s", 2) == 0) {
        if (strlen(argv[1]) > 2) {
            suffix = argv[1] + 2;
            i = 2;
        } else {
            if (argc < 3) {
                perror("Missing suffix argument");
                goto cleanup;
            }

            suffix = argv[2];
            i = 3;
        }
    }

    // Open write file
    out_fd = mkstemp(out_file);
    if (out_fd < 0) {
        perror("mkstemp failed");
        goto cleanup;
    }
    DEBUG_PRINT("Out filename is %s\n", out_file);

    // Init write buffer
    word = word_init(BUF_SIZE);
    if (!word) {
        perror("error: word_init failed");
        goto cleanup;
    }

    // Iterate through each file
    for (; i < argc; i++) {
        char *file = argv[i];
        struct stat data;

        // Get stat of file
        int res = stat(file, &data);
        if (res < 0) {
            perror(file);
            goto cleanup;
        }
        // Check if regular file or directory
        if (S_ISREG(data.st_mode)) {

            if (read_file(file, out_fd, word) < 0) {
                goto cleanup;
            }
        } else if (S_ISDIR(data.st_mode)) {
            if (read_dir(file, out_fd, word, suffix) < 0) {
                goto cleanup;
            }
        } else {
            DEBUG_PRINT("Skipping other file type: (%s)\n", file);
        }
    }

    if (sort_and_count(out_file) < 0) {
        goto cleanup;
    }
    status = EXIT_SUCCESS;
    DEBUG_PRINT("\nComplete!\n");

cleanup:
    if (word) {
        word_free(word);
    }

    if (out_fd >= 0) {
        if (close(out_fd) < 0) {
            perror("close out_file failed.");
            status = EXIT_FAILURE;
        }
        if (unlink(out_file) < 0) {
            perror("unlink out_file failed.");
            status = EXIT_FAILURE;
        }
    }
    return status;
}

int read_file(const char *path, int out_fd, Word *word) {
    DEBUG_PRINT("Reading file: %s\n", path);

    char buf[BUF_SIZE];
    ssize_t bytes;
    int status = -1;

    // Open read file
    int in_fd = open(path, O_RDONLY);
    if (in_fd < 0) {
        perror(path);
        return -1;
    }

    // Read file
    while ((bytes = read(in_fd, buf, BUF_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes; i++) {
            int res = -1;
            char c = buf[i];
            char next_c = '\0';

            if (i + 1 < bytes) {
                next_c = buf[i + 1];
            }

            if (isalpha(c)) {
                // 1. Regular alphabet
                res = word_append(word, c);
            } else if (c == '\'' && (word->len > 0 || isalpha(next_c))) {
                // 2. Apostrophe within/around word
                res = word_append(word, c);
            } else if (c == '-' && word->len > 0 && isalpha(next_c)) {
                // 3. Dash within word
                res = word_append(word, c);
            } else {
                // 4. Endpoints
                res = word_write(word, out_fd);
            }

            if (res < 0) {
                goto cleanup;
            }
        }
    }

    if (bytes < 0) {
        perror("read error");
        goto cleanup;
    }

    if (word_write(word, out_fd) < 0) {
        goto cleanup;
    }

    status = 0;

cleanup:
    if (close(in_fd) < 0) {
        perror("error closing in_fd");
        status = -1;
    }
    return status;
}

int read_dir(const char *parent_path, int out_fd, Word *word, const char *suffix) {
    DEBUG_PRINT("Reading directory: %s/\n", parent_path);

    DIR *dir_ptr = opendir(parent_path);
    if (!dir_ptr) {
        perror(parent_path);
        return -1;
    }

    struct dirent *dir_entry;
    int status = -1;
    char *full_path = NULL;

    while ((dir_entry = readdir(dir_ptr))) {
        // Get file type/name
        unsigned char file_type = dir_entry->d_type;
        const char *file_name = dir_entry->d_name;

        // Skip hidden files
        if (file_name[0] == '.') {
            continue;
        }

        // Check suffix match
        if (file_type == DT_REG) {
            size_t suf_len = strlen(suffix);
            size_t file_len = strlen(file_name);

            if (file_len < suf_len)
                continue;

            const char *cur_suf = file_name + file_len - suf_len;
            if (strcmp(cur_suf, suffix) != 0) {
                continue;
            }
        }

        // Build relative path
        size_t path_len = strlen(parent_path) + strlen(file_name) + 2;
        full_path = malloc(path_len);

        if (!full_path) {
            perror("malloc failure");
            goto cleanup;
        }

        snprintf(full_path, path_len, "%s/%s", parent_path, file_name);

        if (file_type == DT_REG) {
            if (read_file(full_path, out_fd, word) < 0) {
                goto cleanup;
            }
        } else if (file_type == DT_DIR) {
            if (read_dir(full_path, out_fd, word, suffix) < 0) {
                goto cleanup;
            }
        } else {
            DEBUG_PRINT("(read_dir): other file type detected - %s", full_path);
        }

        free(full_path);
        full_path = NULL;
    }

    status = 0;

cleanup:
    free(full_path);
    closedir(dir_ptr);
    return status;
}

int sort_and_count(char *out_file) {
    DEBUG_PRINT("Sorting and counting... \n\n");

    int p1[2], p2[2], p3[2];
    if (pipe(p1) < 0 || pipe(p2) < 0 || pipe(p3) < 0) {
        perror("error: pipe creation failed");
        return -1;
    }
    // 1. Sort the file to prepare for uniq -c
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("error: fork 1 failed");
        return -1;
    }
    if (pid1 == 0) {
        int in_fd = open(out_file, O_RDONLY);
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
        return -1;
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
        return -1;
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
        return -1;
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

    return 0;
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
        return NULL;
    }

    word->len = 0;
    word->cap = cap;
    word->bytes = malloc(cap);
    if (!word->bytes) {
        return NULL;
    }

    return word;
}

int word_append(Word *word, char c) {
    if (word->len + 1 >= word->cap) {
        word->cap = word->cap * 2;

        char *new_bytes = realloc(word->bytes, word->cap);
        if (!new_bytes) {
            perror("resizing word failed in word_append()");
            return -1;
        }
        word->bytes = new_bytes;
    }

    word->bytes[word->len++] = c;
    return 0;
}

int word_write(Word *word, int out_fd) {
    if (word->len == 0)
        return 0;

    // Append new line before write
    if (word_append(word, '\n') < 0) {
        return -1;
    }

    // Write to out_file
    if (write(out_fd, word->bytes, word->len) < 0) {
        word->len--;
        return -1;
    }
    word->len = 0; // Reset word length
    return 0;
}

void word_free(Word *word) {
    if (!word)
        return;

    free(word->bytes);
    free(word);
}