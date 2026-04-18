/**
 * @file worker.c
 * @brief Parallel worker process management for fastpurge.
 *
 * Each worker receives a contiguous slice of the file list.
 * Results are sent to the parent via a pipe as a FpWorkerResult.
 * SIGINT is caught to allow clean shutdown.
 */

#include "fp_worker.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ── Internal: signal handling ───────────────────────────────────────────── */

static volatile sig_atomic_t fp_stop = 0;

static void on_sigint(int sig) { (void)sig; fp_stop = 1; }

/* ── Internal: wall-clock timer ──────────────────────────────────────────── */

static double now_sec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

/* ── Internal: zero-overwrite before unlink ──────────────────────────────── */

/**
 * @brief Overwrites a file with zeros before deletion.
 *
 * Opens the file for writing and writes zeros in 64 KB chunks.
 * Best-effort — if it fails, deletion still proceeds.
 *
 * @param path File to overwrite.
 */
static void zero_file(const char *path) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return;

    static const char zeros[65536]; /* zero-initialized by C standard */
    ssize_t n;
    while ((n = write(fd, zeros, sizeof(zeros))) > 0)
        ; /* keep writing until EOF or error */
    close(fd);
}

/* ── Internal: worker body (runs in child process) ───────────────────────── */

/**
 * @brief Deletes a slice of files, reports result via pipe.
 *
 * @param id      Worker index (0-based).
 * @param files   Full file array.
 * @param start   First index for this worker.
 * @param end     One-past-last index for this worker.
 * @param cfg     Run configuration.
 * @param pipe_wr Write end of result pipe.
 */
static void worker_run(int id, char **files, long start, long end,
                       const FpConfig *cfg, int pipe_wr) {
    signal(SIGINT, on_sigint);

    /* Silence child stdout — parent owns terminal output */
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) { dup2(devnull, STDOUT_FILENO); close(devnull); }

    FpWorkerResult res;
    memset(&res, 0, sizeof(res));
    res.worker_id = id;

    double t0 = now_sec();

    for (long i = start; i < end && !fp_stop; i++) {
        if (cfg->dry_run) {
            res.deleted++;
            continue;
        }
        if (cfg->zero) zero_file(files[i]);
        if (unlink(files[i]) == 0)
            res.deleted++;
        else
            res.errors++;
    }

    res.elapsed = now_sec() - t0;

    /* Send result to parent — write is atomic for small structs */
    if (write(pipe_wr, &res, sizeof(res)) < 0) { /* best-effort */ }
    close(pipe_wr);
    exit(0);
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int fp_workers_run(char **files, long count,
                   const FpConfig *cfg, FpRunResult *out) {
    memset(out, 0, sizeof(*out));
    out->total_files = count;

    if (count == 0) return 0;

    int  n       = cfg->workers;
    int *pipe_rd = (int *)malloc((size_t)n * sizeof(int));
    if (!pipe_rd) return -1;

    signal(SIGINT, on_sigint);

    double t0 = now_sec();

    /* Spawn workers */
    for (int i = 0; i < n; i++) {
        int fds[2];
        if (pipe(fds) != 0) {
            free(pipe_rd);
            return -1;
        }

        long slice = count / n;
        long start = (long)i * slice;
        long end   = (i == n - 1) ? count : start + slice;

        pid_t pid = fork();
        if (pid < 0) {
            close(fds[0]); close(fds[1]);
            free(pipe_rd);
            return -1;
        }

        if (pid == 0) {
            /* Child */
            close(fds[0]);
            worker_run(i, files, start, end, cfg, fds[1]);
            /* worker_run calls exit() — never returns */
        }

        /* Parent */
        close(fds[1]);
        pipe_rd[i] = fds[0];
    }

    /* Collect results from all workers */
    for (int i = 0; i < n; i++) {
        FpWorkerResult res;
        ssize_t r = read(pipe_rd[i], &res, sizeof(res));
        close(pipe_rd[i]);

        if (r == (ssize_t)sizeof(res)) {
            out->deleted += res.deleted;
            out->errors  += res.errors;
        }
    }

    /* Wait for all children */
    while (waitpid(-1, NULL, 0) > 0)
        ;

    out->elapsed = now_sec() - t0;
    free(pipe_rd);
    return 0;
}
