CC := gcc

CFLAGS  = -std=gnu99
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
CFLAGS += -Werror
CFLAGS += -Wmissing-declarations
CFLAGS += -DUNITY_SUPPORT_64 -DUNITY_OUTPUT_COLOR
# CFLAGS += -D_FILE_OFFSET_BITS=64

# ASANFLAGS  = -fsanitize=address
# ASANFLAGS += -fno-common
# ASANFLAGS += -fno-omit-frame-pointer
clean:
	@rm -f *.o tests.out

.PHONY: test
test: clean tests.out
	@./tests.out

tests.out: buddy.c malloc_test.c
	@echo Compiling $@
	@$(CC) -o tests.out $(ASANFLAGS) $(CFLAGS) buddy.c malloc_test.c unity/unity.c
