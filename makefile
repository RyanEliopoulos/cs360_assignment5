all: warn.c
	gcc -o warn warn.c
clean:
	rm README.txt warn warn.c
run:
	./warn
