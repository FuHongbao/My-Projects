CC := g++
CFLAGS := -Wall -g
hzip: hfmzip.o
	$(CC) -o hzip hfmzip.o $(CFLAGS)

hfmzip.o: hfm_zip.c hfm_zip.h util.h
	$(CC) -c $(CFLAGS) -o hfmzip.o hfm_zip.c

.PHONY: clean
clean:
	rm -f *.o hzip
	echo clean Done!
install:
	cp -f hzip /usr/bin/
uninstall:
	rm -f /usr/bin/hzip

