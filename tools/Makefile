CC := gcc
CFLAGS := -O2 -Wall -Wextra -std=c99
SRC := cc.c lexer.c codegen.c
OBJ := $(SRC:.c=.o)
TARGET := cc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
