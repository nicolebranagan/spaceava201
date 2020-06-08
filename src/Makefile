all: sgx_test.iso

HUC_ROOT = /mnt/c/Users/nicol/apps/huc

CC = PCE_INCLUDE=$(HUC_ROOT)/include/pce PCE_PCEAS=$(HUC_ROOT)/bin/pceas $(HUC_ROOT)/bin/huc
ISOLINK = $(HUC_ROOT)/bin/isolink

SRC = sgx_test.c

CFLAGS ?= -fno-recursive -msmall
CDFLAGS = -scd -over

sgx_test.iso: %.ovl cuefile
	$(ISOLINK) ../build/sgx_test.iso ../build/*.ovl

%.ovl: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(SRC) $(LIBS)
	mv *.s *.ovl *.sym ../build

cuefile: sgx_test.cue
	cp sgx_test.cue ../build/

clean:
	rm -f ../build/*
