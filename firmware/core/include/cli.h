/**
 * @file cli.h
 * @brief Line-oriented UART command-line interface (see docs/cli-spec.md).
 *
 * Transport-agnostic: the port feeds received bytes into cli_feed_char();
 * responses go out through esn_hal_cli_write(). Every response line is
 * terminated with "\r\n". Machine-parsable convention:
 *   - success lines start with "OK"  (optionally "OK <payload>")
 *   - failure lines start with "ERR <reason>"
 */
#ifndef CLI_H
#define CLI_H

#include <stddef.h>

#define CLI_MAX_LINE 64

/** Feed one received character into the CLI line buffer.
 *  Executes the command when '\n' (or '\r') is received. */
void cli_feed_char(char c);

/** Execute a complete command line directly (used by tests / ports). */
void cli_execute(const char *line);

#endif /* CLI_H */
