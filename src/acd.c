#ifndef ARCADE_CARD_C
#define ARCADE_CARD_C 1

char dummy0;

char is_acd()
{
    // Technically, we could detect an Arcade Card Duo in a Turbo Duo
    // might implement that for fun later
    return peek(0x1AFF) == 0x51;
}

// 2048 in binary is 1000 0000 0000
// Highest sector offset is 1111 1111
// Bottom 8 bits will always be 0
int true_vaddr;
char load_from_acd(
    int sector_offset,
    int vaddr,
    int nb)
{
    int i;
    char val;
    true_vaddr = vaddr;

    poke(0x1A02, 00);
    pokew(0x1A03, sector_offset << 3);

    poke(0x1A05, 0);
    poke(0x1A06, 0);

    // Auto-increment
    poke(0x1A07, 1);
    poke(0x1A08, 0);

    // Control register
    // 0001 0001
    poke(0x1A09, 0x11);

    #asm
        pha
        tma6
        pha 
        lda #$40
        tam6

        sei
        vreg #0
        lda _true_vaddr
        sta $0002
        lda _true_vaddr+1
        sta $0003
        vreg #2
        tia $C000, $0002, $2000
        cli

        pla 
        tam6 
        pla 
    #endasm
}

#define CD_LOADVRAM(ovl_idx, sector_offset, vramaddr, bytes) (arcade_card_initialized == ACD_INITIALIZED ? load_from_acd(sector_offset, vramaddr, bytes) : cd_loadvram(ovl_idx, sector_offset, vramaddr, bytes))

#endif
