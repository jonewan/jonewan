target:	stringMatch.o jonewan.o 
	gcc stringMatch.o jonewan.o -o stringMatch
stringMatch.o	: stringMatch.c 
	gcc -c stringMatch.c 
jonewan.o:	jonewan.c jonewan.h
	gcc -c jonewan.c
clean:
	rm -f ./*.o stringMatch
