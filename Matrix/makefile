demo	:	demo.c jonewan.o triad.o
	gcc demo.c jonewan.o triad.o -o	demo
jonewan.o	:	jonewan.h	jonewan.c
	gcc -c jonewan.c -o jonewan.o
triad.o	:	triad.h	triad.c
	gcc -c triad.c -o triad.o

clean :
	rm -f *.o demo
