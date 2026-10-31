#include "browser_js.h"

namespace {

static const int MAX_NODES = 384;
static const int MAX_VALUES = 384;
static const int MAX_STRINGS = 1024;
static const int MAX_OBJECTS = 64;
static const int MAX_FUNCS = 64;
static const int MAX_ELEMS = 32;
static const int MAX_SCOPES = 32;
static const int MAX_PARAMS = 8;
static const int STR_POOL = 4096;

enum NodeKind {
    N_PROGRAM, N_VAR, N_FN, N_CALL, N_IF, N_BLOCK, N_RETURN,
    N_EXPR, N_ASSIGN, N_BIN, N_UN, N_MEMBER, N_ID, N_LIT, N_CALL_ARGS
};

enum Vtype { V_UNDEFINED, V_NULL, V_NUMBER, V_STRING, V_BOOLEAN, V_OBJECT, V_FUNCTION };

struct Node {
    uint8_t kind;
    int a, b, c;
    char name[64];
    double num;
    int str;
    int native;
    uint8_t used;
    char params[MAX_PARAMS][32];
    uint8_t param_count;
};

struct Value {
    uint8_t type;
    double num;
    int str;
    int obj;
    int func;
    uint8_t boolean;
};

struct ObjProp { char name[32]; Value val; uint8_t used; };
struct JsObject { ObjProp props[16]; };

struct JsFunc { int body; int param_count; char params[MAX_PARAMS][32]; uint8_t native; int native_id; };

struct Binding { char name[32]; int value_index; uint8_t used; };
struct Scope { Binding bindings[24]; int count; };

struct ElemReg { char tag[16]; char id[64]; int page_index; int object_index; };

static Node g_nodes[MAX_NODES];
static int g_node_count = 0;
static Value g_values[MAX_VALUES];
static int g_value_count = 0;
static char g_str_pool[STR_POOL];
static int g_str_pool_len = 0;
static JsObject g_objects[MAX_OBJECTS];
static int g_object_count = 0;
static JsFunc g_funcs[MAX_FUNCS];
static int g_func_count = 0;
static ElemReg g_elems[MAX_ELEMS];
static int g_elem_count = 0;
static Scope g_scopes[MAX_SCOPES];
static int g_scope_depth = 0;
static int g_current_func = -1;

static const char *g_src = 0;
static int g_src_len = 0;
static int g_pos = 0;

enum Tok {
    TOK_EOF, TOK_NUMBER, TOK_STRING, TOK_IDENT,
    TOK_TRUE, TOK_FALSE, TOK_NULL,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACK, TOK_RBRACK, TOK_SEMICOLON, TOK_COMMA, TOK_DOT,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_EQ, TOK_NE, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_VAR, TOK_LET, TOK_CONST, TOK_FUNCTION, TOK_IF, TOK_ELSE, TOK_RETURN
};

static int tok_type = TOK_EOF;
static double tok_num = 0;
static int tok_str = -1;
static char tok_name[64] = {0};

static int str_len(const char *s) { int n = 0; while (s && s[n]) n++; return n; }
static int str_eq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static int __attribute__((unused)) str_ncase(const char *a, const char *b) {
    while (*a && *b) { char x = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a; char y = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b; if (x != y) return 0; a++; b++; }
    return *a == *b;
}

static int intern_string(const char *s, int n) {
    if (g_str_pool_len + n + 1 > STR_POOL) return -1;
    for (int i = 0; i < n; i++) g_str_pool[g_str_pool_len++] = s[i];
    g_str_pool[g_str_pool_len++] = 0;
    return g_str_pool_len - n - 1;
}
static const char *get_string(int idx) { return idx >= 0 ? g_str_pool + idx : ""; }

static Value make_undefined() { Value v; v.type = V_UNDEFINED; v.num = 0; v.str = -1; v.obj = -1; v.func = -1; v.boolean = 0; return v; }
static Value make_null() { Value v; v.type = V_NULL; v.num = 0; v.str = -1; v.obj = -1; v.func = -1; v.boolean = 0; return v; }
static Value make_number(double n) { Value v; v.type = V_NUMBER; v.num = n; v.str = -1; v.obj = -1; v.func = -1; v.boolean = 0; return v; }
static Value make_string(int s) { Value v; v.type = V_STRING; v.num = 0; v.str = s; v.obj = -1; v.func = -1; v.boolean = 0; return v; }
static Value make_boolean(int b) { Value v; v.type = V_BOOLEAN; v.num = 0; v.str = -1; v.obj = -1; v.func = -1; v.boolean = b ? 1 : 0; return v; }
static Value make_object(int o) { Value v; v.type = V_OBJECT; v.num = 0; v.str = -1; v.obj = o; v.func = -1; v.boolean = 0; return v; }
static Value make_function(int f) { Value v; v.type = V_FUNCTION; v.num = 0; v.str = -1; v.obj = -1; v.func = f; v.boolean = 0; return v; }

static int new_node() { if (g_node_count >= MAX_NODES) return -1; g_nodes[g_node_count].used = 1; return g_node_count++; }
static int new_value(Value v) { if (g_value_count >= MAX_VALUES) return -1; g_values[g_value_count] = v; return g_value_count++; }
static int new_object() { if (g_object_count >= MAX_OBJECTS) return -1; for (int i = 0; i < 16; i++) g_objects[g_object_count].props[i].used = 0; return g_object_count++; }
static int new_func() { if (g_func_count >= MAX_FUNCS) return -1; g_funcs[g_func_count].body = -1; g_funcs[g_func_count].param_count = 0; g_funcs[g_func_count].native = 0; g_funcs[g_func_count].native_id = -1; for (int i = 0; i < MAX_PARAMS; i++) g_funcs[g_func_count].params[i][0] = 0; return g_func_count++; }

static void obj_set(int o, const char *name, Value v) {
    JsObject *obj = &g_objects[o];
    for (int i = 0; i < 16; i++) {
        if (obj->props[i].used && str_eq(obj->props[i].name, name)) { obj->props[i].val = v; return; }
    }
    for (int i = 0; i < 16; i++) {
        if (!obj->props[i].used) { int n = str_len(name); if (n > 31) n = 31; for (int j = 0; j < n; j++) obj->props[i].name[j] = name[j]; obj->props[i].name[n] = 0; obj->props[i].val = v; obj->props[i].used = 1; return; }
    }
}
static Value obj_get(int o, const char *name) {
    JsObject *obj = &g_objects[o];
    for (int i = 0; i < 16; i++) if (obj->props[i].used && str_eq(obj->props[i].name, name)) return obj->props[i].val;
    return make_undefined();
}

static int scope_push() { if (g_scope_depth >= MAX_SCOPES) return -1; g_scopes[g_scope_depth].count = 0; return g_scope_depth++; }
static void scope_pop() { if (g_scope_depth > 0) g_scope_depth--; }
static int scope_find(int depth, const char *name) {
    while (depth >= 0) {
        Scope *s = &g_scopes[depth];
        for (int i = 0; i < s->count; i++) if (s->bindings[i].used && str_eq(s->bindings[i].name, name)) return s->bindings[i].value_index;
        depth--;
    }
    return -1;
}
static Value scope_get(const char *name) {
    int vi = scope_find(g_scope_depth - 1, name);
    if (vi < 0) return make_undefined();
    return g_values[vi];
}
static void scope_set(const char *name, Value v) {
    int vi = scope_find(g_scope_depth - 1, name);
    if (vi >= 0) { g_values[vi] = v; return; }
    Scope *s = &g_scopes[g_scope_depth - 1];
    if (s->count >= 24) return;
    int n = str_len(name); if (n > 31) n = 31;
    for (int i = 0; i < n; i++) s->bindings[s->count].name[i] = name[i];
    s->bindings[s->count].name[n] = 0;
    s->bindings[s->count].value_index = new_value(v);
    s->bindings[s->count].used = 1;
    s->count++;
}

static int peek_char() { return g_pos < g_src_len ? (unsigned char)g_src[g_pos] : -1; }
static int next_char() { return g_pos < g_src_len ? (unsigned char)g_src[g_pos++] : -1; }

static int is_keyword(const char *name) {
    if (str_eq(name, "var")) return TOK_VAR;
    if (str_eq(name, "let")) return TOK_LET;
    if (str_eq(name, "const")) return TOK_CONST;
    if (str_eq(name, "function")) return TOK_FUNCTION;
    if (str_eq(name, "if")) return TOK_IF;
    if (str_eq(name, "else")) return TOK_ELSE;
    if (str_eq(name, "return")) return TOK_RETURN;
    if (str_eq(name, "true")) return TOK_TRUE;
    if (str_eq(name, "false")) return TOK_FALSE;
    if (str_eq(name, "null")) return TOK_NULL;
    return 0;
}

static void next_token() {
    tok_type = TOK_EOF; tok_num = 0; tok_str = -1; tok_name[0] = 0;
    int c;
    do { c = peek_char(); if (c < 0) { tok_type = TOK_EOF; return; } if (c == ' ' || c == '\t' || c == '\n' || c == '\r') next_char(); else break; } while (1);
    c = peek_char();
    if (c < 0) { tok_type = TOK_EOF; return; }
    if (c >= '0' && c <= '9') {
        int n = 0; double val = 0;
        while (peek_char() >= '0' && peek_char() <= '9') { val = val * 10 + (next_char() - '0'); n++; }
        if (peek_char() == '.') { next_char(); double f = 0.1; while (peek_char() >= '0' && peek_char() <= '9') { val += (next_char() - '0') * f; f *= 0.1; } }
        tok_type = TOK_NUMBER; tok_num = val; return;
    }
    if (c == '"' || c == '\'') {
        char q = (char)next_char();
        char buf[128]; int bn = 0;
        while (peek_char() >= 0 && peek_char() != q) {
            char ch = (char)next_char();
            if (ch == '\\' && peek_char() >= 0) { char n2 = (char)next_char(); if (n2 == 'n') ch = '\n'; else if (n2 == '"') ch = '"'; else if (n2 == '\\') ch = '\\'; else ch = n2; }
            if (bn < 127) buf[bn++] = ch;
        }
        if (peek_char() >= 0) next_char();
        buf[bn] = 0;
        tok_type = TOK_STRING; tok_str = intern_string(buf, bn); return;
    }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$') {
        int n = 0;
        while ((peek_char() >= 'a' && peek_char() <= 'z') || (peek_char() >= 'A' && peek_char() <= 'Z') || (peek_char() >= '0' && peek_char() <= '9') || peek_char() == '_' || peek_char() == '$') {
            if (n < 63) tok_name[n++] = (char)next_char();
            else next_char();
        }
        tok_name[n] = 0;
        int kw = is_keyword(tok_name);
        tok_type = kw ? kw : TOK_IDENT;
        return;
    }
    next_char();
    if (c == '(') { tok_type = TOK_LPAREN; return; }
    if (c == ')') { tok_type = TOK_RPAREN; return; }
    if (c == '{') { tok_type = TOK_LBRACE; return; }
    if (c == '}') { tok_type = TOK_RBRACE; return; }
    if (c == '[') { tok_type = TOK_LBRACK; return; }
    if (c == ']') { tok_type = TOK_RBRACK; return; }
    if (c == ';') { tok_type = TOK_SEMICOLON; return; }
    if (c == ',') { tok_type = TOK_COMMA; return; }
    if (c == '.') { tok_type = TOK_DOT; return; }
    if (c == '=') { if (peek_char() == '=') { next_char(); tok_type = TOK_EQ; } else tok_type = TOK_ASSIGN; return; }
    if (c == '!') { if (peek_char() == '=') { next_char(); tok_type = TOK_NE; } else tok_type = TOK_NOT; return; }
    if (c == '<') { if (peek_char() == '=') { next_char(); tok_type = TOK_LE; } else tok_type = TOK_LT; return; }
    if (c == '>') { if (peek_char() == '=') { next_char(); tok_type = TOK_GE; } else tok_type = TOK_GT; return; }
    if (c == '+') { tok_type = TOK_PLUS; return; }
    if (c == '-') { tok_type = TOK_MINUS; return; }
    if (c == '*') { tok_type = TOK_STAR; return; }
    if (c == '/') { tok_type = TOK_SLASH; return; }
    if (c == '&') { if (peek_char() == '&') { next_char(); tok_type = TOK_AND; } return; }
    if (c == '|') { if (peek_char() == '|') { next_char(); tok_type = TOK_OR; } return; }
    tok_type = TOK_EOF;
}

static int parse_expression(int prec);
static int parse_statement();
static int parse_unary();
static int parse_primary() {
    if (tok_type == TOK_NUMBER) { int n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].num = tok_num; return n; }
    if (tok_type == TOK_STRING) { int n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].str = tok_str; return n; }
    if (tok_type == TOK_TRUE) { int n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].num = 1; return n; }
    if (tok_type == TOK_FALSE) { int n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].num = 0; return n; }
    if (tok_type == TOK_NULL) { int n = new_node(); g_nodes[n].kind = N_LIT; return n; }
    if (tok_type == TOK_IDENT) {
        char name[64]; for (int i = 0; tok_name[i] && i < 63; i++) name[i] = tok_name[i]; name[63] = 0;
        next_token();
        if (tok_type == TOK_LPAREN) {
            next_token();
            int args = -1, last = -1;
            if (tok_type != TOK_RPAREN) {
                args = new_node(); g_nodes[args].kind = N_CALL_ARGS; last = args;
                while (1) {
                    int e = parse_expression(0);
                    g_nodes[last].a = e;
                    if (tok_type == TOK_COMMA) { next_token(); int na = new_node(); g_nodes[na].kind = N_CALL_ARGS; g_nodes[last].b = na; last = na; }
                    else break;
                }
            }
            next_token();
            int n = new_node(); g_nodes[n].kind = N_CALL; for (int i = 0; name[i] && i < 63; i++) g_nodes[n].name[i] = name[i]; g_nodes[n].name[63] = 0; g_nodes[n].a = -1; g_nodes[n].b = args;
            return n;
        }
        if (tok_type == TOK_DOT) {
            next_token();
            if (tok_type == TOK_IDENT) {
                char mname[64]; for (int i = 0; tok_name[i] && i < 63; i++) mname[i] = tok_name[i]; mname[63] = 0; next_token();
                int obj = new_node(); g_nodes[obj].kind = N_ID; for (int i = 0; name[i] && i < 63; i++) g_nodes[obj].name[i] = name[i]; g_nodes[obj].name[63] = 0;
                int n = new_node(); g_nodes[n].kind = N_MEMBER; g_nodes[n].a = obj; for (int i = 0; mname[i] && i < 63; i++) g_nodes[n].name[i] = mname[i]; g_nodes[n].name[63] = 0;
                if (tok_type == TOK_LPAREN) {
                    next_token();
                    int args = -1, last = -1;
                    if (tok_type != TOK_RPAREN) {
                        args = new_node(); g_nodes[args].kind = N_CALL_ARGS; last = args;
                        while (1) {
                            int e = parse_expression(0);
                            g_nodes[last].a = e;
                            if (tok_type == TOK_COMMA) { next_token(); int na = new_node(); g_nodes[na].kind = N_CALL_ARGS; g_nodes[last].b = na; last = na; }
                            else break;
                        }
                    }
                    next_token();
                    int c = new_node(); g_nodes[c].kind = N_CALL; g_nodes[c].a = n; g_nodes[c].b = args; return c;
                }
                return n;
            }
        }
        int n = new_node(); g_nodes[n].kind = N_ID; for (int i = 0; name[i] && i < 63; i++) g_nodes[n].name[i] = name[i]; g_nodes[n].name[63] = 0;
        return n;
    }
    if (tok_type == TOK_LPAREN) { next_token(); int e = parse_expression(0); next_token(); return e; }
    if (tok_type == TOK_FUNCTION) {
        next_token();
        int n = new_node(); g_nodes[n].kind = N_FN;
        if (tok_type == TOK_IDENT) { for (int i = 0; tok_name[i] && i < 63; i++) g_nodes[n].name[i] = tok_name[i]; g_nodes[n].name[63] = 0; next_token(); }
        next_token();
        int pc = 0;
        if (tok_type != TOK_RPAREN) {
            while (1) {
                if (tok_type == TOK_IDENT && pc < MAX_PARAMS) { for (int i = 0; tok_name[i] && i < 31; i++) g_nodes[n].params[pc][i] = tok_name[i]; g_nodes[n].params[pc][31] = 0; pc++; }
                next_token();
                if (tok_type == TOK_COMMA) { next_token(); continue; }
                break;
            }
        }
        next_token();
        g_nodes[n].a = -1;
        g_nodes[n].b = pc; g_nodes[n].param_count = pc;
        int body = new_node(); g_nodes[body].kind = N_BLOCK;
        int last = body;
        while (tok_type != TOK_RBRACE && tok_type != TOK_EOF) {
            int s = parse_statement();
            if (last == body) g_nodes[body].a = s;
            else g_nodes[last].b = s;
            last = s;
        }
        next_token();
        g_nodes[n].c = body;
        return n;
    }
    next_token();
    int n = new_node(); g_nodes[n].kind = N_LIT; return n;
}

static int parse_unary() {
    if (tok_type == TOK_PLUS) { next_token(); int e = parse_unary(); return e; }
    if (tok_type == TOK_MINUS) { next_token(); int e = parse_unary(); int n = new_node(); g_nodes[n].kind = N_UN; g_nodes[n].a = e; g_nodes[n].b = 1; return n; }
    if (tok_type == TOK_NOT) { next_token(); int e = parse_unary(); int n = new_node(); g_nodes[n].kind = N_UN; g_nodes[n].a = e; g_nodes[n].b = 2; return n; }
    return parse_primary();
}

static int bin_prec(int t) {
    switch (t) {
        case TOK_OR: return 1;
        case TOK_AND: return 2;
        case TOK_EQ: case TOK_NE: return 3;
        case TOK_LT: case TOK_GT: case TOK_LE: case TOK_GE: return 4;
        case TOK_PLUS: case TOK_MINUS: return 5;
        case TOK_STAR: case TOK_SLASH: return 6;
    }
    return 0;
}

static int parse_expression(int prec) {
    int left = parse_unary();
    while (1) {
        int p = bin_prec(tok_type);
        if (p <= prec) break;
        int op = tok_type; next_token();
        int right = parse_expression(p);
        int n = new_node(); g_nodes[n].kind = N_BIN; g_nodes[n].a = left; g_nodes[n].b = op; g_nodes[n].c = right;
        left = n;
    }
    return left;
}

static int parse_statement() {
    if (tok_type == TOK_VAR || tok_type == TOK_LET || tok_type == TOK_CONST) {
        int kind = tok_type; next_token();
        int n = new_node(); g_nodes[n].kind = N_VAR; for (int i = 0; tok_name[i] && i < 63; i++) g_nodes[n].name[i] = tok_name[i]; g_nodes[n].name[63] = 0;
        next_token();
        g_nodes[n].a = -1; g_nodes[n].b = kind;
        if (tok_type == TOK_ASSIGN) { next_token(); g_nodes[n].a = parse_expression(0); }
        next_token();
        return n;
    }
    if (tok_type == TOK_FUNCTION) {
        next_token();
        int n = new_node(); g_nodes[n].kind = N_VAR;
        if (tok_type == TOK_IDENT) { for (int i = 0; tok_name[i] && i < 63; i++) g_nodes[n].name[i] = tok_name[i]; g_nodes[n].name[63] = 0; next_token(); }
        g_nodes[n].b = 0;
        int fn = new_node(); g_nodes[fn].kind = N_FN;
        next_token();
        int pc = 0;
        if (tok_type != TOK_RPAREN) {
            while (1) {
                if (tok_type == TOK_IDENT && pc < MAX_PARAMS) { for (int i = 0; tok_name[i] && i < 31; i++) g_nodes[fn].params[pc][i] = tok_name[i]; g_nodes[fn].params[pc][31] = 0; pc++; }
                next_token();
                if (tok_type == TOK_COMMA) { next_token(); continue; }
                break;
            }
        }
        next_token();
        g_nodes[fn].a = -1; g_nodes[fn].b = pc; g_nodes[fn].param_count = pc;
        int body = new_node(); g_nodes[body].kind = N_BLOCK;
        int last = body;
        while (tok_type != TOK_RBRACE && tok_type != TOK_EOF) {
            int s = parse_statement();
            if (last == body) g_nodes[body].a = s;
            else g_nodes[last].b = s;
            last = s;
        }
        next_token();
        g_nodes[fn].c = body;
        g_nodes[n].a = fn;
        next_token();
        return n;
    }
    if (tok_type == TOK_IF) {
        next_token(); next_token();
        int cond = parse_expression(0);
        next_token();
        int tb = parse_statement();
        int f = -1;
        if (tok_type == TOK_ELSE) { next_token(); f = parse_statement(); }
        int n = new_node(); g_nodes[n].kind = N_IF; g_nodes[n].a = cond; g_nodes[n].b = tb; g_nodes[n].c = f;
        return n;
    }
    if (tok_type == TOK_LBRACE) {
        next_token();
        int n = new_node(); g_nodes[n].kind = N_BLOCK;
        int last = n;
        while (tok_type != TOK_RBRACE && tok_type != TOK_EOF) {
            int s = parse_statement();
            if (last == n) g_nodes[n].a = s;
            else g_nodes[last].b = s;
            last = s;
        }
        next_token();
        return n;
    }
    if (tok_type == TOK_RETURN) {
        next_token();
        int n = new_node(); g_nodes[n].kind = N_RETURN;
        g_nodes[n].a = (tok_type == TOK_SEMICOLON) ? -1 : parse_expression(0);
        next_token();
        return n;
    }
    int e = parse_expression(0);
    int n = new_node(); g_nodes[n].kind = N_EXPR;
    if (tok_type == TOK_ASSIGN) {
        int n2 = new_node(); g_nodes[n2].kind = N_ASSIGN;
        g_nodes[n2].a = e;
        next_token();
        g_nodes[n2].b = parse_expression(0);
        next_token();
        return n2;
    }
    g_nodes[n].a = e;
    next_token();
    return n;
}

static int parse_program() {
    int n = new_node(); g_nodes[n].kind = N_PROGRAM;
    int last = n;
    while (tok_type != TOK_EOF) {
        int s = parse_statement();
        if (last == n) g_nodes[n].a = s;
        else g_nodes[last].b = s;
        last = s;
    }
    return n;
}

static double to_number(Value v) {
    if (v.type == V_NUMBER) return v.num;
    if (v.type == V_BOOLEAN) return v.boolean ? 1.0 : 0.0;
    if (v.type == V_STRING) {
        const char *s = get_string(v.str);
        double val = 0; int i = 0; while (s[i] && (s[i]==' '||s[i]=='\t')) i++;
        int neg = 0; if (s[i]=='-'){neg=1;i++;} else if (s[i]=='+'){i++;}
        while (s[i] >= '0' && s[i] <= '9') { val = val*10 + (s[i]-'0'); i++; }
        return neg ? -val : val;
    }
    return 0;
}
static int to_bool(Value v) {
    if (v.type == V_UNDEFINED || v.type == V_NULL) return 0;
    if (v.type == V_NUMBER) return v.num != 0;
    if (v.type == V_BOOLEAN) return v.boolean;
    if (v.type == V_STRING) return get_string(v.str)[0] != 0;
    return 1;
}
static int strict_eq(Value a, Value b) {
    if (a.type != b.type) return 0;
    if (a.type == V_NUMBER) return a.num == b.num;
    if (a.type == V_STRING) return str_eq(get_string(a.str), get_string(b.str));
    if (a.type == V_BOOLEAN) return a.boolean == b.boolean;
    if (a.type == V_NULL || a.type == V_UNDEFINED) return 1;
    return a.obj == b.obj || a.func == b.func;
}

static Value eval_bin(int op, Value l, Value r) {
    double ln = to_number(l), rn = to_number(r);
    switch (op) {
        case TOK_PLUS:
            if (l.type == V_STRING || r.type == V_STRING) {
                char buf[256]; int bn = 0;
                const char *ls = l.type == V_STRING ? get_string(l.str) : 0;
                const char *rs = r.type == V_STRING ? get_string(r.str) : 0;
                if (ls) { while (*ls && bn < 255) buf[bn++] = *ls++; }
                else { char t[32]; int tn = 0; double dv = ln; if (dv < 0) { buf[bn++]='-'; dv=-dv; } if (dv==0) buf[bn++]='0'; while (dv > 0 && tn < 31) { t[tn++] = (char)('0' + (int)dv % 10); dv /= 10; } while (tn > 0) buf[bn++] = t[--tn]; }
                if (rs) { while (*rs && bn < 255) buf[bn++] = *rs++; }
                else { char t[32]; int tn = 0; double dv = rn; if (dv < 0) { buf[bn++]='-'; dv=-dv; } if (dv==0) buf[bn++]='0'; while (dv > 0 && tn < 31) { t[tn++] = (char)('0' + (int)dv % 10); dv /= 10; } while (tn > 0) buf[bn++] = t[--tn]; }
                buf[bn] = 0;
                return make_string(intern_string(buf, bn));
            }
            return make_number(ln + rn);
        case TOK_MINUS: return make_number(ln - rn);
        case TOK_STAR: return make_number(ln * rn);
        case TOK_SLASH: return rn == 0 ? make_number(0) : make_number(ln / rn);
        case TOK_EQ: return make_boolean(strict_eq(l, r));
        case TOK_NE: return make_boolean(!strict_eq(l, r));
        case TOK_LT: return make_boolean(ln < rn);
        case TOK_GT: return make_boolean(ln > rn);
        case TOK_LE: return make_boolean(ln <= rn);
        case TOK_GE: return make_boolean(ln >= rn);
        case TOK_AND: return make_boolean(to_bool(l) && to_bool(r));
        case TOK_OR: return make_boolean(to_bool(l) || to_bool(r));
    }
    return make_undefined();
}

static Value eval_node(int node_idx, Value *out_result);
static Value eval_call_node(int node_idx, Value this_val);
static Value js_native_call(int native_id, Value this_val, Value *args, int nac);

static Value eval_statement(int node_idx) {
    Node *n = &g_nodes[node_idx];
    switch (n->kind) {
        case N_BLOCK: {
            int cur = n->a;
            Value last = make_undefined();
            while (cur >= 0) { last = eval_node(cur, &last); cur = g_nodes[cur].b; }
            return last;
        }
        case N_VAR: {
            Value v = n->a >= 0 ? eval_node(n->a, 0) : make_undefined();
            scope_set(n->name, v);
            return v;
        }
        case N_IF: {
            Value c = eval_node(n->a, 0);
            if (to_bool(c) && n->b >= 0) return eval_node(n->b, 0);
            if (!to_bool(c) && n->c >= 0) return eval_node(n->c, 0);
            return make_undefined();
        }
        case N_RETURN: {
            Value v = n->a >= 0 ? eval_node(n->a, 0) : make_undefined();
            return v;
        }
        case N_EXPR: {
            return eval_node(n->a, 0);
        }
        case N_ASSIGN: {
            Value v = eval_node(n->b, 0);
            Node *t = &g_nodes[n->a];
            if (t->kind == N_ID) { scope_set(t->name, v); return v; }
            if (t->kind == N_MEMBER) {
                Value ov = eval_node(t->a, 0);
                if (ov.type == V_OBJECT) obj_set(ov.obj, t->name, v);
                return v;
            }
            return v;
        }
        default: return eval_node(n->a, 0);
    }
}

static Value eval_node(int node_idx, Value *result) {
    if (node_idx < 0) return make_undefined();
    Node *n = &g_nodes[node_idx];
    switch (n->kind) {
        case N_PROGRAM: {
            int cur = n->a; Value last = make_undefined();
            while (cur >= 0) { last = eval_statement(cur); cur = g_nodes[cur].b; }
            if (result) *result = last;
            return last;
        }
        case N_LIT: {
            Value v;
            if (n->str >= 0) v = make_string(n->str);
            else v = make_number(n->num);
            if (result) *result = v;
            return v;
        }
        case N_ID: {
            Value v = scope_get(n->name);
            if (result) *result = v;
            return v;
        }
        case N_BIN: {
            Value l = eval_node(n->a, 0);
            Value r = eval_node(n->c, 0);
            Value v = eval_bin(n->b, l, r);
            if (result) *result = v;
            return v;
        }
        case N_UN: {
            Value v = eval_node(n->a, 0);
            Value r;
            if (n->b == 1) r = make_number(-to_number(v));
            else r = make_boolean(!to_bool(v));
            if (result) *result = r;
            return r;
        }
        case N_MEMBER: {
            Value ov = eval_node(n->a, 0);
            Value r;
            if (ov.type == V_OBJECT) r = obj_get(ov.obj, n->name);
            else r = make_undefined();
            if (result) *result = r;
            return r;
        }
        case N_CALL: {
            Value v = eval_call_node(node_idx, make_undefined());
            if (result) *result = v;
            return v;
        }
        case N_FN: {
            int f = new_func();
            g_funcs[f].body = n->c;
            g_funcs[f].param_count = n->param_count;
            for (int i = 0; i < n->param_count && i < MAX_PARAMS; i++)
                for (int j = 0; j < 31 && n->params[i][j]; j++)
                    g_funcs[f].params[i][j] = n->params[i][j];
            Value v = make_function(f);
            if (result) *result = v;
            return v;
        }
        case N_VAR:
        case N_IF:
        case N_BLOCK:
        case N_RETURN:
        case N_EXPR:
        case N_ASSIGN: {
            Value v = eval_statement(node_idx);
            if (result) *result = v;
            return v;
        }
        default: return make_undefined();
    }
}

static Value eval_call_node(int node_idx, Value this_val) {
    Node *n = &g_nodes[node_idx];
    Value callee = eval_node(n->a, 0);
    Value args[8]; int nac = 0;
    int ca = n->b;
    while (ca >= 0 && nac < 8) {
        args[nac++] = eval_node(g_nodes[ca].a, 0);
        ca = g_nodes[ca].b;
    }
    if (callee.type == V_FUNCTION) {
        int f = callee.func;
        if (g_funcs[f].native) {
            return js_native_call(g_funcs[f].native_id, this_val, args, nac);
        }
        scope_push();
        for (int i = 0; i < g_funcs[f].param_count && i < nac; i++) {
            Scope *s = &g_scopes[g_scope_depth - 1];
            int name_len = str_len(g_funcs[f].params[i]);
            if (name_len > 31) name_len = 31;
            for (int j = 0; j < name_len; j++) s->bindings[s->count].name[j] = g_funcs[f].params[i][j];
            s->bindings[s->count].name[name_len] = 0;
            s->bindings[s->count].value_index = new_value(args[i]);
            s->bindings[s->count].used = 1;
            s->count++;
        }
        int old = g_current_func; g_current_func = f;
        Value ret = make_undefined();
        int body = g_funcs[f].body;
        if (body >= 0) {
            int cur = g_nodes[body].a;
            while (cur >= 0) {
                Value r = eval_statement(cur);
                Node *cn = &g_nodes[cur];
                if (cn->kind == N_RETURN) { ret = r; break; }
                cur = g_nodes[cur].b;
            }
        }
        g_current_func = old;
        scope_pop();
        return ret;
    }
    return make_undefined();
}

static int __attribute__((unused)) js_native_id_from_name(const char *name) {
    if (str_eq(name, "alert")) return 0;
    if (str_eq(name, "log")) return 1;
    if (str_eq(name, "getElementById")) return 2;
    if (str_eq(name, "querySelector")) return 3;
    if (str_eq(name, "addEventListener")) return 4;
    return -1;
}

static Value js_native_call(int native_id, Value this_val, Value *args, int nac) {
    switch (native_id) {
        case 0: {
            const char *msg = nac > 0 && args[0].type == V_STRING ? get_string(args[0].str) : "";
            js_host_alert(msg);
            return make_undefined();
        }
        case 1: {
            const char *msg = nac > 0 && args[0].type == V_STRING ? get_string(args[0].str) : "";
            js_host_log(msg);
            return make_undefined();
        }
        case 2: {
            const char *id = nac > 0 && args[0].type == V_STRING ? get_string(args[0].str) : "";
            for (int i = 0; i < g_elem_count; i++) if (str_eq(g_elems[i].id, id)) return make_object(g_elems[i].object_index);
            return make_null();
        }
        case 3: {
            return make_null();
        }
        case 4: {
            if (this_val.type == V_OBJECT && nac >= 2 && args[1].type == V_FUNCTION) {
                obj_set(this_val.obj, "clickHandler", make_function(args[1].func));
            }
            return make_undefined();
        }
    }
    return make_undefined();
}

}

void js_init(void) {
    g_node_count = 0; g_value_count = 0; g_str_pool_len = 0;
    g_object_count = 0; g_func_count = 0; g_elem_count = 0; g_scope_depth = 0;
    g_current_func = -1;
    g_pos = 0;
    for (int i = 0; i < MAX_NODES; i++) g_nodes[i].used = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) for (int j = 0; j < 16; j++) g_objects[i].props[j].used = 0;

    scope_push();
    int glob = new_object();
    int doc = new_object();
    obj_set(doc, "getElementById", make_function(new_func()));
    g_funcs[g_func_count - 1].native = 1; g_funcs[g_func_count - 1].native_id = 2;
    obj_set(doc, "querySelector", make_function(new_func()));
    g_funcs[g_func_count - 1].native = 1; g_funcs[g_func_count - 1].native_id = 3;
    obj_set(glob, "document", make_object(doc));
    int al = new_func(); g_funcs[al].native = 1; g_funcs[al].native_id = 0;
    obj_set(glob, "alert", make_function(al));
    int con = new_object();
    int lg = new_func(); g_funcs[lg].native = 1; g_funcs[lg].native_id = 1;
    obj_set(con, "log", make_function(lg));
    obj_set(glob, "console", make_object(con));
    scope_set("window", make_object(glob));
    scope_set("document", make_object(doc));
}

int js_run(const char *source, int length) {
    g_src = source; g_src_len = length; g_pos = 0;
    next_token();
    int program = parse_program();
    Value r = eval_node(program, 0);
    (void)r;
    return 1;
}

void js_register_element(const char *tag, const char *id, int page_index) {
    if (id[0] == 0) return;
    for (int i = 0; i < g_elem_count; i++) if (str_eq(g_elems[i].id, id)) return;
    if (g_elem_count >= MAX_ELEMS) return;
    ElemReg *e = &g_elems[g_elem_count++];
    int n = str_len(tag); if (n > 15) n = 15; for (int i = 0; i < n; i++) e->tag[i] = tag[i]; e->tag[n] = 0;
    n = str_len(id); if (n > 63) n = 63; for (int i = 0; i < n; i++) e->id[i] = id[i]; e->id[n] = 0;
    e->page_index = page_index;
    int o = new_object();
    obj_set(o, "tag", make_string(intern_string(tag, str_len(tag))));
    obj_set(o, "id", make_string(intern_string(id, str_len(id))));
    obj_set(o, "clickHandler", make_number(-1));
    obj_set(o, "innerText", make_string(intern_string("", 0)));
    int st = new_object();
    obj_set(st, "color", make_string(intern_string("", 0)));
    obj_set(o, "style", make_object(st));
    int ae = new_func(); g_funcs[ae].native = 1; g_funcs[ae].native_id = 4;
    obj_set(o, "addEventListener", make_function(ae));
    e->object_index = o;
}

int js_find_element_by_id(const char *id, char *tag_out, int tag_cap, int *page_index_out) {
    for (int i = 0; i < g_elem_count; i++) {
        if (str_eq(g_elems[i].id, id)) {
            if (tag_out) { int n = str_len(g_elems[i].tag); if (n > tag_cap - 1) n = tag_cap - 1; for (int j = 0; j < n; j++) tag_out[j] = g_elems[i].tag[j]; tag_out[n] = 0; }
            if (page_index_out) *page_index_out = g_elems[i].page_index;
            return i;
        }
    }
    return -1;
}

void js_dispatch_click(const char *element_id) {
    for (int i = 0; i < g_elem_count; i++) {
        if (!str_eq(g_elems[i].id, element_id)) continue;
        int o = g_elems[i].object_index;
        Value h = obj_get(o, "clickHandler");
        if (h.type == V_FUNCTION && h.func >= 0) {
            int old = g_current_func; g_current_func = h.func;
            scope_push();
            Value ret = make_undefined();
            int body = g_funcs[h.func].body;
            if (body >= 0) {
                int cur = g_nodes[body].a;
                while (cur >= 0) {
                    Value rr = eval_statement(cur);
                    if (g_nodes[cur].kind == N_RETURN) { ret = rr; break; }
                    cur = g_nodes[cur].b;
                }
            }
            scope_pop();
            g_current_func = old;
        }
        return;
    }
}
