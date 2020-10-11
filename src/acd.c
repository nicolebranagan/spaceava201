#ifndef ARCADE_CARD_C
#define ARCADE_CARD_C 1

char dummy0;

char is_acd() {
    // Technically, we could detect an Arcade Card Duo in a Turbo Duo
    // might implement that for fun later
    return peek(0x1AFF) == 0x51;
}

#endif
