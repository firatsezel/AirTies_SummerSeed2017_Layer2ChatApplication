all: Listener.c LinkedList.c Util.c
	gcc -o listener Listener.c LinkedList.c Util.c -lpthread
