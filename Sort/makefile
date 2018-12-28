target: demo.c jonewan.o sort.o
	gcc demo.c jonewan.o sort.o -o demo
jonewan.o : jonewan.h jonewan.c
	gcc -c jonewan.c
sort.o : sort.h sort.c
	gcc -c sort.c

clean :
	rm -f ./*.o demo
