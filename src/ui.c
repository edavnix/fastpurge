/**
 * @file ui.c
 * @brief Terminal output for fastpurge.
 */

#include "fp_ui.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int fp_color = 0;

#define COL_RESET  "\033[0m"
#define COL_BOLD   "\033[1m"
#define COL_RED    "\033[31m"
#define COL_GREEN  "\033[32m"
#define COL_YELLOW "\033[33m"
#define COL_CYAN   "\033[36m"
#define COL_GRAY   "\033[90m"
#define BG_RED     "\033[41m\033[97m"
#define BG_GREEN   "\033[42m\033[30m"
#define BG_YELLOW  "\033[43m\033[30m"
#define BG_CYAN    "\033[46m\033[30m"

static const char *c(const char *code) {
    return fp_color ? code : "";
}


void fp_ui_init(void) { fp_color = isatty(STDOUT_FILENO); }

void fp_ui_info(const char *msg) {
    if (fp_color) printf("%s info %s  %s\n", BG_CYAN,   COL_RESET, msg);
    else          printf("info   %s\n", msg);
}

void fp_ui_warn(const char *msg) {
    if (fp_color) printf("%s warn %s  %s\n", BG_YELLOW, COL_RESET, msg);
    else          printf("warn   %s\n", msg);
}

void fp_ui_error(const char *msg) {
    if (fp_color) fprintf(stderr, "%s error %s  %s\n", BG_RED, COL_RESET, msg);
    else          fprintf(stderr, "error  %s\n", msg);
}

void fp_ui_scan_result(const char *path, long count) {
    printf("%s[fastpurge]%s Scanning %s%s%s ... found %s%ld%s files.\n",
           c(COL_BOLD),   c(COL_RESET),
           c(COL_CYAN),   path,  c(COL_RESET),
           c(COL_YELLOW), count, c(COL_RESET));
}

void fp_ui_worker_result(const FpWorkerResult *r) {
    const char *err_col = r->errors > 0 ? c(COL_RED) : c(COL_GREEN);
    printf("  %sWorker %d%s: deleted %s%ld%s files "
           "(%s%ld error(s)%s)  %.2fs\n",
           c(COL_BOLD),   r->worker_id, c(COL_RESET),
           c(COL_GREEN),  r->deleted,   c(COL_RESET),
           err_col,       r->errors,    c(COL_RESET),
           r->elapsed);
}

void fp_ui_summary(const FpRunResult *r, int dry_run) {
    printf("\n");
    if (dry_run) {
        printf("%s done %s  %s[dry-run]%s  would delete "
               "%s%ld%s file(s)\n",
               c(BG_CYAN),   c(COL_RESET),
               c(COL_CYAN),  c(COL_RESET),
               c(COL_BOLD),  r->deleted, c(COL_RESET));
        return;
    }

    const char *bg = r->errors > 0 ? BG_YELLOW : BG_GREEN;
    printf("%s done %s  %s%ld%s deleted  "
           "%s%ld error(s)%s  "
           "%s%.2fs%s\n",
           c(bg),         c(COL_RESET),
           c(COL_BOLD),   r->deleted,  c(COL_RESET),
           r->errors > 0 ? c(COL_RED) : c(COL_GRAY),
           r->errors,     c(COL_RESET),
           c(COL_GRAY),   r->elapsed,  c(COL_RESET));
}

void fp_ui_help(const char *version) {
    printf("%sfastpurge%s %s — parallel file deletion tool\n\n",
           c(COL_BOLD), c(COL_RESET), version);

    printf("%sUSAGE%s\n  fastpurge [options] <path>\n\n", c(COL_BOLD), c(COL_RESET));

    printf("%sOPTIONS%s\n", c(COL_BOLD), c(COL_RESET));
    printf("  %s-p <n>%s          Number of parallel workers (default: 4)\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s-a <days>%s       Only delete files older than N days\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s-z%s              Overwrite with zeros before deleting\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s--dry%s           Preview deletions without removing files\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s--exclude=<name>%s Skip subdirectory named <name>\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s--version%s       Print version and exit\n",
           c(COL_YELLOW), c(COL_RESET));
    printf("  %s--help%s          Print this help and exit\n\n",
           c(COL_YELLOW), c(COL_RESET));

    printf("%sEXAMPLES%s\n", c(COL_BOLD), c(COL_RESET));
    printf("  fastpurge -p 4 ~/.cache/google-chrome/\n");
    printf("  fastpurge -a 7 -p 2 ~/.thumbnails/\n");
    printf("  fastpurge -z --dry ~/temp/sensitive/\n");
    printf("  fastpurge --exclude=.git ./build/\n\n");
}

void fp_ui_version(const char *version) {
    printf("fastpurge %s\n", version);
}
