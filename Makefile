all: prog.o
	gcc prog.o -o prog -g

prog.o: prog.c 
	gcc -c prog.c 

clean:
	rm -f *.o
	
