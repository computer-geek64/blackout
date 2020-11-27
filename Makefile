# Makefile
# Usage:
# make          # Compile binaries
# make clean    # Remove all binaries and objects

CC=cc


all: blackout

blackout: blackout.o
	$(CC) $? -o $@ -lgit2

blackout.o: src/main.c src/main.h src/censor_string.c src/censor_string.h src/clone_commits.c src/clone_commits.h
	$(CC) -c $< -o $@

clean:
	@rm -rvf blackout blackout.o

install:
	install blackout /usr/bin/blackout

uninstall: /usr/bin/blackout
	@rm -rvf $?
