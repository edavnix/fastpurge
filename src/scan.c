/**
 * @file scan.c
 * @brief Recursive directory scanner for fastpurge.
 */

#define _POSIX_C_SOURCE 200809L
#include "fp_scan.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* ── Dynamic file list ───────────────────────────────────────────────────── */

typedef struct {
    char **files;
    long   count;
    long   cap;
} FileList;

static int fl_push(FileList *fl, const char *path) {
    if (fl->count >= fl->cap) {
        long new_cap = fl->cap ? fl->cap * 2 : 4096;
        char **tmp = (char **)realloc(fl->files,
                                      (size_t)new_cap * sizeof(char *));
        if (!tmp) return -1;
        fl->files = tmp;
        fl->cap   = new_cap;
    }
    fl->files[fl->count] = strdup(path);
    if (!fl->files[fl->count]) return -1;
    fl->count++;
    return 0;
}

/* ── Recursive walk ──────────────────────────────────────────────────────── */

static void walk(const char *dir, const FpConfig *cfg,
                 FileList *fl, time_t cutoff) {
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        /* Apply exclude rule */
        if (cfg->exclude &&
            strcmp(entry->d_name, cfg->exclude) == 0) continue;

        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (lstat(full, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            walk(full, cfg, fl, cutoff);
        } else if (S_ISREG(st.st_mode)) {
            /* Age filter */
            if (cutoff > 0 && st.st_mtime > cutoff) continue;
            fl_push(fl, full);
        }
    }
    closedir(d);
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int fp_scan(const FpConfig *cfg, FpScanResult *result) {
    memset(result, 0, sizeof(*result));

    time_t cutoff = 0;
    if (cfg->max_age > 0)
        cutoff = time(NULL) - (time_t)cfg->max_age * 86400;

    FileList fl = {NULL, 0, 0};
    walk(cfg->path, cfg, &fl, cutoff);

    result->files = fl.files;
    result->count = fl.count;
    return 0;
}

void fp_scan_free(FpScanResult *result) {
    if (!result) return;
    for (long i = 0; i < result->count; i++)
        free(result->files[i]);
    free(result->files);
    memset(result, 0, sizeof(*result));
}
