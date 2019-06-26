srcs = filexm.c
objs = filexm.o
all: filexm Database
filexm: $(objs)
	gcc $(objs) -o filexm
filexm.o: $(srcs)
	gcc -c $(srcs)
Database: Database.c
	gcc Database.c -o Database
clean:
	rm *.o 