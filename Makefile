CC = gcc
COMMON_CFLAGS = -std=c17 -Wall -Wextra -Werror
CFLAGS ?= $(COMMON_CFLAGS) -O2
LDFLAGS ?=

STRICT_CFLAGS = $(COMMON_CFLAGS) -Wpedantic -Wshadow -Wformat=2
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1

OBJS = main.o lexer.o parser.o ast.o semantic.o ir.o codegen.o utils.o update.o target.o

all: anemo

anemo: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o anemo $(OBJS)

check: anemo
	./tests/smoke.sh ./anemo

strict-warnings:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(STRICT_CFLAGS) -O2" all

sanitizers:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(COMMON_CFLAGS) $(SANITIZER_FLAGS)" LDFLAGS="$(SANITIZER_FLAGS)" check

main.o: main.c lexer.h parser.h ast.h semantic.h ir.h codegen.h target.h utils.h
lexer.o: lexer.c lexer.h utils.h
parser.o: parser.c parser.h ast.h lexer.h utils.h
ast.o: ast.c ast.h utils.h
semantic.o: semantic.c semantic.h ast.h utils.h
ir.o: ir.c ir.h ast.h utils.h
codegen.o: codegen.c codegen.h ir.h utils.h
utils.o: utils.c utils.h
update.o: update.c update.h utils.h
target.o: target.c target.h

clean:
	rm -f $(OBJS) anemo

.PHONY: all check strict-warnings sanitizers clean
