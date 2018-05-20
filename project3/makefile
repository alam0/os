
all: mandel mandelmovie oldmandel

mandelmovie: mandelmovie.o bitmap.o
	/usr/bin/gcc mandelmovie.o bitmap.o -o mandelmovie -lpthread

mandelmovie.o: mandelmovie.c
	/usr/bin/gcc -Wall -g -c mandelmovie.c -o mandelmovie.o

mandel: mandel.o bitmap.o
	/usr/bin/gcc mandel.o bitmap.o -o mandel -lpthread

mandel.o: mandel.c
	/usr/bin/gcc -Wall -g -c mandel.c -o mandel.o
	
oldmandel: oldmandel.o bitmap.o
	/usr/bin/gcc oldmandel.o bitmap.o -o oldmandel -lpthread
	
oldmandel.o: oldmandel.c
	/usr/bin/gcc -Wall -g -c oldmandel.c -o oldmandel.o

bitmap.o: bitmap.c
	/usr/bin/gcc -Wall -g -c bitmap.c -o bitmap.o

clean:
	rm -f mandel.o bitmap.o mandel oldmandel.o oldmandel mandelmovie mandelmovie.o
