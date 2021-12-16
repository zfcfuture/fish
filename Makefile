
all: fish

clean:
	rm -f fish

fish: fish.c
	gcc -o fish fish.c -Wall