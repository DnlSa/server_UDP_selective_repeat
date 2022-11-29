CC = gcc
CFLAGS=-ggdb
DEPS = settings.h basic.h sender.h  reciever.h utility.h


FILEC = basic.h utility.c receiver.c sender.c client.c -lm -lpthread
FILES = basic.h utility.c receiver.c sender.c server.c -lm -lpthread
do:
	$(CC) $(FILEC)   -o client
	$(CC) $(FILES)   -o server
	@echo "Compilato"

