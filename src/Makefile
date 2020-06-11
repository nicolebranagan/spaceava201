all: spaceava.iso

HUC_ROOT = /mnt/c/Users/nicol/apps/huc

CC = PCE_INCLUDE=$(HUC_ROOT)/include/pce PCE_PCEAS=$(HUC_ROOT)/bin/pceas $(HUC_ROOT)/bin/huc
ISOLINK = $(HUC_ROOT)/bin/isolink

SRC = logo.c

CFLAGS ?= -fno-recursive -msmall
CDFLAGS = -scd -over

spaceava.iso: %.ovl adpcm_ovl cuefile
	$(ISOLINK) ../build/spaceava.iso ../build/logo.ovl ../build/adpcm.bin

%.ovl: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(SRC) $(LIBS)
	mv *.s *.ovl *.sym ../build

cuefile: spaceava.cue
	cp spaceava.cue ../build/

adpcm_ovl: whistle.vox
	cat whistle.vox > ../build/adpcm.bin

pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f ../build/*
