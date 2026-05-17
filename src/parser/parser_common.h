#pragma once

#include "defines.h"
#include "logger.h"
#include "parser.h"

typedef struct SnukItem SnukItem;
typedef struct SnukExpr SnukExpr;
typedef struct SnukType SnukType;
typedef struct SnukParam SnukParam;

typedef enum ParseFlag {
    PARSE_FLAG_NORMAL,
    PARSE_FLAG_STOP_LBRACE,

    PARSE_FLAG_MAX
} ParseFlag;

/**
 * @brief Advance to the next token.
 *
 * @param parser Parser context to operate on.
 */
SNUK_INLINE void parser_advance(SnukParser *parser) {
    parser->previous = parser->current;
    parser->current = snuk_lexer_next_token(&parser->lexer);
}

/**
 * @brief Check whether the current token has the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 *
 * @return True when the current token matches expected.
 */
SNUK_INLINE bool parser_check(SnukParser *parser, SnukTokenType expected) {
    // Does not consume
    return parser->current.type == expected;
}

/**
 * @brief Consume the current token if it has the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 *
 * @return True when a token was consumed.
 */
SNUK_INLINE bool parser_match(SnukParser *parser, SnukTokenType expected) {
    // Consumes
    if (!parser_check(parser, expected)) return false;
    parser_advance(parser);
    return true;
}

/**
 * @brief Require and consume a token of the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 * @param err_msg Error message to report if the token does not match.
 */
SNUK_INLINE void parser_expect(
    SnukParser *parser, SnukTokenType expected, const char *err_msg) {
    if (!parser_match(parser, expected)) parser_error(parser, err_msg);
}

/**
 * @brief Consume item end token if exists.
 *
 * @param parser Parser context to operate on.
 *
 * @return True when token is consumed.
 */
SNUK_INLINE bool parser_match_item_end(SnukParser *parser) {
    return parser_match(parser, SNUK_TOKEN_SEMICOLON)
        || parser_match(parser, SNUK_TOKEN_VSEMICOLON)
        || parser_match(parser, SNUK_TOKEN_EOF);
}

/**
 * @brief Require and consume item end token.
 *
 * @param parser Parser context to operate on.
 */
SNUK_INLINE void parser_expect_item_end(SnukParser *parser) {
    if (!parser_match_item_end(parser))
        parser_error(parser, "expected a new line or a semicolon");
}
