#include "snuk_expr.h"

#include "darray.h"
#include "snuk_item.h"
#include "snuk_param.h"
#include "snuk_type.h"

/**
 * @brief Pratt parser precedence levels.
 */
typedef enum Precedence {
    PRECEDENCE_NONE = 0, /**< No binding precedence. */
    PRECEDENCE_ASSIGNMENT, /**< Assignment precedence for '='. */
    PRECEDENCE_LOGICAL_OR, /**< Logical OR precedence for '||'. */
    PRECEDENCE_LOGICAL_AND, /**< Logical AND precedence for '&&'. */
    PRECEDENCE_OR, /**< Bitwise OR precedence for '|'. */
    PRECEDENCE_XOR, /**< Bitwise XOR precedence for '^'. */
    PRECEDENCE_AND, /**< Bitwise AND precedence for '&'. */
    PRECEDENCE_EQUALITY, /**< Equality precedence for '==' and '!='. */
    PRECEDENCE_COMPARISION, /**< Comparison precedence for '<', '>', '<=', and
                               '>='. */
    PRECEDENCE_SHIFT, /**< Shift precedence for '<<' and '>>'. */
    PRECEDENCE_TERM, /**< Additive precedence for '+' and '-'. */
    PRECEDENCE_FACTOR, /**< Multiplicative precedence for '*', '/', and '%'. */
    PRECEDENCE_UNARY, /**< Unary operator precedence. */
    PRECEDENCE_PRIMARY /**< Primary expression precedence. */
} Precedence;

/**
 * @brief Prefix parse function for tokens that start expressions.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
typedef SnukExpr *(*prefix_fn)(SnukParser *parser, ParseFlag flag);

/**
 * @brief Infix parse function for tokens that continue expressions.
 *
 * @param parser Parser context to operate on.
 * @param expr Left-hand expression already parsed.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
typedef SnukExpr *(*infix_fn)(
    SnukParser *parser, SnukExpr *expr, ParseFlag flag);

/**
 * @brief Pratt parser rule for a token type.
 */
typedef struct ParseRule {
        prefix_fn pfn; /**< Prefix parse function, or NULL if unsupported. */
        infix_fn ifn; /**< Infix parse function, or NULL if unsupported. */
        Precedence
            precedence; /**< Binding precedence for the infix function. */
} ParseRule;

/**
 * @brief Parse an expression at or above the given precedence.
 *
 * @param parser Parser context to operate on.
 * @param precedence Minimum precedence to parse.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_precedence(
    SnukParser *parser, Precedence precedence, ParseFlag flag);

/**
 * @brief Parse a primary expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_primary(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse a grouped expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_grouping(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse a unary expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_unary(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse a binary expression.
 *
 * @param parser Parser context to operate on.
 * @param left Left-hand expression.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_binary(
    SnukParser *parser, SnukExpr *left, ParseFlag flag);

/**
 * @brief Parse an assignment expression.
 *
 * @param parser Parser context to operate on.
 * @param left Candidate assignment target.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_assignment(
    SnukParser *parser, SnukExpr *left, ParseFlag flag);

/**
 * @brief Parse an compound assignment expression.
 *
 * @param parser Parser context to operate on.
 * @param left Candidate assignment target.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_compound_assignment(
    SnukParser *parser, SnukExpr *left, ParseFlag flag);

/**
 * @brief Parse an if expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_if(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an match expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_match(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an while or do while loop expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_while(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an for loop expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_for(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an function expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_fn(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an type expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_type(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse an block expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_block(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse function call expression.
 *
 * @param parser Parser context to operate on.
 * @param left Name of the function
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_call(SnukParser *parser, SnukExpr *left, ParseFlag flag);

/**
 * @brief Parse comment expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_comment(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse type instance.
 *
 * @param parser Parser context to operate on.
 * @param left Name of the type.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_type_inst(
    SnukParser *parser, SnukExpr *left, ParseFlag flag);

/**
 * @brief Parse member access.
 *
 * @param parser Parser context to operate on.
 * @param left type to access field from.
 */
static SnukExpr *parse_member(
    SnukParser *parser, SnukExpr *left, ParseFlag flag);

static ParseRule rules[] = {
    [SNUK_TOKEN_IDENTIFIER] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_INTEGER] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_FLOAT] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_STRING] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_TRUE] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_FALSE] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_NULL] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_NAN] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_INF] = {parse_primary,  NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_LPAREN] = {parse_grouping, parse_call,                PRECEDENCE_PRIMARY    },

    [SNUK_TOKEN_PLUS] = {parse_unary,    parse_binary,              PRECEDENCE_TERM       },
    [SNUK_TOKEN_MINUS] = {parse_unary,    parse_binary,              PRECEDENCE_TERM       },

    [SNUK_TOKEN_STAR] = {NULL,           parse_binary,              PRECEDENCE_FACTOR     },
    [SNUK_TOKEN_SLASH] = {NULL,           parse_binary,              PRECEDENCE_FACTOR     },
    [SNUK_TOKEN_PERCENT] = {NULL,           parse_binary,              PRECEDENCE_FACTOR     },

    [SNUK_TOKEN_BANG] = {parse_unary,    NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_KW_NOT] = {parse_unary,    NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_TILDE] = {parse_unary,    NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_ASSIGN] = {NULL,           parse_assignment,          PRECEDENCE_ASSIGNMENT },

    [SNUK_TOKEN_PLUS_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_MINUS_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_STAR_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_SLASH_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_PERCENT_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_LSHIFT_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_RSHIFT_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_PIPE_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_AMP_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },
    [SNUK_TOKEN_CARET_ASSIGN] =
        {NULL,           parse_compound_assignment, PRECEDENCE_ASSIGNMENT },

    [SNUK_TOKEN_EQUAL] = {NULL,           parse_binary,              PRECEDENCE_EQUALITY   },
    [SNUK_TOKEN_BANG_EQUAL] = {NULL,           parse_binary,              PRECEDENCE_EQUALITY   },

    [SNUK_TOKEN_LESS] = {NULL,           parse_binary,              PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_LESS_EQUAL] = {NULL,           parse_binary,              PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER] = {NULL,           parse_binary,              PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER_EQUAL] = {NULL,           parse_binary,              PRECEDENCE_COMPARISION},

    [SNUK_TOKEN_PIPE] = {NULL,           parse_binary,              PRECEDENCE_OR         },
    [SNUK_TOKEN_CARET] = {NULL,           parse_binary,              PRECEDENCE_XOR        },
    [SNUK_TOKEN_AMP] = {NULL,           parse_binary,              PRECEDENCE_AND        },

    [SNUK_TOKEN_PIPE_PIPE] = {NULL,           parse_binary,              PRECEDENCE_LOGICAL_OR },
    [SNUK_TOKEN_KW_OR] = {NULL,           parse_binary,              PRECEDENCE_LOGICAL_OR },
    [SNUK_TOKEN_AMP_AMP] = {NULL,           parse_binary,              PRECEDENCE_LOGICAL_AND},
    [SNUK_TOKEN_KW_AND] = {NULL,           parse_binary,              PRECEDENCE_LOGICAL_AND},

    [SNUK_TOKEN_LSHIFT] = {NULL,           parse_binary,              PRECEDENCE_SHIFT      },
    [SNUK_TOKEN_RSHIFT] = {NULL,           parse_binary,              PRECEDENCE_SHIFT      },

    [SNUK_TOKEN_IF] = {parse_if,       NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_MATCH] = {parse_match,    NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_WHILE] = {parse_while,    NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_DO] = {parse_while,    NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_FOR] = {parse_for,      NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_FN] = {parse_fn,       NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_TYPE] = {parse_type,     NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_LBRACE] = {parse_block,    parse_type_inst,           PRECEDENCE_PRIMARY    },

    [SNUK_TOKEN_LINE_COMMENT] = {parse_comment,  NULL,                      PRECEDENCE_NONE       },
    [SNUK_TOKEN_BLOCK_COMMENT] = {parse_comment,  NULL,                      PRECEDENCE_NONE       },

    [SNUK_TOKEN_DOT] = {NULL,           parse_member,              PRECEDENCE_PRIMARY    },
};

/**
 * @brief Get the parse rule for a token type.
 *
 * @param type Token type to look up.
 *
 * @return Pointer to the static parse rule for type.
 */
SNUK_FORCE_INLINE ParseRule *get_rule(SnukTokenType type) {
    return &rules[type];
}

SnukExpr *parse_expression(SnukParser *parser, ParseFlag flag) {
    return parse_precedence(parser, PRECEDENCE_ASSIGNMENT, flag);
}

static SnukExpr *parse_precedence(
    SnukParser *parser, Precedence precedence, ParseFlag flag) {
    parser_advance(parser);
    prefix_fn pfn = get_rule(parser->previous.type)->pfn;
    if (!pfn) {
        parser_error(parser, "expected expression");
        return NULL;
    }

    SnukExpr *left = pfn(parser, flag);

    while (precedence <= get_rule(parser->current.type)->precedence) {
        if (flag == PARSE_FLAG_STOP_LBRACE
            && parser->current.type == SNUK_TOKEN_LBRACE)
            return left;

        parser_advance(parser);
        infix_fn ifn = get_rule(parser->previous.type)->ifn;
        left = ifn(parser, left, flag);
    }

    return left;
}

static SnukExpr *parse_primary(SnukParser *parser, ParseFlag flag) {
    SNUK_UNUSED(flag);
    SnukToken t = parser->previous;
    switch (t.type) {
        case SNUK_TOKEN_IDENTIFIER:
            return build_identifier_expr(parser);
        case SNUK_TOKEN_INTEGER:
            return build_int_literal_expr(parser);
        case SNUK_TOKEN_FLOAT:
            return build_float_literal_expr(parser);
        case SNUK_TOKEN_STRING:
            return build_string_literal_expr(parser);
        case SNUK_TOKEN_TRUE:
        case SNUK_TOKEN_FALSE:
            return build_bool_expr(parser);
        case SNUK_TOKEN_NULL:
            return build_null_expr(parser);
        case SNUK_TOKEN_NAN:
            // TODO:
        case SNUK_TOKEN_INF:
            // TODO:
            break;
        default:
            // TODO:
            parser_error(parser, "unexpected expression");
            break;
    }

    return NULL;
}

static SnukExpr *parse_grouping(SnukParser *parser, ParseFlag flag) {
    SnukExpr *expr = parse_expression(parser, flag);
    parser_expect(parser, SNUK_TOKEN_RPAREN, "expected ')'");
    return expr;
}

static SnukExpr *parse_unary(SnukParser *parser, ParseFlag flag) {
    SnukToken op = parser->previous;
    SnukExpr *right = parse_precedence(parser, PRECEDENCE_UNARY, flag);
    return build_unary_expr(parser, op.type, right);
}

static SnukExpr *parse_binary(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    SnukToken op = parser->previous;
    ParseRule *rule = get_rule(op.type);
    SnukExpr *right = parse_precedence(parser, rule->precedence + 1, flag);
    return build_binary_expr(parser, op.type, left, right);
}

static SnukExpr *parse_assignment(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    SnukExpr *value = parse_precedence(parser, PRECEDENCE_ASSIGNMENT, flag);
    return build_assign_expr(parser, left, value);
}

static SnukExpr *parse_compound_assignment(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    if (left->type != SNUK_EXPR_IDENTIFIER) {
        parser_error(parser, "invalid assignment target");
        return NULL;
    }
    SnukTokenType op = parser->previous.type;
    SnukExpr *value = parse_precedence(parser, PRECEDENCE_ASSIGNMENT, flag);
    return build_compound_assign_expr(parser, op, left, value);
}

static SnukExpr *parse_if(SnukParser *parser, ParseFlag flag) {
    SnukExpr *condition = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    SnukExpr *then_block = parse_block(parser, flag);
    SnukExpr *else_block = NULL;
    if (parser_match(parser, SNUK_TOKEN_ELSE)) {
        if (parser_match(parser, SNUK_TOKEN_IF))
            else_block = parse_if(parser, flag);
        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
        else_block = parse_block(parser, flag);
    }
    return build_if_expr(parser, condition, then_block, else_block);
}

static SnukExpr *parse_match(SnukParser *parser, ParseFlag flag) {
    SNUK_UNUSED(flag);
    // TODO:
    return build_match_expr(parser, NULL);
}

static SnukExpr *parse_while(SnukParser *parser, ParseFlag flag) {
    SnukExpr *condition = NULL;
    SnukExpr *body = NULL;

    if (parser->previous.type == SNUK_TOKEN_DO) {
        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
        body = parse_block(parser, flag);
        parser_expect(parser, SNUK_TOKEN_WHILE, "expected while");
        condition = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);
        return build_while_expr(parser, condition, body, true);
    }

    condition = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    body = parse_block(parser, flag);
    return build_while_expr(parser, condition, body, false);
}

static SnukExpr *parse_for(SnukParser *parser, ParseFlag flag) {
    // Case 1: for { ... }
    if (parser_match(parser, SNUK_TOKEN_LBRACE))
        return build_for_expr(
            parser, NULL, NULL, NULL, parse_block(parser, flag));

    SnukItem *init = NULL;
    SnukExpr *condition = NULL;
    SnukExpr *update = NULL;
    SnukExpr *body = NULL;

    // Case 2: for var ... → must be C-style
    if (parser_check(parser, SNUK_TOKEN_VAR)) {
        init = parse_item(parser, flag);

        if (!parser_check(parser, SNUK_TOKEN_SEMICOLON))
            condition = parse_expression(parser, flag);
        parser_expect(
            parser, SNUK_TOKEN_SEMICOLON, "expected ';' after condition");

        if (!parser_check(parser, SNUK_TOKEN_LBRACE))
            update = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);

        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");
        body = parse_block(parser, flag);

        return build_for_expr(parser, init, condition, update, body);
    }

    // Case 3: for ; ... → C-style with no init
    if (parser_match(parser, SNUK_TOKEN_SEMICOLON)) {
        // condition (optional)
        // No need to have PARSE_FLAG_STOP_LBRACE since we are expecting ';'
        // after condition
        if (!parser_check(parser, SNUK_TOKEN_SEMICOLON))
            condition = parse_expression(parser, flag);
        parser_expect(
            parser, SNUK_TOKEN_SEMICOLON, "expected ';' after condition");

        // update (optional)
        if (!parser_check(parser, SNUK_TOKEN_LBRACE))
            update = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);

        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");
        body = parse_block(parser, flag);

        return build_for_expr(parser, NULL, condition, update, body);
    }

    // Otherwise parse an expression first
    // Well, we are preventing user from having type inst if it is init position
    // of for loop
    SnukExpr *first = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);

    // Case 4: for condition { ... }
    if (parser_check(parser, SNUK_TOKEN_LBRACE)) {
        condition = first;

        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");
        body = parse_block(parser, flag);

        return build_for_expr(parser, NULL, condition, NULL, body);
    }

    // Case 5: must be C-style → first is init
    parser_expect(
        parser, SNUK_TOKEN_SEMICOLON,
        "expected ';' or '{' after for expression");

    init = build_expr_item(parser, first);

    if (!parser_check(parser, SNUK_TOKEN_SEMICOLON))
        condition = parse_expression(parser, flag);
    parser_expect(parser, SNUK_TOKEN_SEMICOLON, "expected ';' after condition");

    if (!parser_check(parser, SNUK_TOKEN_LBRACE))
        update = parse_expression(parser, PARSE_FLAG_STOP_LBRACE);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");
    body = parse_block(parser, flag);

    return build_for_expr(parser, init, condition, update, body);
}

static SnukExpr *parse_fn(SnukParser *parser, ParseFlag flag) {
    SnukStringView name = {0};
    if (parser_match(parser, SNUK_TOKEN_IDENTIFIER))
        name = parser->previous.string_literal;

    SnukParam **params = snuk_darray_create(SnukParam *, parser->allocator);
    parser_expect(parser, SNUK_TOKEN_LPAREN, "expected '('");
    while (!parser_match(parser, SNUK_TOKEN_RPAREN)
           && parser->current.type != SNUK_TOKEN_EOF) {
        parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected parameter name");

        SnukStringView name = parser->previous.string_literal;
        SnukExpr *default_value = NULL;
        SnukType *type = NULL;

        if (parser_match(parser, SNUK_TOKEN_COLON))
            type = parse_type_annot(parser, flag);
        else type = build_any_type(parser);

        if (parser_match(parser, SNUK_TOKEN_ASSIGN))
            default_value = parse_expression(parser, flag);

        snuk_darray_push(
            &params, build_param(parser, name, type, default_value));

        if (!parser_check(parser, SNUK_TOKEN_RPAREN))
            parser_expect(parser, SNUK_TOKEN_COMMA, "expected comma");
    }

    if (parser->previous.type != SNUK_TOKEN_RPAREN) {
        parser_error(parser, "expected ')'");
        return NULL;
    }

    SnukType *ret_type = NULL;
    if (parser_match(parser, SNUK_TOKEN_ARROW))
        ret_type = parse_type_annot(parser, flag);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of function");
    SnukExpr *body = parse_block(parser, flag);

    return build_fn_expr(parser, params, body, ret_type, name);
}

static SnukExpr *parse_call(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    SnukExpr **params = snuk_darray_create(SnukExpr *, parser->allocator);
    while (!parser_match(parser, SNUK_TOKEN_RPAREN)
           && parser->current.type != SNUK_TOKEN_EOF) {
        SnukExpr *expr = parse_expression(parser, flag);
        snuk_darray_push(&params, expr);
        if (!parser_check(parser, SNUK_TOKEN_RPAREN))
            parser_expect(parser, SNUK_TOKEN_COMMA, "expected comma");
    }

    if (parser->previous.type != SNUK_TOKEN_RPAREN) {
        parser_error(parser, "expected ')");
        return NULL;
    }
    return build_call_expr(parser, left, params);
}

static SnukExpr *parse_member(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    SnukExpr *field = parse_expression(parser, flag);
    return build_member_access_expr(parser, left, field);
}

static SnukExpr *parse_comment(SnukParser *parser, ParseFlag flag) {
    SNUK_UNUSED(flag);
    SnukToken t = parser->previous;
    return build_comment_expr(parser, t);
}

static SnukExpr *parse_type(SnukParser *parser, ParseFlag flag) {
    SNUK_UNUSED(flag);
    SnukStringView name = {0};
    if (parser_match(parser, SNUK_TOKEN_IDENTIFIER))
        name = parser->previous.string_literal;

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");

    SnukItem **members = snuk_darray_create(SnukItem *, parser->allocator);

    while (!parser_match(parser, SNUK_TOKEN_RBRACE)
           && parser->current.type != SNUK_TOKEN_EOF) {
        if (parser_check(parser, SNUK_TOKEN_VAR)
            || parser_check(parser, SNUK_TOKEN_CONST)
            || parser_check(parser, SNUK_TOKEN_FN)
            || parser_check(parser, SNUK_TOKEN_TYPE)) {
            snuk_darray_push(&members, parse_item(parser, flag));
        } else {
            parser_error(parser, "unexpected token");
        }
    }

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "expected '}'");
        return NULL;
    }

    return build_type_expr(parser, members, name);
}

static SnukExpr *parse_type_inst(
    SnukParser *parser, SnukExpr *left, ParseFlag flag) {
    SnukExpr **init = snuk_darray_create(SnukExpr *, parser->allocator);
    while (!parser_match(parser, SNUK_TOKEN_RBRACE)
           && parser->current.type != SNUK_TOKEN_EOF) {
        parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected an member name");
        SnukExpr *identifier = parse_primary(parser, flag);
        parser_expect(parser, SNUK_TOKEN_COLON, "expected ':'");
        SnukExpr *value = parse_expression(parser, flag);
        parser_expect_item_end(parser);
        SnukExpr *assign = build_assign_expr(parser, identifier, value);
        snuk_darray_push(&init, assign);
    }

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "expected '}'");
        return NULL;
    }

    return build_type_inst_expr(parser, left, init);
}

static SnukExpr *parse_block(SnukParser *parser, ParseFlag flag) {
    SNUK_UNUSED(flag);
    SnukExpr *block_expr = build_block_expr(parser, NULL, NULL);

    while (!parser_match(parser, SNUK_TOKEN_RBRACE)
           && parser->current.type != SNUK_TOKEN_EOF)
        block_expr =
            build_block_expr(parser, block_expr, parse_item(parser, flag));

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "block was not closed");
        return NULL;
    }

    return block_expr;
}

const char *snuk_parser_expr_type_to_string(SnukExprType type) {
    switch (type) {
        case SNUK_EXPR_IDENTIFIER:
            return SNUK_STRINGIFY(SNUK_EXPR_IDENTIFIER);
        case SNUK_EXPR_INT:
            return SNUK_STRINGIFY(SNUK_EXPR_INT);
        case SNUK_EXPR_FLOAT:
            return SNUK_STRINGIFY(SNUK_EXPR_FLOAT);
        case SNUK_EXPR_STRING:
            return SNUK_STRINGIFY(SNUK_EXPR_STRING);
        case SNUK_EXPR_BOOL:
            return SNUK_STRINGIFY(SNUK_EXPR_BOOL);
        case SNUK_EXPR_NULL:
            return SNUK_STRINGIFY(SNUK_EXPR_NULL);
        case SNUK_EXPR_UNARY:
            return SNUK_STRINGIFY(SNUK_EXPR_UNARY);
        case SNUK_EXPR_BINARY:
            return SNUK_STRINGIFY(SNUK_EXPR_BINARY);
        case SNUK_EXPR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_EXPR_ASSIGN);
        case SNUK_EXPR_COMPOUND_ASSIGN:
            return SNUK_STRINGIFY(SNUK_EXPR_COMPOUND_ASSIGN);
        case SNUK_EXPR_IF:
            return SNUK_STRINGIFY(SNUK_EXPR_IF);
        case SNUK_EXPR_MATCH:
            return SNUK_STRINGIFY(SNUK_EXPR_MATCH);
        case SNUK_EXPR_WHILE:
            return SNUK_STRINGIFY(SNUK_EXPR_WHILE);
        case SNUK_EXPR_DO_WHILE:
            return SNUK_STRINGIFY(SNUK_EXPR_DO_WHILE);
        case SNUK_EXPR_FOR:
            return SNUK_STRINGIFY(SNUK_EXPR_FOR);
        case SNUK_EXPR_FN:
            return SNUK_STRINGIFY(SNUK_EXPR_FN);
        case SNUK_EXPR_TYPE:
            return SNUK_STRINGIFY(SNUK_EXPR_TYPE);
        case SNUK_EXPR_BLOCK:
            return SNUK_STRINGIFY(SNUK_EXPR_BLOCK);
        case SNUK_EXPR_CALL:
            return SNUK_STRINGIFY(SNUK_EXPR_CALL);
        case SNUK_EXPR_MEMBER:
            return SNUK_STRINGIFY(SNUK_EXPR_MEMBER);
        case SNUK_EXPR_INDEX:
            return SNUK_STRINGIFY(SNUK_EXPR_INDEX);
        case SNUK_EXPR_LINE_COMMENT:
            return SNUK_STRINGIFY(SNUK_EXPR_LINE_COMMENT);
        case SNUK_EXPR_BLOCK_COMMENT:
            return SNUK_STRINGIFY(SNUK_EXPR_BLOCK_COMMENT);
        case SNUK_EXPR_MAX:
            return SNUK_STRINGIFY(SNUK_EXPR_MAX);
        default:
            return "Unknown expression type";
    }
}

void snuk_parser_log_expr(SnukExpr *expr) {
    if (!expr) return;
    log_trace(
        "Expression type: %s", snuk_parser_expr_type_to_string(expr->type));

    uint64_t count;
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            log_trace(
                "Identifier: " SNUK_STRING_VIEW_FORMAT,
                SNUK_STRING_VIEW_ARG(expr->identifier));
            break;
        case SNUK_EXPR_INT:
            log_trace("Integer: %ld", expr->int_literal);
            break;
        case SNUK_EXPR_FLOAT:
            log_trace("Float: %lf", expr->float_literal);
            break;
        case SNUK_EXPR_STRING:
            log_trace(
                "String: " SNUK_STRING_VIEW_FORMAT,
                SNUK_STRING_VIEW_ARG(expr->string_literal));
            break;
        case SNUK_EXPR_BOOL:
            log_trace("Bool: %s", expr->bool_literal ? "true" : "false");
            break;
        case SNUK_EXPR_NULL:
            log_trace("Null", NULL);
            break;
        case SNUK_EXPR_UNARY:
            log_trace("Unary:", NULL);
            log_trace("%s", snuk_lexer_token_type_to_string(expr->unary.op));
            snuk_parser_log_expr(expr->unary.operand);
            break;
        case SNUK_EXPR_BINARY:
            log_trace("Binary:", NULL);
            snuk_parser_log_expr(expr->binary.left);
            log_trace("%s", snuk_lexer_token_type_to_string(expr->binary.op));
            snuk_parser_log_expr(expr->binary.right);
            break;
        case SNUK_EXPR_ASSIGN:
            snuk_parser_log_expr(expr->assign.identifier);
            snuk_parser_log_expr(expr->assign.value);
            break;
        case SNUK_EXPR_COMPOUND_ASSIGN:
            snuk_parser_log_expr(expr->compound_assign.identifier);
            log_trace(
                "%s",
                snuk_lexer_token_type_to_string(expr->compound_assign.op));
            snuk_parser_log_expr(expr->compound_assign.value);
            break;
        case SNUK_EXPR_IF:
            log_trace("if:", NULL);
            snuk_parser_log_expr(expr->if_else.condition);
            log_trace("then:", NULL);
            snuk_parser_log_expr(expr->if_else.then_block);
            log_trace("else:", NULL);
            snuk_parser_log_expr(expr->if_else.else_block);
            break;
        case SNUK_EXPR_MATCH:
            // TODO:
            log_trace("match expression:", NULL);
            break;
        case SNUK_EXPR_WHILE:
            log_trace("while:", NULL);
            snuk_parser_log_expr(expr->while_loop.condition);
            log_trace("run:", NULL);
            snuk_parser_log_expr(expr->while_loop.body);
            break;
        case SNUK_EXPR_DO_WHILE:
            log_trace("do:", NULL);
            snuk_parser_log_expr(expr->while_loop.body);
            log_trace("while:", NULL);
            snuk_parser_log_expr(expr->while_loop.condition);
            break;
        case SNUK_EXPR_FOR:
            log_trace("for:", NULL);
            snuk_parser_log_item(expr->for_loop.init);
            snuk_parser_log_expr(expr->for_loop.condition);
            snuk_parser_log_expr(expr->for_loop.update);
            log_trace("run:", NULL);
            snuk_parser_log_expr(expr->for_loop.body);
            break;
        case SNUK_EXPR_FN:
            log_trace("fn expression:", NULL);
            if (expr->fn_expr.name.len)
                log_trace(
                    "Name: " SNUK_STRING_VIEW_FORMAT,
                    SNUK_STRING_VIEW_ARG(expr->fn_expr.name));
            count = snuk_darray_get_length(expr->fn_expr.params);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_param(expr->fn_expr.params[i]);
            log_trace("body:", NULL);
            snuk_parser_log_expr(expr->fn_expr.body);
            snuk_parser_log_type(expr->fn_expr.return_type);
            break;
        case SNUK_EXPR_TYPE:
            log_trace("type expression:", NULL);
            if (expr->type_expr.name.len)
                log_trace(
                    "Name: " SNUK_STRING_VIEW_FORMAT,
                    SNUK_STRING_VIEW_ARG(expr->type_expr.name));
            count = snuk_darray_get_length(expr->type_expr.members);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_item(expr->type_expr.members[i]);
            break;
        case SNUK_EXPR_BLOCK:
            log_trace("block expression:", NULL);
            count = snuk_darray_get_length(expr->block_items);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_item(expr->block_items[i]);
            break;
        case SNUK_EXPR_CALL:
            // TODO:
            snuk_parser_log_expr(expr->call.fn);
            count = snuk_darray_get_length(expr->call.params);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_expr(expr->call.params[i]);
            break;
        case SNUK_EXPR_MEMBER:
            // TODO:
            log_trace("Member:", NULL);
            break;
        case SNUK_EXPR_INDEX:
            // TODO:
            log_trace("Index:", NULL);
            break;
        case SNUK_EXPR_LINE_COMMENT:
            log_trace(
                "single line comment: " SNUK_STRING_VIEW_FORMAT,
                SNUK_STRING_VIEW_ARG(expr->comment));
            break;
        case SNUK_EXPR_BLOCK_COMMENT:
            log_trace(
                "multi-line comment: " SNUK_STRING_VIEW_FORMAT,
                SNUK_STRING_VIEW_ARG(expr->comment));
            break;
        default:
            break;
    }
}
