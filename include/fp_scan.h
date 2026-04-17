/**
 * @file fp_scan.h
 * @brief Recursive directory scanner for fastpurge.
 */

#ifndef FP_SCAN_H
#define FP_SCAN_H

#include "fp_types.h"

/**
 * @brief Recursively collects files under @p cfg->path.
 *
 * Applies the age filter (cfg->max_age) and exclude rule (cfg->exclude).
 * The caller must free each string in result->files and then result->files.
 *
 * @param cfg    Run configuration.
 * @param result Output scan result.
 * @return 0 on success, -1 on error.
 */
int fp_scan(const FpConfig *cfg, FpScanResult *result);

/**
 * @brief Frees all memory inside a @ref FpScanResult.
 * @param result Result to free.
 */
void fp_scan_free(FpScanResult *result);

#endif /* FP_SCAN_H */
