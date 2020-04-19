CC = gcc
CFLAGS = -Wall -g -lpthread
OBJS = master.o util.o

pi_mst: $(OBJS)
	$(CC) -o pi_mst  $(OBJS) $(CFLAGS)

master.o: master.c master.h
	$(CC) -c $(CFLAGS) -o master.o master.c
util.o: ./util/util.c ./util/util.h
	$(CC) -c $(CFLAGS) -o util.o ./util/util.c 

.PHONY: clean

clean: 
	rm -f *.o pi_mst
	echo Done cleaning!
install:
	test -d /opt/pi_master/ || mkdir /opt/pi_master
	test -d /opt/pi_master/log/ || mkdir /opt/pi_master/log
	cp ./master.conf /opt/pi_master/
	cp -f pi_mst /usr/bin/
uninstall:
	rm -r /opt/pi_master/
del:
	killall pi_mst

