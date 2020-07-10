all: spaceava201

HUC_ROOT = /mnt/c/Users/nicol/apps/huc

CC = PCE_INCLUDE=$(HUC_ROOT)/include/pce PCE_PCEAS=$(HUC_ROOT)/bin/pceas $(HUC_ROOT)/bin/huc
ISOLINK = $(HUC_ROOT)/bin/isolink

LOGO_SRC = logo/logo.c
CLASSIC_SRC = classic/classic.c
GOVERNOR_SRC = governor/governor.c
STORY_SRC = story/story.c

CFLAGS ?= -fno-recursive -msmall
CDFLAGS = -scd -over

spaceava201: spaceava.iso
	rm ../build/iso/*
	cp ../waves/track1.wav "../build/iso/Space Ava 201 - 01 - Warning.wav"
	mv ../build/spaceava.iso "../build/iso/Space Ava 201 - Track 2.iso"
	cp spaceava.cue "../build/iso/Space Ava 201.cue"
	cp ../mods/ballad.wav "../build/iso/Space Ava 201 - 03 - Ballad of St. Janet.wav"
	cp ../mods/evanssense.wav "../build/iso/Space Ava 201 - 04 - Evan S. Sense.wav"
	cp ../mods/spaceful.wav "../build/iso/Space Ava 201 - 05 - Spaceful.wav"
	cp ../mods/chime2020.wav "../build/iso/Space Ava 201 - 06 - Chime 2020.wav"
	cp ../placeholder.wav ../build/iso

spaceava.iso: logo.ovl classic.ovl governor.ovl adpcm_ovl classicdata images story.ovl storydata
	mv */*.s */*.ovl */*.sym ../build
	$(ISOLINK) ../build/spaceava.iso ../build/logo.ovl ../build/adpcm.bin ../build/classic.ovl ../build/classicdata.bin ../build/images.bin ../build/governor.ovl ../build/story.ovl ../build/storydata.bin ../build/bats.bin

logo.ovl: $(LOGO_SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(LOGO_SRC) $(LIBS)

classic.ovl: $(CLASSIC_SRC) images
	$(CC) $(CFLAGS) $(CDFLAGS) $(CLASSIC_SRC) $(LIBS)

governor.ovl: $(GOVERNOR_SRC) images
	$(CC) $(CFLAGS) $(CDFLAGS) $(GOVERNOR_SRC) $(LIBS)

story.ovl: $(STORY_SRC) images
	$(CC) $(CFLAGS) $(CDFLAGS) $(STORY_SRC) $(LIBS)

adpcm_ovl: adpcm/whistle.vox
	python3 ../tools/overlay/overlay.py ../build/adpcm.bin ./adpcm/adpcm.h ./adpcm/*.vox

classicdata: classicdata/classicdata.bin
	cp classicdata/classicdata.bin ../build/classicdata.bin

storydata: story/script.json
	python3 ../tools/storyparser/story.py story/script.json ../build/storydata.bin

images: sprites tiles 8x8fonts 8x16fonts faces
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

faces:../images/faces/*.terra
	FILES=../images/faces/*.terra;\
	for f in $$FILES; \
	do \
		python3 ../tools/turboize/turboize.py face $$f ./images/$$(basename $$f .terra)_face.chr ./palettes/$$(basename $$f .terra)_face.pal; \
	done;

pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f ../build/*
