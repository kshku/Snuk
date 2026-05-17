#pragma once

#include "defines.h"
#include "lexer.h"

/*
 * We will have Items similar to Rust.
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
        SnukToken current,
            previous; /**< Current and previously consumed tokens. */

        SnukAllocator *allocator;

        bool had_error, panic_mode; /**< Error and recovery state flags. */
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
SNUK_INLINE void snuk_parser_init(
    SnukParser *parser, const char *src, SnukAllocator *allocator) {
    *parser = (SnukParser){
        .allocator = allocator,
        .had_error = false,
        .panic_mode = false,
    };
    snuk_lexer_init(&parser->lexer, src);

    parser->previous = (SnukToken){0};
    parser->current = snuk_lexer_next_token(&parser->lexer);
}

/**
 * @brief Deinitialize a parser context.
 *
 * @param parser Parser context to deinitialize.
 */
SNUK_INLINE void snuk_parser_deinit(SnukParser *parser) {
    if (!parser) return;
    snuk_lexer_deinit(&parser->lexer);
    *parser = (SnukParser){0};
}

/**
 * @brief Parse and return the next item from the source.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL when the parser reaches EOF.
 *
 * @note Returned items are allocated with the parser allocation callback.
 */
SnukItem *snuk_parser_next_item(SnukParser *parser);

/**
 * @brief Report a parser error and enter panic mode.
 *
 * @param parser Parser context to operate on.
 * @param err_msg Error message to print.
 */
void parser_error(SnukParser *parser, const char *err_msg);

/**
 * @brief Recover parser state after an error.
 *
 * @param parser Parser context to operate on.
 */
void parser_sync(SnukParser *parser);
