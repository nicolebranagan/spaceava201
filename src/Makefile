all: spaceava.iso

HUC_ROOT = /mnt/c/Users/nicol/apps/huc

CC = PCE_INCLUDE=$(HUC_ROOT)/include/pce PCE_PCEAS=$(HUC_ROOT)/bin/pceas $(HUC_ROOT)/bin/huc
ISOLINK = $(HUC_ROOT)/bin/isolink

LOGO_SRC = logo/logo.c
CLASSIC_SRC = classic/classic.c
GOVERNOR_SRC = governor/governor.c

CFLAGS ?= -fno-recursive -msmall
CDFLAGS = -scd -over

spaceava.iso: logo.ovl classic.ovl governor.ovl adpcm_ovl classicdata cuefile images
	mv */*.s */*.ovl */*.sym ../build
	$(ISOLINK) ../build/spaceava.iso ../build/logo.ovl ../build/adpcm.bin ../build/classic.ovl ../build/classicdata.bin ../build/images.bin ../build/governor.ovl

logo.ovl: $(LOGO_SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(LOGO_SRC) $(LIBS)

classic.ovl: $(CLASSIC_SRC) images
	$(CC) $(CFLAGS) $(CDFLAGS) $(CLASSIC_SRC) $(LIBS)

governor.ovl: $(GOVERNOR_SRC) images
	$(CC) $(CFLAGS) $(CDFLAGS) $(GOVERNOR_SRC) $(LIBS)

cuefile: spaceava.cue
	cp spaceava.cue ../build/

adpcm_ovl: adpcm/whistle.vox
	cat adpcm/whistle.vox > ../build/adpcm.bin

classicdata: classicdata/classicdata.bin
	cp classicdata/classicdata.bin ../build/classicdata.bin

images: sprites tiles 8x8fonts 8x16fonts
	python3 ../tools/overlay/overlay.py ../build/images.bin ./images/images.h ./images/*.chr

sprites: ../images/sprites/*.terra
	FILES=../images/sprites/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py sprite $$f ./images/$$(basename $$f .terra).chr ./palettes/$$(basename $$f .terra).pal; \
	done;

tiles: ../images/tiles/*.terra
	FILES=../images/tiles/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py tile $$f ./images/$$(basename $$f .terra).chr ./palettes/$$(basename $$f .terra).pal; \
	done;

8x8fonts: ../images/8x8fonts/*.terra
	FILES=../images/8x8fonts/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py 8x8font $$f ./images/$$(basename $$f .terra).chr ./palettes/$$(basename $$f .terra).pal; \
	done;

8x16fonts:../images/8x16fonts/*.terra
	FILES=../images/8x16fonts/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py 8x16font $$f ./images/$$(basename $$f .terra).chr ./palettes/$$(basename $$f .terra).pal; \
	done;

pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f ../build/*
