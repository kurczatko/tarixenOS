CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -I../include -I../include/printf -I../libc -I. -O2 -Wall -Wextra -std=c99
LDFLAGS = -m elf_i386 -nostdlib -Ttext=0x00100000 -e main

USER_LIBS = libc/string.c libc/stdio.c libc/stdlib.c

APPS = apps/kalkulator_user apps/notatnik_user apps/eksplorator_user apps/browser_user apps/gra_user apps/kursor_user

all: $(APPS)

apps/kalkulator_user: apps/kalkulator_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/kalkulator_user.c -o apps/kalkulator_user.o
	ld $(LDFLAGS) -o KALKULATOR.ELF apps/kalkulator_user.o okno_user.o string.o stdio.o stdlib.o

apps/notatnik_user: apps/notatnik_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/notatnik_user.c -o apps/notatnik_user.o
	ld $(LDFLAGS) -o NOTATNIK.ELF apps/notatnik_user.o okno_user.o string.o stdio.o stdlib.o

apps/eksplorator_user: apps/eksplorator_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/eksplorator_user.c -o apps/eksplorator_user.o
	ld $(LDFLAGS) -o EKSPLORATOR.ELF apps/eksplorator_user.o okno_user.o string.o stdio.o stdlib.o

apps/browser_user: apps/browser_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/browser_user.c -o apps/browser_user.o
	ld $(LDFLAGS) -o BROWSER.ELF apps/browser_user.o okno_user.o string.o stdio.o stdlib.o

apps/gra_user: apps/gra_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/gra_user.c -o apps/gra_user.o
	ld $(LDFLAGS) -o GRA.ELF apps/gra_user.o okno_user.o string.o stdio.o stdlib.o

apps/kursor_user: apps/kursor_user.c okno_user.c $(USER_LIBS)
	$(CC) $(CFLAGS) -c okno_user.c -o okno_user.o
	$(CC) $(CFLAGS) -c libc/string.c -o string.o
	$(CC) $(CFLAGS) -c libc/stdio.c -o stdio.o
	$(CC) $(CFLAGS) -c libc/stdlib.c -o stdlib.o
	$(CC) $(CFLAGS) -c apps/kursor_user.c -o apps/kursor_user.o
	ld $(LDFLAGS) -o KURSOR.ELF apps/kursor_user.o okno_user.o string.o stdio.o stdlib.o

clean:
	rm -f *.o apps/*.o apps/kalkulator_user apps/notatnik_user apps/eksplorator_user apps/browser_user apps/gra_user apps/kursor_user *.ELF
