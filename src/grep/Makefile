CC=gcc
CFLAGS=-c -Wall -Werror -Wextra
CLANG_FORMAT=../../materials/linters/.clang-format
COMMON=../common/


all: s21_grep clean_o

build_test: s21_grep_test clean_o

s21_grep: s21_grep_o common_o
	$(CC) s21_grep.o common.o -o s21_grep 

s21_grep_test: s21_grep_o_test common_o
	$(CC) s21_grep_test.o common.o -o s21_grep 

s21_grep_o_test:
	$(CC) $(CFLAGS) -DTEST s21_grep.c -o s21_grep_test.o 

s21_grep_o:
	$(CC) $(CFLAGS) s21_grep.c -o s21_grep.o 

common_o:
	$(CC) $(CFLAGS) $(COMMON)common.c -o common.o

clean_o:
	rm -rf *.o

clean: clean_o
	rm -rf $(CAT)s21_cat

rebuild: clean all

clang:
	clang-format --style=file:$(CLANG_FORMAT) -i *.c *.h $(common)*.c $(common)*.h
