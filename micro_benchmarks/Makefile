SOLO5 := ../solo5
LKL := ../lkl

TOPDIR := $(SOLO5)
LKLINC := -I$(LKL)/tools/lkl/lib

SPT := syscall_test.spt

all: $(SPT)

include $(SOLO5)/Makefile.common

LDS.spt := $(SOLO5)/bindings/lkl/solo5_lkl.lds
BINDINGS.lkl := $(SOLO5)/bindings/lkl/solo5_lkl.o

ELFTOOL := $(SOLO5)/elftool/solo5-elftool

manifest.c: manifest.json $(SOLO5)/include/solo5/mft_abi.h $(ELFTOOL)
	@echo "ELFTOOL $<"
	$(ELFTOOL) gen-manifest $< $@

%.o: %.c $(SOLO5)/include/solo5/solo5.h
	@echo "CC $<"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

syscall_test.o: CFLAGS += $(LKLINC)

%.spt: %.o manifest.o $(LDS.spt) $(BINDINGS.lkl)
	@echo "LD $@"
	$(LD) $(LDFLAGS) -T $(LDS.spt) $(BINDINGS.lkl) $< ../musl/lib/libc.a manifest.o -o $@

.PHONY: clean

clean:
	rm -f *.o $(SPT)
