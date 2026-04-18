/**
 * @file fp_worker.h
 * @brief Parallel worker process management for fastpurge.
 *
 * Uses fork() + pipe() to spawn N worker processes. Each worker
 * receives a slice of the file list and reports results back to
 * the parent via a pipe as a FpWorkerResult struct.
 */

#ifndef FP_WORKER_H
#define FP_WORKER_H

#include "fp_types.h"

/**
 * @brief Spawns N worker processes to delete a slice of files each.
 *
 * Blocks until all workers finish (waitpid). Aggregates results
 * from all workers into @p out.
 *
 * @param files   Full file list from fp_scan().
 * @param count   Number of files.
 * @param cfg     Run configuration (workers, zero, dry_run).
 * @param out     Output aggregate result.
 * @return 0 on success, -1 if any worker failed to spawn.
 */
int fp_workers_run(char **files, long count,
                   const FpConfig *cfg, FpRunResult *out);

#endif /* FP_WORKER_H */
