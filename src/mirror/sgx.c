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
    poke(0x12, 0x00);
    poke(0x13, 0x00);

    // Set XRES
    poke(0x10, 0x0B);
    poke(0x12, 0x1F);
    poke(0x13, 1);

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

    // Set control register
    poke(0x10, 0x5);
    poke(0x12, 0xC0); // 1100 0000
    poke(0x13, 0);
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
