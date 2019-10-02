CC := gcc
CFLAGS := -Wall -g -lpthread
OBJS := client.o util.o

pi_clt: $(OBJS)
	$(CC) -o pi_clt $(OBJS) $(CFLAGS) 

client.o: client2.c client2.h
	$(CC) -c $(CFLAGS) -o client.o client2.c
util.o: util.c util.h
	$(CC) -c $(CFLAGS) -o util.o util.c

.PHONY: clean

clean:
	rm -f *.o pi_clt
	echo Done cleaning!

install:
	mkdir /opt/pi_client
	mkdir /opt/pi_client/script
	mkdir /opt/pi_client/log
	cp ./client.conf /opt/pi_client
	cp ./*.sh /opt/pi_client/script
	cp -f pi_clt /usr/bin/
uninstall:
	rm -r /opt/pi_client
del:
	killall pi_clt
