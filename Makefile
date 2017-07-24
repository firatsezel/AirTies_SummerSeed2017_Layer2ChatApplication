all: Listener.c LinkedList.c
	gcc -o listener Listener.c LinkedList.c -lpthread
