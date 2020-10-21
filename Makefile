CFLAGS += -ggdb 
TARGET_ARCH=
CC = gcc
db : db.o _val.o
	gcc $^ -o $@

clean c : 
	/bin/rm *.o db -f
install : db
	sudo install db /usr/bin/
