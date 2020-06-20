all: spaceava.iso

HUC_ROOT = /mnt/c/Users/nicol/apps/huc

CC = PCE_INCLUDE=$(HUC_ROOT)/include/pce PCE_PCEAS=$(HUC_ROOT)/bin/pceas $(HUC_ROOT)/bin/huc
ISOLINK = $(HUC_ROOT)/bin/isolink

LOGO_SRC = logo/logo.c
CLASSIC_SRC = classic/classic.c

CFLAGS ?= -fno-recursive -msmall
CDFLAGS = -scd -over

spaceava.iso: logo.ovl classic.ovl adpcm_ovl classicdata cuefile
	$(ISOLINK) ../build/spaceava.iso ../build/logo.ovl ../build/adpcm.bin ../build/classic.ovl ../build/classicdata.bin

logo.ovl: $(LOGO_SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(LOGO_SRC) $(LIBS)
	mv */*.s */*.ovl */*.sym ../build

classic.ovl: $(CLASSIC_SRC) sprites tiles
	$(CC) $(CFLAGS) $(CDFLAGS) $(CLASSIC_SRC) $(LIBS)
	mv */*.s */*.ovl */*.sym ../build

cuefile: spaceava.cue
	cp spaceava.cue ../build/

adpcm_ovl: adpcm/whistle.vox
	cat adpcm/whistle.vox > ../build/adpcm.bin

classicdata: classicdata/classicdata.bin
	cp classicdata/classicdata.bin ../build/classicdata.bin

sprites: ../images/sprites/*.terra
	FILES=../images/sprites/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py sprite $$f ./images/$$(basename $$f .terra).bin ./images/$$(basename $$f .terra).pal.bin; \
	done;

tiles: ../images/tiles/*.terra
	FILES=../images/tiles/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py tile $$f ./images/$$(basename $$f .terra).bin ./images/$$(basename $$f .terra).pal.bin; \
	done;

pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f ../build/*
