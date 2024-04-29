#include "s21_grep.h"

int main(int argc, char *argv[]) {
  int error = 0;
  char res_flags[COUNT_FLAGS] = "\0";
  struct option long_options[] = {{"regexp", 1, 0, 'e'},
                                  {"file", 1, 0, 'f'},
                                  {"ignore-case", 0, 0, 'i'},
                                  {"invert-match", 0, 0, 'v'},
                                  {"count", 0, 0, 'c'},
                                  {"files-with-matches", 0, 0, 'l'},
                                  {"only-matching", 0, 0, 'o'},
                                  {"no-messages", 0, 0, 's'},
                                  {"no-filename", 0, 0, 'h'},
                                  {"line-number", 0, 0, 'n'},
                                  {0, 0, 0, 0}};
  // получаем флаги, и если есть флаги e и f записываем строку для сравнения
  char *pattern_str =
      get_mode(&argv, &argc, res_flags, &error, STD_FLAGS, long_options);
  // если не было флагов e и f записываем аргумент

  // если не было ошибок при считывании и есть аргумент файлов и есть строка для
  // сравнения
  if (!error && argc > 0 && pattern_str) {
    // поочереди читаем все файлы
    for (int i = 0; i < argc; i++) {
      char *file_name = argv[i];
      file_out(file_name, res_flags, pattern_str, argc);
    }
  }
  free(pattern_str);
  return 0;
}
// получаем флаги и если есть e или f - строку для сравнения
char *get_mode(char ***argv, int *argc, char *flags, int *error,
               char *str_flags, struct option long_options[]) {
  int opt;
  char *opt_arg = NULL;
  int option_index = 0;
  while ((opt = getopt_long(*argc, *argv, str_flags, long_options,
                            &option_index)) != -1 &&
         !*error) {
    if (opt != '?' && opt != ':') {
      if (optarg && opt == 'f') {
        opt_arg = get_pattern_from_file(optarg, opt_arg);
      } else if (optarg && opt == 'e') {
        opt_arg = get_pattern_from_arg(optarg, opt_arg);
      }
      append_flag(flags, opt);
    }
  }

  (*argv) += optind;
  *argc -= optind;
  if (!strchr(flags, 'f') && !strchr(flags, 'e') && *argc > 0) {
    opt_arg = get_pattern(argv, argc);
  }
  return opt_arg;
}
// В случае флага f получаем строки из файла
char *get_pattern_from_file(char *file_name, char *pattern) {
  FILE *file;
  if ((file = fopen(file_name, "rt"))) {
    char *curr_str = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&curr_str, &len, file)) != -1) {
      pattern = get_pattern_from_arg(curr_str, pattern);
    }

    fclose(file);
    free(curr_str);
  } else {
    perror(file_name);
  }

  return pattern;
}
// в итоговую строку для сравнений записываются аргументы (строки файла)
char *get_pattern_from_arg(const char *opt_arg, char *pattern) {
  size_t size = strlen(opt_arg) + 5;
  if (pattern) {
    size += strlen(pattern);
  }
  char *new_pattern = realloc(pattern, size);
  if (new_pattern) {
    if (!pattern) {
      *new_pattern = '\0';
    }
    if (*new_pattern) {
      strcat(new_pattern, "|");
    }
    char *copy_opt_arg = strdup(opt_arg);
    if (!copy_opt_arg) {
      free(new_pattern);
      new_pattern = NULL;
    } else {
      strcat(new_pattern, strtok(copy_opt_arg, "\n"));
      free(copy_opt_arg);
    }
  }
  return new_pattern;
}
// если не было флагов e и f записывается аргумент командной строки в сравнение
char *get_pattern(char ***argv, int *argc) {
  char *pattern = strdup(*argv[0]);
  (*argv)++;
  *argc -= 1;
  return pattern;
}
// чтение файла
void file_out(char *file_name, char *flags, char *pattern_str, int argc) {
  FILE *file;
  char *file_name_to_print = NULL;
  // если файлов подано больше 1, то необходимо печатать название файла
  if (argc > 1) {
    file_name_to_print = file_name;
  }
  if ((file = fopen(file_name, "r"))) {
    // сравниваем строки и получаем кол-во совпадений в файле
    int match_count =
        compare_file_str(pattern_str, file, flags, file_name_to_print);
    // если совпадения есть и стоит флаг l или c печатаем вывод
    print_after_compare(match_count, file_name, flags, argc);
    // если нет файла и не стоит флаг s
  } else if (!strchr(flags, 's')) {
    perror(file_name);
  }
}

int compare_file_str(char *pattern_str, FILE *file, char *flags,
                     char *file_name) {
  int row = 1;          // счетчик строк
  int match_count = 0;  // счетчик совпадений
  // инициализируем регулярное выражение
  regular_s pattern_s = init_regular(flags, pattern_str);

  char *str = NULL;
  size_t len = 0;
  ssize_t read;
  // читаем файл построчно
  while ((read = getline(&str, &len, file)) != -1) {
    pattern_s.offset = 0;
    pattern_s.ovector[0] = 0;
    pattern_s.ovector[1] = 0;
    // сравниваем строк с паттерном
    while ((pattern_s.count = regexec(&pattern_s.regex, str + pattern_s.offset,
                                      NMATCH, pattern_s.pmatch, 0)) == 0) {
      if (pattern_s.offset == 0) {
        match_count += 1;
      }
      pattern_s.ovector[0] = pattern_s.offset + (int)pattern_s.pmatch->rm_so;
      pattern_s.ovector[1] = pattern_s.offset + (int)pattern_s.pmatch->rm_eo;
      // проверка на инвертированный вывод
      if (!strchr(flags, 'v')) {
        // выводим строку до начала совпадения и совпадение
        print_result(str, pattern_s, flags, row, file_name);
      }
      // устанавливаем смещение на конец совпадения и ищем дальшей
      pattern_s.offset = pattern_s.ovector[1];
    }
    // если смещения не было (нет совпадений) и есть флаг v или наоборот и нет
    // флага o печатаем строку до конца
    if (((pattern_s.offset != 0 && !strchr(flags, 'v')) ||
         (pattern_s.offset == 0 && strchr(flags, 'v'))) &&
        !strchr(flags, 'o')) {
      pattern_s.ovector[0] = (int)strlen(str);
      print_result(str, pattern_s, flags, row, file_name);
    }
    row++;
  }
  free(str);
  regfree(&pattern_s.regex);

  if (strchr(flags, 'v')) {
    match_count = row - match_count - 1;
  }
  return match_count;
}
// инициализация регулярного выражения
regular_s init_regular(char *flags, char *pattern_str) {
  regular_s pattern_s;
  pattern_s.count = 0;
  pattern_s.option = 0;
  // если есть флаг i, то меняем опции регулярного выражения
  if (strchr(flags, 'i')) {
    pattern_s.option = REG_ICASE;
  }
  // создаем регулярное выражение
  regcomp(&pattern_s.regex, pattern_str, REG_EXTENDED | pattern_s.option);

  return pattern_s;
}
// вывод результата
void print_result(char *str, regular_s pattern_s, char *flags, int row,
                  char *file_name) {
  // если нет флагов l и c
  if ((!strchr(flags, 'l')) && (!strchr(flags, 'c'))) {
    // если нам передали имя файла и нет флага h и начало строки или флаг o
    // выводим имя
    if ((file_name && (!strchr(flags, 'h')) &&
         (pattern_s.offset == 0 || strchr(flags, 'o')))) {
      printf("%s:", file_name);
    }
    // если есть флаг n и нет смещения (печатаем с начала строки) или есть флаг
    // o выводим строку
    if ((strchr(flags, 'n') && (pattern_s.offset == 0 || strchr(flags, 'o')))) {
      printf("%d:", row);
    }
    // если нет флага o выводим текст до совпадения
    if ((!strchr(flags, 'o'))) {
      print_str_from_to(str, pattern_s.offset, pattern_s.ovector[0]);
    }
    // выводим совпадение
    print_str_from_to(str, pattern_s.ovector[0], pattern_s.ovector[1]);
    // если мы допечатали строку а в конце неё не оказалось переноса - переносим
    // указатель
    if (pattern_s.ovector[0] == (int)strlen(str) || (strchr(flags, 'o'))) {
      new_line(str, flags);
    }
  }
}
// вывод новой строки
void new_line(char *str, char *flags) {
  size_t len = strlen(str);
  if (strchr(flags, 'o') || len == 0 || str[len - 1] != '\n') {
    printf("\n");
  }
}
// вывод строки от и до с нужным цветом
void print_str_from_to(char *str, int start, int end) {
  for (int i = start; i < end; i++) {
    printf("%c", str[i]);
  }
}

void print_after_compare(int match_count, char *file_name, char *flags,
                         int argc) {
  // если есть совпадения и стоит флаг l вывести имена
  if (match_count > 0 && strchr(flags, 'l')) {
    printf("%s", file_name);
    printf("\n");
  }
  // если стоит флаг c и нет l вывести кол-во совпадений
  if (strchr(flags, 'c') && !strchr(flags, 'l')) {
    // если нет флага h и файлов больше одного, сначала вывести имя файла
    if (argc > 1 && !strchr(flags, 'h')) {
      printf("%s:", file_name);
    }
    printf("%d", match_count);
    printf("\n");
  }
}