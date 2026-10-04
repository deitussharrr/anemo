#include "target.h"

static const AnemoTargetProfile TARGET_PROFILES[] = {
    {"android-arm64", "aarch64-linux-android"},
    {"linux-arm64", "aarch64-unknown-linux-gnu"},
    {"linux-x86_64", "x86_64-unknown-linux-gnu"},
    {"windows-x86_64", "x86_64-pc-windows-msvc"},
    {"macos-arm64", "aarch64-apple-darwin"}
};

const AnemoTargetProfile *anemo_target_profiles(size_t *count) {
    if (count != NULL) {
        *count = sizeof(TARGET_PROFILES) / sizeof(TARGET_PROFILES[0]);
    }
    return TARGET_PROFILES;
}

void anemo_target_print_profiles(FILE *out) {
    size_t count;
    const AnemoTargetProfile *profiles = anemo_target_profiles(&count);

    fprintf(out, "Supported target profiles (planned; backends not yet available):\n");
    for (size_t i = 0; i < count; i++) {
        fprintf(out, "  %-16s %s [profile/planned]\n",
                profiles[i].name, profiles[i].triple);
    }
}
