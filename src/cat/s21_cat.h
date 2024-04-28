#ifndef S21_CAT_H
#define S21_CAT_H

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/common.h"

#define STD_FLAGS "beEnstTv"  // Набор стандартных флагов
#define COUNT_FLAGS 9

void s21_cat(char *file_name, char *flags);
void print_char(int ch, int prev, int *row, char *flags);
void transform_mode(char *flags);
void get_mode(char ***argv, int *argc, char *flags, int *error, char *str_flags,
              struct option long_options[]);
#endif