char is_sgx()
{
    // FF:0018 is 0xFF on a SuperGrafx, and "something else" on a PC Engine
    // (mirror of VDC status register)

    return peek(0x18) == 0xFF;
}

sgx_load_vram(int vaddr, int *data, int nb)
{
    int i;

    poke(0x10, 0x5);
    poke(0x12, 0xC0); // 1100 0000
    poke(0x13, 0);

    poke(0x10, 0x00);
    pokew(0x12, vaddr);

    for (i = 0; i < (nb >> 1); i++)
    {
        poke(0x10, 0x02);
        pokew(0x12, data[i]);
    }

    return;
}

sgx_init()
{
    //  Init VPC
    poke(0x08, 0x33);
    poke(0x09, 0x33);

    // Set scroll of VDC #2 to 0,0
    poke(0x10, 0x07);
    poke(0x12, 0);
    poke(0x13, 0);

    poke(0x10, 0x08);
    poke(0x12, 0);
    poke(0x13, 0);

    // Set XRES
    // x = 256
    // 100000: HDW
    // hde = (38 - ( (18-(x/16)) + (x/8) )) = 4
    // 00000001 00100000
    // 01 32
    poke(0x10, 0x0B);
    poke(0x12, 32);
    poke(0x13, 1);
}

sgx_disable()
{
    //  Init VPC to 11; this kills it, oddly
    poke(0x08, 0x11);
    poke(0x09, 0x11);
}

scroll_sgx(char x, char y)
{
    poke(0x10, 0x07);
    poke(0x12, x);
    poke(0x13, 0);

    poke(0x10, 0x08);
    poke(0x12, y);
    poke(0x13, 0);
}
