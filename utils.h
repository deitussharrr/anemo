#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);

void fatal(const char *fmt, ...);
void fatal_at(const char *file, int line, int col, const char *fmt, ...);

char *read_file_all(const char *path, size_t *out_size);
char *path_stem(const char *path);
int has_extension(const char *path, const char *ext);

#endif