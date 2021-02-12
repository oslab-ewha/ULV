# SPDX-License-Identifier: GPL-2.0
out := arch/$(SRCARCH)/include/generated/asm
uapi := arch/$(SRCARCH)/include/generated/uapi/asm

# Create output directory if not already present
_dummy := $(shell [ -d '$(out)' ] || mkdir -p '$(out)') \
	  $(shell [ -d '$(uapi)' ] || mkdir -p '$(uapi)')

syscall64 := $(srctree)/$(src)/syscall_64.tbl

syshdr := $(srctree)/$(src)/syscallhdr.sh
systbl := $(srctree)/$(src)/syscalltbl.sh

quiet_cmd_syshdr = SYSHDR  $@
      cmd_syshdr = $(CONFIG_SHELL) '$(syshdr)' '$<' '$@' \
		   '$(syshdr_abi_$(basetarget))' \
		   '$(syshdr_pfx_$(basetarget))' \
		   '$(syshdr_offset_$(basetarget))'
quiet_cmd_systbl = SYSTBL  $@
      cmd_systbl = $(CONFIG_SHELL) '$(systbl)' $< $@

quiet_cmd_hypercalls = HYPERCALLS $@
      cmd_hypercalls = $(CONFIG_SHELL) '$<' $@ $(filter-out $<,$^)

syshdr_abi_unistd_32_ia32 := i386
syshdr_pfx_unistd_32_ia32 := ia32_
$(out)/unistd_32_ia32.h: $(syscall32) $(syshdr)
	$(call if_changed,syshdr)

syshdr_abi_unistd_64 := common,64
$(uapi)/unistd_64.h: $(syscall64) $(syshdr)
	$(call if_changed,syshdr)

$(out)/syscalls_64.h: $(syscall64) $(systbl)
	$(call if_changed,systbl)

uapisyshdr-y			+= unistd_64.h
syshdr-y			+= syscalls_64.h

targets	+= $(uapisyshdr-y) $(syshdr-y)

PHONY += all

all: $(addprefix $(uapi)/,$(uapisyshdr-y))
all: $(addprefix $(out)/,$(syshdr-y))
	@:
