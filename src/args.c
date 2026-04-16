/**
 * @file args.c
 * @brief CLI argument parsing for fastpurge.
 */

#include "fp_args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fp_args_parse(FpConfig *cfg, int argc, char *argv[]) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->workers = 4; /* default */

    if (argc < 2) return -1;

    /* Last non-flag argument is the path */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dry") == 0) {
            cfg->dry_run = 1;
        } else if (strcmp(argv[i], "-z") == 0) {
            cfg->zero = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error  -p requires a number\n");
                return -1;
            }
            cfg->workers = atoi(argv[++i]);
            if (cfg->workers < 1 || cfg->workers > 64) {
                fprintf(stderr, "error  -p must be between 1 and 64\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error  -a requires a number of days\n");
                return -1;
            }
            cfg->max_age = atoi(argv[++i]);
            if (cfg->max_age < 1) {
                fprintf(stderr, "error  -a must be >= 1\n");
                return -1;
            }
        } else if (strncmp(argv[i], "--exclude=", 10) == 0) {
            cfg->exclude = argv[i] + 10;
        } else if (argv[i][0] != '-') {
            cfg->path = argv[i];
        } else {
            fprintf(stderr, "error  unknown option: %s\n", argv[i]);
            return -1;
        }
    }

    if (!cfg->path) return -1;
    return 0;
}
