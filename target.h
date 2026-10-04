#ifndef ANEMO_TARGET_H
#define ANEMO_TARGET_H

#include <stddef.h>
#include <stdio.h>

typedef struct {
    const char *name;
    const char *triple;
} AnemoTargetProfile;

const AnemoTargetProfile *anemo_target_profiles(size_t *count);
void anemo_target_print_profiles(FILE *out);

#endif
