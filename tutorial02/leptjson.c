#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <ctype.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* org){
    const char* tmp = org;
    while(*tmp && *c->json){
        if(*c->json++ != *tmp++){
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    if(*tmp){
        return LEPT_PARSE_INVALID_VALUE;
    }
    if(org[0] == 't') v->type = LEPT_TRUE;
    if(org[0] == 'f') v->type = LEPT_FALSE;
    if(org[0] == 'n') v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    if (c->json[0] == '+' || c->json[0] == '.' ||
        c->json[0] == 'N' || c->json[0] == 'n' || 
        c->json[0] == 'I'|| c->json[0] == 'i'){
        return LEPT_PARSE_INVALID_VALUE;
    }

    const char* tmp = c->json;
    while(*tmp && *tmp != '.') tmp++;
    if(*tmp == '.' && !isdigit(*(tmp+1))){
        return LEPT_PARSE_INVALID_VALUE;
    }

    tmp = c->json;
    if(*tmp == '0' && !(*(tmp+1) == '\0' || *(tmp+1) == '.')){
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }

    /* \TODO validate number */

    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    if(!isfinite(v->n)){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true");
        case 'f':  return lept_parse_literal(c, v, "false");
        case 'n':  return lept_parse_literal(c, v, "null");
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
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
