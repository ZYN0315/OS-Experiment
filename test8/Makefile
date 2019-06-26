head = disk.h
srcs = disk.cc
objs = disk.o
opts = -w -g -c

all: disk
disk: $(objs)
	g++ $(objs) -o disk
disk.o: $(head) $(srcs)
	g++ $(opts) $(srcs)
clean:
	rm disk *.o