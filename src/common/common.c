#include "common.h"

void append_flag(char *flags, char curr_ch) {
  if (strchr(flags, curr_ch) == NULL) {
    char str_to_add[2] = "\0";
    str_to_add[0] = curr_ch;
    strcat(flags, str_to_add);
  }
}
