#pragma once

#include "defines.h"

#include "lexer.h"
#include "string_view.h"

/**
 * @brief Parser statement node kinds.
 */
typedef enum SnukStmtType {
    SNUK_STMT_EXPR, /**< Expression statement. */

    SNUK_STMT_VAR_DECL, /**< Mutable variable declaration. */
    SNUK_STMT_CONST_DECL, /**< Constant declaration. */

    SNUK_STMT_IF, /**< If or else-if conditional statement. */
    SNUK_STMT_MATCH, /**< Match statement. */

    SNUK_STMT_WHILE, /**< While loop statement. */
    SNUK_STMT_DO_WHILE, /**< Do-while loop statement. */
    SNUK_STMT_FOR, /**< For loop statement. */

    SNUK_STMT_RETURN, /**< Return control-flow statement. */
    SNUK_STMT_BREAK, /**< Break control-flow statement. */
    SNUK_STMT_CONTINUE, /**< Continue control-flow statement. */

    SNUK_STMT_FN, /**< Function declaration statement. */

    SNUK_STMT_TYPE, /**< Type declaration statement. */

    SNUK_STMT_PRINT, /**< Print statement. */

    SNUK_STMT_BLOCK, /**< Statement block. */

    SNUK_STMT_SLCOMMENT, /**< Single-line comment statement. */
    SNUK_STMT_MLCOMMENT, /**< Multi-line comment statement. */

    SNUK_STMT_MAX /**< Sentinel value for statement kinds. */
} SnukStmtType;

/**
 * @brief Parser expression node kinds.
 */
typedef enum SnukExprType {
    SNUK_EXPR_IDENTIFIER, /**< Identifier reference expression. */
    SNUK_EXPR_INT_LITERAL, /**< Integer literal expression. */
    SNUK_EXPR_FLOAT_LITERAL, /**< Floating-point literal expression. */
    SNUK_EXPR_STRING_LITERAL, /**< String literal expression. */
    SNUK_EXPR_TRUE_LITERAL, /**< Boolean true literal expression. */
    SNUK_EXPR_FALSE_LITERAL, /**< Boolean false literal expression. */
    SNUK_EXPR_NULL_LITERAL, /**< Null literal expression. */

    SNUK_EXPR_UNARY, /**< Unary operator expression. */
    SNUK_EXPR_BINARY, /**< Binary operator expression. */

    SNUK_EXPR_ASSIGN, /**< Assignment expression. */
    SNUK_EXPR_COMPOUND_ASSIGN, /**< Compound assignment expression. */

    SNUK_EXPR_CALL, /**< Function call expression. */
    SNUK_EXPR_MEMBER, /**< Member access expression. */
    SNUK_EXPR_INDEX, /**< Index access expression. */

    SNUK_EXPR_MAX /**< Sentinel value for expression kinds. */
} SnukExprType;

typedef struct SnukExpr SnukExpr;

/**
 * @brief Parsed expression node.
 */
struct SnukExpr {
    SnukExprType type; /**< Discriminant selecting the active expression payload. */

    union {
        SnukStringView string_literal;
        int64_t int_literal;
        double float_literal;
        SnukStringView identifier;

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
            // TODO:
            SnukExpr **params; /**< Call argument expressions. */
            uint64_t count; /**< Number of call argument expressions. */
        } call;
    };
};

/**
 * @brief Parsed function parameter.
 */
typedef struct SnukParam {
    SnukExpr *identifier; /**< Parameter name expression. */
    // TODO: type
    SnukExpr *default_value; /**< Optional default value expression. */
} SnukParam;

typedef struct SnukStmt SnukStmt;

/**
 * @brief Parsed statement node.
 */
struct SnukStmt {
    SnukStmtType type; /**< Discriminant selecting the active statement payload. */

    union {
        SnukExpr *expr_stmt; /**< Expression statement payload. */

        struct {
            SnukExpr *identifier; /**< Declared variable or constant name. */
            SnukExpr *init; /**< Initializer expression. */
        } decl_stmt; // var or const

        struct {
            SnukExpr * condition; /**< Condition expression. */
            SnukStmt *then_branch; /**< Statement executed when the condition is true. */
            SnukStmt *else_branch; /**< Optional statement executed otherwise. */
        } if_stmt;

        struct {
            SnukExpr *value; /**< Value expression being matched. */
            // TODO:
        } match_stmt;

        struct {
            SnukExpr *condition; /**< Loop condition expression. */
            SnukStmt *block; /**< Loop body block. */
        } while_stmt;

        struct {
            SnukStmt *init; /**< Optional initializer statement. */
            SnukExpr *cond; /**< Optional loop condition expression. */
            SnukExpr *update; /**< Optional loop update expression. */
            SnukStmt *block; /**< Loop body block. */
        } for_stmt;

        SnukExpr *return_stmt; /**< Optional return value expression. */

        struct {
            SnukExpr *identifier; /**< Function name expression. */
            SnukParam **params; /**< Dynamic array of parameter nodes. */
            SnukStmt *body; /**< Function body block. */
        } fn_stmt;

        struct {
            SnukExpr *identifier; /**< Type name expression. */
            SnukStmt **vars; /**< Dynamic array of field declarations. */
            SnukStmt **fns; /**< Dynamic array of method declarations. */
        } type_stmt;

        struct {
            SnukExpr **exprs; /**< Dynamic array of expressions to print. */
        } print_stmt;

        struct {
            SnukStmt **stmts; /**< Dynamic array of statements in the block. */
        } block_stmt;

        SnukStringView comment;
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
 * @brief Parse and return the next statement from the source.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL when the parser reaches EOF.
 *
 * @note Returned nodes are allocated with the parser allocation callback.
 */
SnukStmt *snuk_parser_next_stmt(SnukParser *parser);

/**
 * @brief Get a string name for a statement type.
 *
 * @param type Statement type to convert.
 *
 * @return Static string describing the statement type.
 */
const char *snuk_parser_stmt_type_to_string(SnukStmtType type);

/**
 * @brief Get a string name for an expression type.
 *
 * @param type Expression type to convert.
 *
 * @return Static string describing the expression type.
 */
const char *snuk_parser_expr_type_to_string(SnukExprType type);

/**
 * @brief Log a parsed statement tree.
 *
 * @param stmt Statement to log.
 */
void snuk_parser_log_stmt(SnukStmt *stmt);

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
