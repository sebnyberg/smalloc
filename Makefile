CC := gcc

CFLAGS  = -std=gnu99
CFLAGS += -g
CFLAGS += -O3
CFLAGS += -fno-optimize-strlen
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
CFLAGS += -Werror
CFLAGS += -Wmissing-declarations
CFLAGS += -DUNITY_SUPPORT_64 -DUNITY_OUTPUT_COLOR

ll:
	$(CC) -shared -fPIC $(CFLAGS) ll.c -o ll.so

buddy:
	$(CC) -shared -fPIC $(CFLAGS) buddy.c -o buddy.so

clean:
	@rm -f *.o tests.out buddy.so ll.so

tests-ll.out: clean ll.c malloc_test.c
	@$(CC) -o tests-ll.out $(CFLAGS) ll.c malloc_test.c unity/unity.c

tests-buddy.out: clean buddy.c malloc_test.c
	@$(CC) -o tests-buddy.out $(CFLAGS) buddy.c malloc_test.c unity/unity.c

.PHONY: test
test: test-ll test-buddy

.PHONY: test-ll
test-ll: tests-ll.out
	@./tests-ll.out

.PHONY: test-buddy
test-buddy: tests-buddy.out
	@./tests-buddy.out
