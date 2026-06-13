#pragma once

#include "snuk/defines.h"
#include "snuk/lexer.h"
#include "snuk/snuk_error.h"

/*
 * We will have items similar to rust.
 * But since this is interpreted language, items cannot appear in any order.
 * Items should be defined or declared first before it can be used.
 *
 * In Snuk items are actually special kind of expressions, that may have some
 * restrictions (var, const declaration statements, cannot appear everywhere)
 * or may be syntax sugars (fn function() {} is syntax sugar of var function =
 * fn () {}) or keywords with special purpose (like print keyword) or may be
 * just expressions.
 *
 * Parser parses the source and returns items.
 * Everything else that produces a value is a `SnukExpr`.
 */

typedef struct SnukItem SnukItem;

/**
 * @brief Parser state for a single source buffer.
 */
typedef struct SnukParser {
    SnukLexer lexer; /**< Lexer used to produce tokens. */
    SnukToken previous; /**< previously consumed tokens. */
    SnukToken current; /**< Current token. */
    SnukToken next; /**< Next token. */

    SnMemoryAllocator *allocator;

    bool panic_mode; /**< Error and recovery state flags. */
    SnukError err;
} SnukParser;

/**
 * @brief Initialize a parser for the given source buffer.
 *
 * This initializes the embedded lexer and consumes the first token from the
 * source so parser->current is ready for parsing.
 *
 * @param parser Parser context to initialize.
 * @param src Null-terminated source text to parse.
 * @param alloc_data User data passed to alloc.
 * @param alloc Allocation callback used for parser nodes.
 *
 * @note The source text must remain valid for the lifetime of parsed nodes that
 * reference token text.
 */
SNUK_API void snuk_parser_init(SnukParser *parser, const char *src, SnMemoryAllocator *allocator);

/**
 * @brief Deinitialize a parser context.
 *
 * @param parser Parser context to deinitialize.
 */
SNUK_API void snuk_parser_deinit(SnukParser *parser);

/**
 * @brief Parse and return the next item from the source.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL when the parser reaches EOF.
 *
 * @note Returned items are allocated with the parser allocation callback.
 */
SNUK_API SnukItem *snuk_parser_next_item(SnukParser *parser);

/**
 * @brief Report a parser error and enter panic mode.
 *
 * @param parser Parser context to operate on.
 * @param code Parse error code.
 * @param msg Error message to print.
 */
void parser_error(SnukParser *parser, SnukParseError code, const char *msg);

/**
 * @brief Clear the parser error state.
 *
 * @param parser Parser context to operate on.
 *
 * @return Previous error value.
 */
SnukError snuk_parser_clear_error(SnukParser *parser);

/**
 * @brief Recover parser state after an error.
 *
 * @param parser Parser context to operate on.
 */
SnukItem *parser_sync(SnukParser *parser);
