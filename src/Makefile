#Makefile for shell
#command to compile:make
shell: shell.c shell_core.c handlers.c logs.c pipe_command.c
	gcc -o shell shell.c shell_core.c handlers.c logs.c pipe_command.c shell.h -w
clean:
	rm -rf shell
