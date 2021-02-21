
objs=$(patsubst %.c,%.o,$(wildcard src/*.c))
PWD=$(shell pwd)

%.o:%.c
	$(CC) -I$(PWD)/include -g -c $^ -o $@

all:$(objs)
	$(CC) -g $^ -lpthread -lform -lmenu -lpanel -lncursesw -o udiff 
clean:
	rm src/*.o udiff
