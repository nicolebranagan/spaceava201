#include "classic/classic.h"

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
