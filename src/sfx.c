#ifndef ADPCM_C
#define ADPCM_C 1

sfx_play(int ad_addr, int bytes, char freq)
{
    ad_stop();
    for(;;) {
        if (!ad_stat()) {
            break;
        }
    }
    ad_play(ad_addr, bytes, freq, 0);
}

#endif