ifndef ($(CC))
CC = gcc
endif

override CPPFLAGS += -D__KERNEL__ -I../include
override ASFLAGS += $(CPPFLAGS) -D__ASSEMBLY__ -traditional

ifeq ($(TESTING),)
ifeq ($(FOREIGN),"yes")
override CFLAGS	+= -Os -Wall -fno-builtin
else
override CFLAGS += -Os -Wall -mno-fp-regs -fno-builtin
endif
else
override CFLAGS += -Os -g3 -Wall
endif

ifeq ($(TESTING),)
libaboot.a: vsprintf.o memcpy.o memset.o string.o _setjmp.o \
	_longjmp.o isolib.o __divqu.o __remqu.o __divlu.o \
	__remlu.o
	ar cru $@ $?
else
libaboot.a: isolib.o
	ar cru $@ $?
endif

clean:
	rm -f libaboot.a *.o

__divqu.o: divide.S
	$(CC) -DDIV -c -o $@ divide.S

__remqu.o: divide.S
	$(CC) -DREM -c -o $@ divide.S

__divlu.o: divide.S
	$(CC) -DDIV -DINTSIZE -c -o $@ divide.S

__remlu.o: divide.S
	$(CC) -DREM -DINTSIZE -c -o $@ divide.S

