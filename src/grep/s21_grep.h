#ifndef S21_GREP_H
#define S21_GREP_H

#define _POSIX_C_SOURCE 200809L

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/common.h"

#define STD_FLAGS "e:ivclnhsof:"
#define COUNT_FLAGS 11
#define NMATCH 2

typedef struct {
  regex_t regex;
  int count;
  int option;
  regmatch_t pmatch[NMATCH];
  int ovector[NMATCH];
  int offset;
} regular_s;

void file_out(char *file_name, char *flags, char *pattern_str, int argc);
int compare_file_str(char *pattern_str, FILE *file, char *flags,
                     char *file_name);
void print_result(char *str, regular_s pattern_s, char *flags, int row,
                  char *file_name);
regular_s init_regular(char *flags, char *pattern_str);
char *get_pattern(char ***argv, int *argc);

void print_str_from_to(char *str, int start, int end);

char *get_pattern_from_file(char *file_name, char *pattern);
char *get_pattern_from_arg(const char *opt_arg, char *pattern);
char *get_mode(char ***argv, int *argc, char *flags, int *error,
               char *str_flags);
void new_line(char *str, char *flags);
void print_after_compare(int match_count, char *file_name, char *flags,
                         int argc);
#endif
