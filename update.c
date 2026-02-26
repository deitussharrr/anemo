#include "update.h"

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define popen _popen
#define pclose _pclose
#endif

static const char *default_repo(void) {
    const char *repo = getenv("ANEMO_GITHUB_REPO");
    if (repo && *repo) {
        return repo;
    }
    return "tussh/anemo";
}

static void copy_trimmed(char *dst, size_t dst_cap, const char *src) {
    while (*src && isspace((unsigned char)*src)) {
        src++;
    }
    size_t n = 0;
    while (src[n] && src[n] != '\r' && src[n] != '\n' && n + 1 < dst_cap) {
        dst[n] = src[n];
        n++;
    }
    while (n > 0 && isspace((unsigned char)dst[n - 1])) {
        n--;
    }
    dst[n] = '\0';
}

static const char *normalized_version_ptr(const char *v) {
    while (*v && isspace((unsigned char)*v)) {
        v++;
    }
    if ((v[0] == 'v' || v[0] == 'V') && isdigit((unsigned char)v[1])) {
        v++;
    }
    if ((v[0] == 'b' || v[0] == 'B') &&
        (v[1] == 'e' || v[1] == 'E') &&
        (v[2] == 't' || v[2] == 'T') &&
        (v[3] == 'a' || v[3] == 'A')) {
        v += 4;
        while (*v && !isdigit((unsigned char)*v)) {
            v++;
        }
    }
    return v;
}

static int parse_next_num(const char **p, int *out_num) {
    while (**p && !isdigit((unsigned char)**p)) {
        if (**p == '.') {
            (*p)++;
            break;
        }
        (*p)++;
    }
    if (!isdigit((unsigned char)**p)) {
        *out_num = 0;
        return 0;
    }
    int n = 0;
    while (isdigit((unsigned char)**p)) {
        n = (n * 10) + (**p - '0');
        (*p)++;
    }
    *out_num = n;
    return 1;
}

static int compare_versions(const char *a, const char *b) {
    const char *pa = normalized_version_ptr(a);
    const char *pb = normalized_version_ptr(b);

    for (int i = 0; i < 4; i++) {
        int na = 0;
        int nb = 0;
        int ha = parse_next_num(&pa, &na);
        int hb = parse_next_num(&pb, &nb);
        if (!ha && !hb) {
            return 0;
        }
        if (na < nb) return -1;
        if (na > nb) return 1;
    }
    return 0;
}

static int read_cmd_output_line(const char *cmd, char *out, size_t out_cap) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return 0;
    }
    if (!fgets(out, (int)out_cap, fp)) {
        pclose(fp);
        return 0;
    }
    pclose(fp);
    copy_trimmed(out, out_cap, out);
    return out[0] != '\0';
}

static int fetch_latest_tag(char *out_tag, size_t out_cap) {
    const char *repo = default_repo();
    char cmd[4096];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd),
             "powershell -NoProfile -ExecutionPolicy Bypass -Command "
             "\"$ProgressPreference='SilentlyContinue';"
             "$u='https://api.github.com/repos/%s/releases/latest';"
             "try{$r=Invoke-RestMethod -UseBasicParsing -Uri $u -Headers @{\\\"User-Agent\\\"='anemo'};"
             "if($r.tag_name){[Console]::Out.Write($r.tag_name)}}catch{}\"",
             repo);
#else
    snprintf(cmd, sizeof(cmd),
             "sh -lc \"curl -fsSL -H 'User-Agent: anemo' "
             "https://api.github.com/repos/%s/releases/latest "
             "| sed -n 's/.*\\\"tag_name\\\"[[:space:]]*:[[:space:]]*\\\"\\([^\\\"]*\\)\\\".*/\\1/p' | head -n1\"",
             repo);
#endif
    return read_cmd_output_line(cmd, out_tag, out_cap);
}

static int ensure_dir(const char *path) {
#ifdef _WIN32
    int rc = _mkdir(path);
    return rc == 0 || rc == -1;
#else
    (void)path;
    return 1;
#endif
}

static int should_check_now(void) {
    const char *disable = getenv("ANEMO_DISABLE_UPDATE_CHECK");
    if (disable && strcmp(disable, "1") == 0) {
        return 0;
    }

#ifdef _WIN32
    const char *local = getenv("LOCALAPPDATA");
    if (!local || !*local) {
        return 1;
    }
    char dir[1024];
    char cache[1200];
    snprintf(dir, sizeof(dir), "%s\\Anemo", local);
    if (!ensure_dir(dir)) {
        return 1;
    }
    snprintf(cache, sizeof(cache), "%s\\update_check.txt", dir);
    FILE *f = fopen(cache, "rb");
    if (!f) {
        return 1;
    }
    long last = 0;
    if (fscanf(f, "%ld", &last) != 1) {
        fclose(f);
        return 1;
    }
    fclose(f);
    long now = (long)time(NULL);
    return (now - last) >= 86400;
#else
    return 1;
#endif
}

static void mark_checked_now(void) {
#ifdef _WIN32
    const char *local = getenv("LOCALAPPDATA");
    if (!local || !*local) {
        return;
    }
    char dir[1024];
    char cache[1200];
    snprintf(dir, sizeof(dir), "%s\\Anemo", local);
    if (!ensure_dir(dir)) {
        return;
    }
    snprintf(cache, sizeof(cache), "%s\\update_check.txt", dir);
    FILE *f = fopen(cache, "wb");
    if (!f) {
        return;
    }
    fprintf(f, "%ld\n", (long)time(NULL));
    fclose(f);
#endif
}

void anemo_auto_check_for_updates(const char *current_version) {
    if (!should_check_now()) {
        return;
    }
    mark_checked_now();

    char latest[128];
    if (!fetch_latest_tag(latest, sizeof(latest))) {
        return;
    }
    if (compare_versions(current_version, latest) < 0) {
        fprintf(stderr,
                "[anemo] Update available: %s (current %s). Run `anemo update`.\n",
                latest, current_version);
    }
}

int anemo_run_update(const char *current_version) {
    char latest[128];
    if (!fetch_latest_tag(latest, sizeof(latest))) {
        fprintf(stderr, "error: unable to reach GitHub releases API\n");
        return 1;
    }

    if (compare_versions(current_version, latest) >= 0) {
        printf("anemo is up to date (%s)\n", current_version);
        return 0;
    }

#ifdef _WIN32
    const char *repo = default_repo();
    char cmd[8192];
    snprintf(cmd, sizeof(cmd),
             "powershell -NoProfile -ExecutionPolicy Bypass -Command "
             "\"$ProgressPreference='SilentlyContinue';"
             "$repo='%s';"
             "$r=Invoke-RestMethod -UseBasicParsing -Uri ('https://api.github.com/repos/'+$repo+'/releases/latest') -Headers @{\\\"User-Agent\\\"='anemo'};"
             "$asset=$r.assets | Where-Object {$_.name -match '\\\\.msi$'} | Select-Object -First 1;"
             "if(-not $asset){throw 'No MSI asset found in latest release.'};"
             "$out=Join-Path $env:TEMP $asset.name;"
             "Invoke-WebRequest -UseBasicParsing -Uri $asset.browser_download_url -OutFile $out;"
             "Write-Host ('Downloaded: '+$out);"
             "Start-Process msiexec -ArgumentList ('/i `\"'+$out+'`\"') -Wait\"",
             repo);

    int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "error: update installer failed (exit code %d)\n", rc);
        return 1;
    }
    return 0;
#else
    fprintf(stderr, "error: automatic update is currently implemented for Windows only\n");
    return 1;
#endif
}
