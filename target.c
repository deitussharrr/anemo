#include "target.h"

#include <string.h>

static const AnemoTargetProfile TARGET_PROFILES[] = {
    {"android-arm64", "aarch64-linux-android", 0},
    {"linux-arm64", "aarch64-unknown-linux-gnu", 0},
    {"linux-x86_64", "x86_64-unknown-linux-gnu", 1},
    {"windows-x86_64", "x86_64-pc-windows-msvc", 0},
    {"macos-arm64", "aarch64-apple-darwin", 0}
};

const AnemoTargetProfile *anemo_target_profiles(size_t *count) {
    if (count != NULL) {
        *count = sizeof(TARGET_PROFILES) / sizeof(TARGET_PROFILES[0]);
    }
    return TARGET_PROFILES;
}

const AnemoTargetProfile *anemo_target_find(const char *name) {
    size_t count;
    const AnemoTargetProfile *profiles = anemo_target_profiles(&count);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(profiles[i].name, name) == 0) {
            return &profiles[i];
        }
    }
    return NULL;
}

void anemo_target_print_profiles(FILE *out) {
    size_t count;
    const AnemoTargetProfile *profiles = anemo_target_profiles(&count);

    fprintf(out, "Anemo target profiles:\n");
    for (size_t i = 0; i < count; i++) {
        const char *status = profiles[i].backend_available ? "prototype" : "planned";
        fprintf(out, "  %-16s %s [%s]\n",
                profiles[i].name, profiles[i].triple, status);
    }
}
