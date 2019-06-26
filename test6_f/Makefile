head = ip.h
srcs = ip.cc
objs = ip.o
opts = -w -g -c

all: ip
ip:	$(objs)
	g++ $(objs) -o ip
ip.o: $(head) $(srcs)
	g++ $(opts) $(srcs)
clean:
	rm ip *.o