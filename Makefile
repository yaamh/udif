
objs=$(patsubst %.c,%.o,$(wildcard *.c))

%.o:%.c
	$(CC) -g -c $^ -o $@

all:$(objs)
	$(CC) -g $^ -lpthread -lform -lmenu -lpanel -lncursesw -o main
clear:
	rm *.o main
