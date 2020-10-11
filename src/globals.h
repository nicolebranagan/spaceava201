char current_level;
char governor_step;
char has_backup_ram;
char story_mode;

int steps, deaths;

// I think all memory is initialized to 00,
// but just in case, I'm using A and 5 because they
// have alternating characters, which is in my experience
// rare in uninitialized RAM
#define ACD_INITIALIZED 0xA5
#define ACD_CANT_INITIALIZED 0x5A

char arcade_card_initialized;
