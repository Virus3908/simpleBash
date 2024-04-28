CC=gcc
CFLAGS=-c -Wall -Werror -Wextra
CLANG_FORMAT=../../materials/linters/.clang-format
COMMON=../common/


all: s21_cat clean_o

s21_cat: s21_cat_o common_o
	$(CC) s21_cat.o common.o -o s21_cat

s21_cat_o:
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat.o


common_o:
	$(CC) $(CFLAGS) $(COMMON)common.c -o common.o

clean_o:
	rm -rf *.o

clean: clean_o
	rm -rf s21_cat

rebuild: clean all

clang:
	clang-format --style=file:$(CLANG_FORMAT) -i *.c *.h $(common)*.c $(common)*.h
