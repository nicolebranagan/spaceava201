#ifndef TIA_INSTRUCTION
#define TIA_INSTRUCTION 1
char tia_instruction[8];
#endif

char is_sgx()
{
    // FF:0018 is 0xFF on a SuperGrafx, and "something else" on a PC Engine
    // (mirror of VDC status register)

    return peek(0x18) == 0xFF;
}

sgx_load_vram(int vaddr, int *data, int nb)
{
    int i;

    // This function might not work if you try to transfer
    // more than 2048 bytes of data. Sorry

    // tia data, $0012, $0800
    // rts
    tia_instruction[0] = 0xe3;
    tia_instruction[1] = data & 0x00FF;
    tia_instruction[2] = (data & 0xFF00) >> 8;
    tia_instruction[3] = 0x12;
    tia_instruction[4] = 0x00;
    tia_instruction[5] = 0x00;
    tia_instruction[6] = 0x08;
    tia_instruction[7] = 0x60;

    if (nb != 2048) {
        tia_instruction[5] = nb & 0x00FF;
        tia_instruction[6] = (nb & 0xFF00) >> 8;
    }

    poke(0x10, 0x00);
    pokew(0x12, vaddr);

    #asm
        pha

        sei
        lda #$02
        sta $0010
        jsr _tia_instruction
        cli

        pla 
    #endasm
    return;
}

sgx_init(char size)
{
    //  Init VPC
    poke(0x08, 0x33);
    poke(0x09, 0x33);
    poke(0x0a, 0x00);
    poke(0x0b, 0x00);
    poke(0x0c, 0x00);
    poke(0x0d, 0x00);

    // Set scroll of VDC #2 to 0,0
    poke(0x10, 0x07);
    poke(0x12, 0);
    poke(0x13, 0);

    poke(0x10, 0x08);
    poke(0x12, 0);
    poke(0x13, 0);

    // Set MWR
    poke(0x10, 0x09);
    poke(0x12, size << 4);
    poke(0x13, 0x00);

    // Set XRES
    poke(0x10, 0x0A);
    poke(0x12, 0x02);
    poke(0x13, 0x02);

    poke(0x10, 0x0B);
    poke(0x12, 0x1F);
    poke(0x13, 0x04);

    // Set YRES
    poke(0x10, 0x0C);
    poke(0x12, 0x02);
    poke(0x13, 0x17);

    poke(0x10, 0x0D);
    poke(0x12, 0xDF);
    poke(0x13, 0x00);

    poke(0x10, 0x0E);
    poke(0x12, 0x0C);
    poke(0x13, 0x00);

    poke(0x10, 0x0F);
    poke(0x12, 0x10);
    poke(0x13, 0x00);

    // Set control register
    poke(0x10, 0x5);
    poke(0x12, 0x80); // 1000 0000
    poke(0x13, 0);

    // Set scanline interrupt
    poke(0x10, 0x6);
    pokew(0x12, 0);
}

sgx_disable()
{
    //  Init VPC to 11; this kills it, oddly
    poke(0x08, 0x11);
    poke(0x09, 0x11);
}

scroll_sgx(int x, int y)
{
    poke(0x10, 0x07);
    pokew(0x12, x);

    poke(0x10, 0x08);
    pokew(0x12, y);
}
