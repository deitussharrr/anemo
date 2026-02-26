#include "utils.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fatal("out of memory allocating %zu bytes", size);
    }
    return p;
}

void *xcalloc(size_t count, size_t size) {
    void *p = calloc(count, size);
    if (!p) {
        fatal("out of memory allocating %zu bytes", count * size);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p) {
        fatal("out of memory reallocating %zu bytes", size);
    }
    return p;
}

char *xstrdup(const char *s) {
    size_t n = strlen(s);
    char *copy = xmalloc(n + 1);
    memcpy(copy, s, n + 1);
    return copy;
}

static void vreport(const char *prefix, const char *fmt, va_list ap) {
    fprintf(stderr, "%s", prefix);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

void fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vreport("error: ", fmt, ap);
    va_end(ap);
    exit(1);
}

void fatal_at(const char *file, int line, int col, const char *fmt, ...) {
    fprintf(stderr, "%s:%d:%d: error: ", file, line, col);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

char *read_file_all(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fatal("cannot open '%s': %s", path, strerror(errno));
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        fatal("cannot seek '%s'", path);
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        fatal("cannot tell size for '%s'", path);
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        fatal("cannot seek '%s'", path);
    }

    char *buf = xmalloc((size_t)size + 1);
    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (n != (size_t)size) {
        fatal("cannot read all bytes from '%s'", path);
    }
    buf[n] = '\0';
    if (out_size) {
        *out_size = n;
    }
    return buf;
}

char *path_stem(const char *path) {
    const char *slash1 = strrchr(path, '/');
    const char *slash2 = strrchr(path, '\\');
    const char *base = path;
    if (slash1 && slash2) {
        base = (slash1 > slash2 ? slash1 : slash2) + 1;
    } else if (slash1) {
        base = slash1 + 1;
    } else if (slash2) {
        base = slash2 + 1;
    }

    const char *dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);
    char *out = xmalloc(len + 1);
    memcpy(out, base, len);
    out[len] = '\0';
    return out;
}

int has_extension(const char *path, const char *ext) {
    size_t lp = strlen(path);
    size_t le = strlen(ext);
    if (lp < le) {
        return 0;
    }
    return strcmp(path + (lp - le), ext) == 0;
}