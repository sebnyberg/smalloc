CC := clang

CFLAGS  = -std=gnu99
CFLAGS += -g
CFLAGS += -O3
# CFLAGS += -fno-optimize-strlen
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
CFLAGS += -Werror
CFLAGS += -Wmissing-declarations
CFLAGS += -DUNITY_SUPPORT_64 -DUNITY_OUTPUT_COLOR

buddy:
	$(CC) -shared -fPIC $(CFLAGS) buddy.c -o malloc.so

clean:
	@rm -f *.o tests.out

.PHONY: test
test: clean tests.out
	@./tests.out

tests.out: buddy.c malloc_test.c
	@echo Compiling $@
	@$(CC) -o tests.out $(CFLAGS) -O0 buddy.c malloc_test.c unity/unity.c
