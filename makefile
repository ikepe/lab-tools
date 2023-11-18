CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lyaml

dos: dos.c
	$(CC) $(CFLAGS) -o dos dos.c $(LIBS)

clean:
	rm -f dos
