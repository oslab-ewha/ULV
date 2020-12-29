SOLO5 := ../solo5
LKL := ../lkl

TOPDIR := $(SOLO5)
LKLINC := -I$(LKL)/tools/lkl/lib

SPT := gdbm_test.spt wget_test.spt

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

%.o: CFLAGS += $(LKLINC)

gdbm_test.spt: gdbm_test.o manifest.o $(LDS.spt) $(BINDINGS.lkl)
	@echo "LD $@"
	$(LD) $(LDFLAGS) -T $(LDS.spt) $(BINDINGS.lkl) $< manifest.o /usr/lib/x86_64-linux-gnu/libgdbm.a ../musl/lib/libc.a -o $@

wget_test.spt: wget_test.o manifest.o $(LDS.spt) $(BINDINGS.lkl)
	@echo "LD $@"
	$(LD) $(LDFLAGS) -T $(LDS.spt) $(BINDINGS.lkl) $< manifest.o wget/src/libwget.a wget/lib/libgnu.a ../musl/lib/libc.a -o $@

.PHONY: clean

clean:
	rm -f *.o $(SPT)pac
