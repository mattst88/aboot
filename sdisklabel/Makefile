CC = gcc -O2
CFLAGS= -I../include $(CPPFLAGS) -Wall

all:	sdisklabel swriteboot

sdisklabel: sdisklabel.o library.o
	$(CC) $(LDFLAGS) sdisklabel.o library.o -o sdisklabel

swriteboot: swriteboot.o library.o
	$(CC) $(LDFLAGS) swriteboot.o library.o -o swriteboot

clean:
	rm -f sdisklabel swriteboot *.o
