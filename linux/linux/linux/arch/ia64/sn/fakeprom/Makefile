#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
#  Copyright (c) 2000-2001 Silicon Graphics, Inc.  All rights reserved.
#

TOPDIR=../../../..
HPATH           = $(TOPDIR)/include

LIB     = ../../lib/lib.a

OBJ=fpromasm.o main.o  fw-emu.o fpmem.o klgraph_init.o
obj-y=fprom

fprom: $(OBJ)
	$(LD) -static -Tfprom.lds -o fprom $(OBJ) $(LIB)

.S.o:
	$(CC)  -D__ASSEMBLY__ $(AFLAGS) $(AFLAGS_KERNEL) -c -o $*.o $<

fpmem.o: fpmem.c
	$(CC)  $(subst -pg,,$(CFLAGS)) $(subst -pg,,$(CFLAGS_KERNEL)) -c -o $*.o $<

fw-emu.o: fw-emu.c
	$(CC)  $(subst -pg,,$(CFLAGS)) $(subst -pg,,$(CFLAGS_KERNEL)) -c -o $*.o $<

klgraph_init.o: klgraph_init.c
	$(CC)  $(subst -pg,,$(CFLAGS)) $(subst -pg,,$(CFLAGS_KERNEL)) -c -o $*.o $<

main.o: main.c
	$(CC)  $(subst -pg,,$(CFLAGS)) $(subst -pg,,$(CFLAGS_KERNEL)) -c -o $*.o $<

clean:
	rm -f *.o fprom


include $(TOPDIR)/Rules.make
