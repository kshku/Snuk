#pragma once

#include "defines.h"

#include "lexer.h"
#include "string_view.h"

/*
 * We will have Items similar to Rust.
 * But since this is interpreted language, items cannot appear in any order.
 * Items should be defined or declared first before it can be used.
 *
 * In Snuk items are actually special kind of expressions, that may have some
 * restrictions (var, const declaration statements, cannot appear everywhere)
 * or may be syntax sugars (fn function() {} is syntax sugar of var function = fn () {})
 * or keywords with special purpose (like print keyword)
 * or may be just expressions.
 *
 * Parser parses the source and returns items.
 * Everything else that produces a value is a `SnukExpr`.
 */

/**
 * @brief Parser items kinds.
 */
typedef enum SnukItemType {
    SNUK_ITEM_EXPR, /**< Expression items */

    SNUK_ITEM_VAR_DECL, /**< Variable declaration (expression with restriction) */
    SNUK_ITEM_CONST_DECL, /**< Constant declaration (expression with restriction) */

    SNUK_ITEM_FN_DECL, /**< Function declaration (syntax sugar) */
    SNUK_ITEM_TYPE_DECL, /**< Type declaration (syntax sugar) */

    SNUK_ITEM_PRINT, /**< Printing expression (special expression, always returns null) */

    SNUK_ITEM_RETURN, /**< Return expression, transfer control out of function, may carry value with them */
    SNUK_ITEM_BREAK, /**< Transfer control out of loop, may carray value with them. */
    SNUK_ITEM_CONTINUE, /**< Transfer control to next iteration. */

    SNUK_ITEM_LINE_COMMENT, /**< Single line comment */
    SNUK_ITEM_BLOCK_COMMENT, /**< Multi line comment */

    SNUK_ITEM_MAX /**< Sentinel value for item kinds. */
} SnukItemType;

/**
 * @brief Parser expression node kinds.
 */
typedef enum SnukExprType {
    SNUK_EXPR_IDENTIFIER, /**< Identifier reference expression. */
    SNUK_EXPR_INT, /**< Integer literal expression. */
    SNUK_EXPR_FLOAT, /**< Floating-point literal expression. */
    SNUK_EXPR_STRING, /**< String literal expression. */
    SNUK_EXPR_BOOL, /**< Boolean literal expression. */
    SNUK_EXPR_NULL, /**< Null literal expression. */

    SNUK_EXPR_UNARY, /**< Unary operator expression. */
    SNUK_EXPR_BINARY, /**< Binary operator expression. */

    SNUK_EXPR_ASSIGN, /**< Assignment expression. */
    SNUK_EXPR_COMPOUND_ASSIGN, /**< Compound assignment expression. */

    SNUK_EXPR_IF, /**< If expression. */
    SNUK_EXPR_MATCH, /**< Match expression. */

    SNUK_EXPR_WHILE, /**< while loop expression. */
    SNUK_EXPR_DO_WHILE, /**< do-while loop expression. */
    SNUK_EXPR_FOR, /**< for loop expression. */

    SNUK_EXPR_FN, /**< Funtion expression */
    SNUK_EXPR_TYPE, /**< Type expression */

    SNUK_EXPR_BLOCK, /**< Block expression */

    SNUK_EXPR_CALL, /**< Function call expression. */
    SNUK_EXPR_MEMBER, /**< Member access expression. */
    SNUK_EXPR_INDEX, /**< Index access expression. */

    SNUK_EXPR_MAX /**< Sentinel value for expression kinds. */
} SnukExprType;

typedef struct SnukItem SnukItem;
typedef struct SnukExpr SnukExpr;
typedef struct SnukParam SnukParam;
typedef struct SnukType SnukType;

/**
 * @brief Parsed function parameter.
 */
struct SnukParam {
    SnukExpr *identifier; /**< Parameter name expression. */
    SnukType *type; /**< Type information of the parameter */
    SnukExpr *default_value; /**< Optional default value expression. */
};

/**
 * @brief Parsed type.
 */
struct SnukType {
    enum {
        TYPE_ANY, /**< No type annotation */
        TYPE_NAMED, /**< Named type */
        TYPE_FN, /**< Function type */

        TYPE_MAX /**< Sentinel value for type kinds. */
    } type;

    union {
        SnukStringView name; /**< Type name (for named parameters) */

        struct {
            SnukType *return_type; /**< The return type of function */
            SnukType **param_types; /**< Darray of parameter types */
        } fn;
    };
};

/**
 * @brief Parsed item.
 */
struct SnukItem {
    SnukItemType type; /**< Discriminant selecting the active item payload. */

    union {
        SnukExpr *expr; /**< expression item payload (also used for return and break). */

        struct {
            SnukExpr *identifier; /**< Declared variable or constant name. */
            SnukType *type; /**< Type information of the variable or constant. */
            SnukExpr *init; /**< Initializer expression. */
        } var_decl; // var or const

        struct {
            SnukExpr *identifier; /**< Name of function. */
            SnukExpr *fn_expr; /**< Function expression. */
        } fn_decl;

        struct {
            SnukExpr *identifier; /**< Name of the type. */
            SnukExpr *type_expr; /**< Type expression. */
        } type_decl;

        SnukExpr **print_exprs; /**< Dynamic array of expressions to print. */

        SnukStringView comment; /**< Comment */
    };
};

/**
 * @brief Parsed expression node.
 */
struct SnukExpr {
    SnukExprType type; /**< Discriminant selecting the active expression payload. */

    union {
        SnukStringView identifier;
        int64_t int_literal;
        double float_literal;
        SnukStringView string_literal;
        bool bool_literal;

        struct {
            SnukTokenType op; /**< Unary operator token. */
            SnukExpr *operand; /**< Unary operand expression. */
        } unary;

        struct {
            SnukTokenType op; /**< Binary operator token. */
            SnukExpr *left; /**< Left-hand operand expression. */
            SnukExpr *right; /**< Right-hand operand expression. */
        } binary;

        struct {
            SnukExpr *identifier; /**< Assignment target identifier expression. */
            SnukExpr *value; /**< Assigned value expression. */
        } assign;

        struct {
            SnukTokenType op; /**< Compound assignment token */
            SnukExpr *identifier; /**< Assignment target identifier expression. */
            SnukExpr *value; /**< Assigned value expression. */
        } compound_assign;

        struct {
            SnukExpr *condition; /**< Condition expression. */
            SnukExpr *then_block; /**< Block expression to execute on true condition */
            SnukExpr *else_block; /**< Block expression to execute on false condition */
        } if_else;

        struct {
            SnukExpr *value; /**< Value expression being matched. */
            // TODO:
        } match;

        struct {
            SnukExpr *condition; /**< Loop condition expression. */
            SnukExpr *body; /**< Loop body block. */
        } while_loop; // while, do while

        struct {
            SnukItem *init; /**< Optional initializer. */
            SnukExpr *condition; /**< Optional loop condition expression. */
            SnukExpr *update; /**< Optional loop update expression. */
            SnukExpr *body; /**< Loop body block. */
        } for_loop;

        struct {
            SnukParam **params; /**< Darray of parameters. */
            SnukExpr *body; /**< Body of function */
            SnukType *return_type; /**< Return type of function */
        } fn_expr;

        struct {
            // TODO:
            SnukItem **vars; //**< Dynamic array of field declarations. */
            SnukItem **fns; //**< Dynamic array of method declarations. */
        } type_expr;

        SnukItem **block_items; /**< Dynamic array of items in the block. */

        struct {
            // TODO:
            SnukExpr **params; /**< Darray of call argument expressions. */
        } call;
    };
};

/**
 * @brief Parser allocation callback.
 *
 * @param data Allocator user data.
 * @param size Allocation size in bytes.
 * @param align Required allocation alignment.
 *
 * @return Pointer to allocated storage, or NULL on failure.
 */
typedef void *(*alloc_fn)(void *data, uint64_t size, uint64_t align);

/**
 * @brief Parser state for a single source buffer.
 */
typedef struct SnukParser {
    SnukLexer lexer; /**< Lexer used to produce tokens. */
    SnukToken current, previous; /**< Current and previously consumed tokens. */

    // allocation data
    void *alloc_data; /**< User data passed to the allocation callback. */
    alloc_fn alloc; /**< Allocation callback used for parser nodes. */

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
SNUK_INLINE void snuk_parser_init(SnukParser *parser, const char *src, void *alloc_data, alloc_fn alloc) {
    *parser = (SnukParser){
        .alloc_data = alloc_data,
        .alloc = alloc,
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
 * @brief Get a string name for a item type.
 *
 * @param type Item type to convert.
 *
 * @return Static string describing the item type.
 */
const char *snuk_parser_item_type_to_string(SnukItemType type);

/**
 * @brief Get a string name for an expression type.
 *
 * @param type Expression type to convert.
 *
 * @return Static string describing the expression type.
 */
const char *snuk_parser_expr_type_to_string(SnukExprType type);

/**
 * @brief Log a parsed item tree.
 *
 * @param item item to log.
 */
void snuk_parser_log_item(SnukItem *item);

/**
 * @brief Log a parsed expression tree.
 *
 * @param expr Expression to log.
 */
void snuk_parser_log_expr(SnukExpr *expr);

/**
 * @brief Log a parsed function parameter.
 *
 * @param param Parameter to log.
 */
void snuk_parser_log_param(SnukParam *param);

/**
 * @brief Log a parsed type annotation.
 *
 * @param type The type to log.
 */
void snuk_parser_log_type(SnukType *type);
