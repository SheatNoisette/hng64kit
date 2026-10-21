#ifndef HNG_SOUND_H
#define HNG_SOUND_H

// Copies the hng64kit-mksound image from PR to sound RAM and releases the V53A
// Needs hng_core_init first and a cart built with --sound, call it once
void hng_sound_init(void);

#endif
