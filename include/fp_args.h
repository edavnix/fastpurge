/**
 * @file fp_args.h
 * @brief CLI argument parsing for fastpurge.
 */

#ifndef FP_ARGS_H
#define FP_ARGS_H

#include "fp_types.h"

/**
 * @brief Parses argc/argv into a @ref FpConfig.
 * @param cfg  Output config struct.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, -1 on invalid arguments.
 */
int fp_args_parse(FpConfig *cfg, int argc, char *argv[]);

#endif /* FP_ARGS_H */
