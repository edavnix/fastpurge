/**
 * @file fp_ui.h
 * @brief Terminal output for fastpurge.
 */

#ifndef FP_UI_H
#define FP_UI_H

#include "fp_types.h"

/** @brief Initializes color detection (TTY check). */
void fp_ui_init(void);

/** @brief Prints full help text. @param version Version string. */
void fp_ui_help(const char *version);

/** @brief Prints version string. @param version Version string. */
void fp_ui_version(const char *version);

/** @brief Prints an info message. */
void fp_ui_info(const char *msg);

/** @brief Prints a warning message. */
void fp_ui_warn(const char *msg);

/** @brief Prints an error message to stderr. */
void fp_ui_error(const char *msg);

/**
 * @brief Prints scan result line.
 * @param path  Scanned path.
 * @param count Files found.
 */
void fp_ui_scan_result(const char *path, long count);

/**
 * @brief Prints a single worker result.
 * @param r Worker result struct.
 */
void fp_ui_worker_result(const FpWorkerResult *r);

/**
 * @brief Prints the final run summary.
 * @param r       Aggregate run result.
 * @param dry_run 1 if dry-run mode was active.
 */
void fp_ui_summary(const FpRunResult *r, int dry_run);

#endif /* FP_UI_H */
