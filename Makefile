SOURCE=micro-bash.c parser.c

CFLAGS=-ggdb -lreadline

CC=gcc

PROGRAM_NAME=micro-Bash

$(PROGRAM_NAME):$(SOURCE)
	$(CC) $(SOURCE) $(CFLAGS) -o $(PROGRAM_NAME)

clean:
	rm  *.txt
