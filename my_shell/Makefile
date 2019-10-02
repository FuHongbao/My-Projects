#设置编译器
CC = gcc

#-wall 大部分警告 -g调试
CFLAGS = -g -Wall

MAINOBJS = my_shell.o my_cd.o my_ls.o

.PHONY: clean

main: $(MAINOBJS)
	$(CC) $(CFLAGS) -o main $(MAINOBJS)

my_cd.o: my_cd.c my_cd.h
	$(CC) $(CFLAGS) -c -o my_cd.o my_cd.c

my_ls.o: my_ls.c my_ls.h
	$(CC) $(CFLAGS) -c -o my_ls.o my_ls.c

my_shell.o: my_shell.c my_cd.h my_ls.h
	$(CC) $(CFLAGS) -c -o my_shell.o my_shell.c

clean:
	rm -f $(MAINOBJS) main



