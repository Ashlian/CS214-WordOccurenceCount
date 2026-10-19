#define _DEFAULT_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    char *bytes;
    size_t len;
    size_t cap;
} Word;

Word *word_init(size_t cap);
int word_append(Word *word, char c);
int word_write(Word *word, int out_fd);
void word_free(Word *word);

int read_file(const char *path, int out_fd, Word *word);
int read_dir(const char *parent_path, int out_fd, Word *word, const char *suffix);
int sort_and_count(char *out_file);

void close_pipes(int p1[2], int p2[2], int p3[2]);
