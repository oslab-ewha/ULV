BENCHS := syscall network
BENCHS_MB := $(BENCHS:%=mb_%)
BENCHS_BASE := $(BENCHS:%=%_test)
BENCHS_SRC := $(BENCHS_BASE:%=%.c)
BENCHS_OBJ := $(BENCHS_SRC:.c=.o)

LIBC_A = ../musl/lib/libc.a

CFLAGS = -g
LDFLAGS = -static -nolibc

all: $(BENCHS_MB)

%.o: %.c
	@echo "CC $<"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(TESTS_OBJ): CFLAGS += $(LKLINC)

mb_%: %_test.o libmb.a
	@echo "LD $@"
	musl-gcc $(LDFLAGS) $^ -o $@ $(LIBC_A)

libmb.a: libmb_stdio.o libmb_time.o libmb_net.o
	ar rcs $@ $^

.PHONY: clean

clean:
	rm -f *.o $(BENCHS)
