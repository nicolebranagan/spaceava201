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
char tia_instruction[8];
char load_from_acd(
    int sector_offset,
    int vaddr,
    int nb)
{
    char i;
    int bigSteps;
    int remainder;
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

    // tia $c000, $0002, $2000
    // rts
    tia_instruction[0] = 0xe3;
    tia_instruction[1] = 0x00;
    tia_instruction[2] = 0xC0;
    tia_instruction[3] = 0x02;
    tia_instruction[4] = 0x00;
    tia_instruction[5] = 0x00;
    tia_instruction[6] = 0x20;
    tia_instruction[7] = 0x60;

    bigSteps = nb / 0x2000;
    remainder = nb;

    for (i = 0; i < bigSteps; i++) {
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
            jsr _tia_instruction
            cli

            pla 
            tam6 
            pla 
        #endasm
        remainder -= 0x2000;
        true_vaddr += 0x1000;
    }

    tia_instruction[5] = remainder & 0x00FF;
    tia_instruction[6] = (remainder & 0xFF00) >> 8;

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
        jsr _tia_instruction
        cli

        pla 
        tam6 
        pla 
    #endasm



}

#define CD_LOADVRAM(ovl_idx, sector_offset, vramaddr, bytes) (arcade_card_initialized == ACD_INITIALIZED ? load_from_acd(sector_offset, vramaddr, bytes) : cd_loadvram(ovl_idx, sector_offset, vramaddr, bytes))

#endif
