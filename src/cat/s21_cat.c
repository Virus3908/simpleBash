#include "s21_cat.h"

#define _POSIX_C_SOURCE 200809L

int main(int argc, char *argv[]) {
  int error = 0;
  char res_flags[COUNT_FLAGS] = "\0";
  // длинные опции
  struct option long_options[] = {{"number-nonblank", 0, 0, 'b'},
                                  {"number", 0, 0, 'n'},
                                  {"squeeze-blank", 0, 0, 's'},
                                  {0, 0, 0, 0}};
  // получаем флаги
  get_mode(&argv, &argc, res_flags, &error, STD_FLAGS, long_options);

  transform_mode(res_flags);

  // если аргументы не флаги остались, то читаем файлы, иначе stdin
  if (!error) {
    if (argc) {
      for (int i = 0; i < argc; i++) {
        char *file_name = argv[i];
        s21_cat(file_name, res_flags);
      }
    } else {
      s21_cat("-", res_flags);
    }
  }

  return 0;
}

void s21_cat(char *file_name, char *flags) {
  static int row = 1;
  int prev;
  if (row == 1) {
    prev = '\n';
  }
  // читать из файла или из ввода
  FILE *file;
  if (!strcmp(file_name, "-")) {
    file = stdin;
  } else {
    file = fopen(file_name, "r");
  }
  // проверка на наличие файла (ввода)
  if (file) {
    int ch;
    while ((ch = getc(file)) != EOF) {
      print_char(ch, prev, &row, flags);
      prev = ch;
    }
    if (file != stdin) {
      fclose(file);
    }
  } else {
    printf("s21_cat: %s: No such file or directory\n", file_name);
  }
}
// вывод символа на экран (или не вывод)
void print_char(int ch, int prev, int *row, char *flags) {
  static int empty_row_printed = 0;
  // проверка, что ранее была выведена пустая строка и наличие флага s
  if (!(strchr(flags, 's') && prev == '\n' && ch == '\n' &&
        empty_row_printed)) {
    // если ранее был перенос и сейчас перенос - мы печатаем пустую строку
    if (prev == '\n' && ch == '\n') {
      empty_row_printed = 1;
    } else {
      empty_row_printed = 0;
    }
    // выводим нумерацию b или n
    if (strchr(flags, 'b') && prev == '\n' && ch != '\n') {
      printf("%6d\t", *row);
      *row += 1;
    } else if (strchr(flags, 'n') && !strchr(flags, 'b') && prev == '\n') {
      printf("%6d\t", *row);
      *row += 1;
    }
    // выводим t и n
    if (strchr(flags, 'E') && ch == '\n') printf("$");
    if (strchr(flags, 'T') && ch == '\t') {
      printf("^");
      ch += 64;
    }
    // выводим непечатные символы
    if (strchr(flags, 'v') && ch != '\n' && ch != '\t') {
      if (ch > 126 && ch < 255) {
        ch -= 128;
        if (ch >= 0) {
          printf("M-");
        }
      }
      if ((ch < 32) || ch == 127) {
        printf("^");
      }
      if ((ch < 32)) {
        ch += 64;
      }
    }
    // печатаем наш символ
    printf("%c", ch);
  }
}
// приводим флаги e и t к комбинации простых флагов
void transform_mode(char *flags) {
  if (strchr(flags, 'e')) {
    append_flag(flags, 'E');
    append_flag(flags, 'v');
  }
  if (strchr(flags, 't')) {
    append_flag(flags, 'T');
    append_flag(flags, 'v');
  }
}

void get_mode(char ***argv, int *argc, char *flags, int *error, char *str_flags,
              struct option long_options[]) {
  int opt;
  int option_index = 0;
  while ((opt = getopt_long(*argc, *argv, str_flags, long_options,
                            &option_index)) != -1 &&
         !*error) {
    if (opt == '?') {
      *error = 1;
    } else {
      append_flag(flags, opt);
    }
  }
  (*argv) += optind;
  *argc -= optind;
}
