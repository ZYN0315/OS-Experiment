head = vm.h
srcs = vm.cc
objs = vm.o
opts = -w -g -c

all: vm
vm:	$(objs)
	g++ $(objs) -o vm
vm.o: $(head) $(srcs)
	g++ $(opts) $(srcs)
clean:
	rm vm *.o