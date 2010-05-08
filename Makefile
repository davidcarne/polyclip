
SRCS=src/polymath.c
OBJS=$(patsubst %.c,build/%.o, $(SRCS) )


TEST_SRCS=tests/main.c  tests/libtest.c \
			tests/internal_operations_test.c \
			tests/basic_polygon_tests.c tests/support.c \
			tests/phase_2_tests.c tests/phase_3_tests.c \
			tests/global_tests.c
			
TEST_OBJS=$(patsubst %.c,build/%_t.o, $(TEST_SRCS) )

test: build/test_runner
	./build/test_runner $(TESTNAME)

dtest: build/test_runner
	gdb  --command=gdbsetup --args ./build/test_runner $(TESTNAME)
	
CFLAGS=-Isrc/ -DINT_ASSERT -ggdb3

build/test_runner: $(TEST_OBJS) $(OBJS)
	@echo "LD	$@"
	@mkdir -p $(@D)
	@gcc $(LDFLAGS) -o $@ $^
	
build/%_t.o: %.c src/polymath.h src/polymath_internal.h
	@echo "CC	$@"
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -c -o $@ $<

build/%.o: %.c src/polymath.h src/polymath_internal.h
	@echo "CC	$@"
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -c -o $@ $<
	
clean:
	rm -rf build/*
