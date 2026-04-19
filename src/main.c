/**
 * @file main.c
 * @brief Entry point for fastpurge.
 */

#include "fp_args.h"
#include "fp_scan.h"
#include "fp_ui.h"
#include "fp_worker.h"

#include <stdio.h>
#include <string.h>

#define FP_VERSION "1.0.0"

int main(int argc, char *argv[]) {
    fp_ui_init();

    if (argc < 2) { fp_ui_help(FP_VERSION); return 1; }
    if (strcmp(argv[1], "--version") == 0) { fp_ui_version(FP_VERSION); return 0; }
    if (strcmp(argv[1], "--help")    == 0) { fp_ui_help(FP_VERSION);    return 0; }

    FpConfig cfg;
    if (fp_args_parse(&cfg, argc, argv) != 0) {
        fp_ui_error("invalid arguments");
        fp_ui_help(FP_VERSION);
        return 1;
    }

    /* Scan */
    FpScanResult scan;
    if (fp_scan(&cfg, &scan) != 0) {
        fp_ui_error("scan failed");
        return 1;
    }

    fp_ui_scan_result(cfg.path, scan.count);

    if (scan.count == 0) {
        fp_ui_warn("no files matched — nothing to do");
        fp_scan_free(&scan);
        return 0;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "spawning %d worker(s)...", cfg.workers);
    fp_ui_info(msg);

    /* Run workers */
    FpRunResult result;
    if (fp_workers_run(scan.files, scan.count, &cfg, &result) != 0) {
        fp_ui_error("workers failed to spawn");
        fp_scan_free(&scan);
        return 1;
    }

    fp_scan_free(&scan);
    fp_ui_summary(&result, cfg.dry_run);
    return result.errors > 0 ? 1 : 0;
}
