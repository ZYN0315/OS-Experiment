hdrs = ipc.h 

r_src = consumer.c ipc.c
r_obj = consumer.o ipc.o
w_src = barber.c ipc.c
w_obj = barber.o ipc.o
opts = -g -c
all: consumer barber
consumer: $(r_obj)
	gcc $(r_obj) -o consumer
consumer.o: $(r_src) $(hdrs)
	gcc $(opts) $(r_src)
barber: $(w_obj)
	gcc $(w_obj) -o barber
barber.o:$(w_src) $(hdrs)
	gcc $(opts) $(w_src)
clean:
	rm consumer barber *.o 