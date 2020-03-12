#include <stdlib.h> /* NULL strtod() */
#include <assert.h> /* assert */
#include "leptjson.h"

#define EXPECT(c, ch) do { assert(*(c->json) == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
} lept_context;

static int lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
        p++;
    }
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char * literal, lept_type type) {
    int i;
    EXPECT(c, literal[0]);
    const char *p = c->json;
    for (i = 0; literal[i+1]; i++)
        if (p[i] != literal[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context *c, lept_value *v) {
    EXPECT(c, 'n');
    const char *p = c->json;
    if (p[0] != 'u' || p[1] != 'l' || p[2] != 'l') { /* 注意这里的短路计算，不会导致越界 */
        return LEPT_PARSE_INVALID_VALUE; 
    }
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context *c, lept_value *v) {
    EXPECT(c, 't');
    const char *p = c->json;
    if (p[0] != 'r' || p[1] != 'u' || p[2] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context *c, lept_value *v) {
    EXPECT(c, 'f');
    const char *p = c->json;
    if (p[0] != 'a' || p[1] != 'l' || p[2] != 's' || p[3] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

lept_type lept_get_type(lept_value *v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
