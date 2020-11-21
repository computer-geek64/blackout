# Makefile
# Usage:
# make          # Compile binaries
# make clean    # Remove all binaries and objects

CC=cc


all: blackout

blackout: blackout.o
	$(CC) $? -o $@ -lgit2

blackout.o: src/blackout.c src/blackout.h
	$(CC) -c $< -o $@

clean:
	@rm -rvf blackout blackout.o

install:
	install blackout /usr/bin/blackout

uninstall: /usr/bin/blackout
	@rm -rvf $?
