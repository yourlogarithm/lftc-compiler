CC = gcc
CFLAGS = -Iinclude -Wall -g
DEPS = include/utils.h include/djb2.h include/lexer.h include/domain.h include/parser.h include/type.h
OBJ = obj/main.o obj/utils.o obj/djb2.o obj/lexer.o obj/domain.o obj/parser.o obj/type.o
TARGET = bin/compiler

$(shell mkdir -p bin obj)

all: $(TARGET)

$(OBJ): obj/%.o : src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean all

clean:
	rm -f obj/*.o $(TARGET)
