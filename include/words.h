#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void read_file(char *path, int out_fd);
void read_dir(char *parent_path, int out_fd, char *suffix);
int is_word_char(char c);
