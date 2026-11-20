/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/apps/browser_js.h>

namespace {

static const int MAX_NODES = 384;
static const int MAX_VALUES = 512;
static const int MAX_STRINGS = 1024;
static const int MAX_OBJECTS = 64;
static const int MAX_FUNCS = 64;
static const int MAX_ELEMS = 32;
static const int MAX_SCOPES = 32;
static const int MAX_PARAMS = 8;
static const int STR_POOL = 4096;
static const int STEP_LIMIT = 200000;
static const int DEPTH_LIMIT = 64;

enum NodeKind {
    N_PROGRAM, N_VAR, N_FN, N_CALL, N_IF, N_BLOCK, N_RETURN,
    N_EXPR, N_ASSIGN, N_BIN, N_UN, N_MEMBER, N_ID, N_LIT, N_CALL_ARGS
};

enum Vtype { V_UNDEFINED, V_NULL, V_NUMBER, V_STRING, V_BOOLEAN, V_OBJECT, V_FUNCTION };

struct Node {
    uint8_t kind;
    int a, b, c;
    int next;               /* kolejny statement w bloku / programie */
    char name[64];
    double num;
    int str;
    uint8_t param_count;
    char params[MAX_PARAMS][32];
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

static int g_overflow = 0;
static int g_abort = 0;
static int g_steps = 0;
static int g_depth = 0;
static int g_returning = 0;
static Value g_ret;

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
    TOK_VAR, TOK_LET, TOK_CONST, TOK_FUNCTION, TOK_IF, TOK_ELSE, TOK_RETURN,
    TOK_OTHER
};

static int tok_type = TOK_EOF;
static double tok_num = 0;
static int tok_str = -1;
static char tok_name[64] = {0};

static int str_len(const char *s) { int n = 0; while (s && s[n]) n++; return n; }
static int str_eq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static void copy_str(char *dst, int cap, const char *src) {
    int n = 0;
    while (src[n] && n < cap - 1) { dst[n] = src[n]; n++; }
    dst[n] = 0;
}

static int intern_string(const char *s, int n) {
    if (g_str_pool_len + n + 1 > STR_POOL) return -1;
    for (int i = 0; i < n; i++) g_str_pool[g_str_pool_len++] = s[i];
    g_str_pool[g_str_pool_len++] = 0;
    return g_str_pool_len - n - 1;
}
static const char *get_string(int idx) { return idx >= 0 ? g_str_pool + idx : ""; }

static Value make_undefined() { Value v; v.type = V_UNDEFINED; v.num = 0; v.str = -1; v.obj = -1; v.func = -1; v.boolean = 0; return v; }
static Value make_null() { Value v = make_undefined(); v.type = V_NULL; return v; }
static Value make_number(double n) { Value v = make_undefined(); v.type = V_NUMBER; v.num = n; return v; }
static Value make_string(int s) { Value v = make_undefined(); v.type = V_STRING; v.str = s; return v; }
static Value make_boolean(int b) { Value v = make_undefined(); v.type = V_BOOLEAN; v.boolean = b ? 1 : 0; return v; }
static Value make_object(int o) { Value v = make_undefined(); v.type = V_OBJECT; v.obj = o; return v; }
static Value make_function(int f) { Value v = make_undefined(); v.type = V_FUNCTION; v.func = f; return v; }

/* Zawsze zwraca poprawny indeks i w pelni zainicjowany wezel. */
static int new_node() {
    int i;
    if (g_node_count >= MAX_NODES) { g_overflow = 1; i = MAX_NODES - 1; }
    else i = g_node_count++;
    Node *n = &g_nodes[i];
    n->kind = N_LIT;
    n->a = n->b = n->c = n->next = -1;
    n->name[0] = 0;
    n->num = 0;
    n->str = -1;
    n->param_count = 0;
    return i;
}
static int new_value(Value v) { if (g_value_count >= MAX_VALUES) return -1; g_values[g_value_count] = v; return g_value_count++; }
static int new_object() {
    if (g_object_count >= MAX_OBJECTS) return -1;
    for (int i = 0; i < 16; i++) g_objects[g_object_count].props[i].used = 0;
    return g_object_count++;
}
static int new_func() {
    if (g_func_count >= MAX_FUNCS) return -1;
    JsFunc *f = &g_funcs[g_func_count];
    f->body = -1; f->param_count = 0; f->native = 0; f->native_id = -1;
    for (int i = 0; i < MAX_PARAMS; i++) f->params[i][0] = 0;
    return g_func_count++;
}
static Value make_native(int id) {
    int f = new_func();
    if (f < 0) return make_undefined();
    g_funcs[f].native = 1; g_funcs[f].native_id = id;
    return make_function(f);
}

static void obj_set(int o, const char *name, Value v) {
    if (o < 0 || o >= g_object_count) return;
    JsObject *obj = &g_objects[o];
    for (int i = 0; i < 16; i++)
        if (obj->props[i].used && str_eq(obj->props[i].name, name)) { obj->props[i].val = v; return; }
    for (int i = 0; i < 16; i++) {
        if (!obj->props[i].used) {
            copy_str(obj->props[i].name, 32, name);
            obj->props[i].val = v; obj->props[i].used = 1; return;
        }
    }
}
static Value obj_get(int o, const char *name) {
    if (o < 0 || o >= g_object_count) return make_undefined();
    JsObject *obj = &g_objects[o];
    for (int i = 0; i < 16; i++)
        if (obj->props[i].used && str_eq(obj->props[i].name, name)) return obj->props[i].val;
    return make_undefined();
}

static int scope_push() { if (g_scope_depth >= MAX_SCOPES) return -1; g_scopes[g_scope_depth].count = 0; return g_scope_depth++; }
static void scope_pop() { if (g_scope_depth > 1) g_scope_depth--; }
static int scope_find(int depth, const char *name) {
    while (depth >= 0) {
        Scope *s = &g_scopes[depth];
        for (int i = 0; i < s->count; i++)
            if (s->bindings[i].used && str_eq(s->bindings[i].name, name)) return s->bindings[i].value_index;
        depth--;
    }
    return -1;
}
static Value scope_get(const char *name) {
    int vi = scope_find(g_scope_depth - 1, name);
    if (vi < 0) return make_undefined();
    return g_values[vi];
}
static void scope_bind(const char *name, Value v) {
    if (g_scope_depth <= 0) return;
    Scope *s = &g_scopes[g_scope_depth - 1];
    if (s->count >= 24) return;
    copy_str(s->bindings[s->count].name, 32, name);
    s->bindings[s->count].value_index = new_value(v);
    s->bindings[s->count].used = 1;
    s->count++;
}
static void scope_set(const char *name, Value v) {
    int vi = scope_find(g_scope_depth - 1, name);
    if (vi >= 0) { g_values[vi] = v; return; }
    scope_bind(name, v);
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

static void skip_ws() {
    while (1) {
        int c = peek_char();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { next_char(); continue; }
        if (c == '/' && g_pos + 1 < g_src_len) {
            char d = g_src[g_pos + 1];
            if (d == '/') { while (peek_char() >= 0 && peek_char() != '\n') next_char(); continue; }
            if (d == '*') {
                g_pos += 2;
                while (g_pos < g_src_len && !(g_src[g_pos] == '*' && g_pos + 1 < g_src_len && g_src[g_pos + 1] == '/')) g_pos++;
                if (g_pos < g_src_len) g_pos += 2;
                continue;
            }
        }
        break;
    }
}

static void next_token() {
    tok_type = TOK_EOF; tok_num = 0; tok_str = -1; tok_name[0] = 0;
    skip_ws();
    int c = peek_char();
    if (c < 0) { tok_type = TOK_EOF; return; }
    if (c >= '0' && c <= '9') {
        double val = 0;
        while (peek_char() >= '0' && peek_char() <= '9') val = val * 10 + (next_char() - '0');
        if (peek_char() == '.') {
            next_char();
            double f = 0.1;
            while (peek_char() >= '0' && peek_char() <= '9') { val += (next_char() - '0') * f; f *= 0.1; }
        }
        tok_type = TOK_NUMBER; tok_num = val; return;
    }
    if (c == '"' || c == '\'') {
        char q = (char)next_char();
        char buf[128]; int bn = 0;
        while (peek_char() >= 0 && peek_char() != q) {
            char ch = (char)next_char();
            if (ch == '\\' && peek_char() >= 0) {
                char n2 = (char)next_char();
                if (n2 == 'n') ch = '\n'; else if (n2 == 't') ch = '\t'; else ch = n2;
            }
            if (bn < 127) buf[bn++] = ch;
        }
        if (peek_char() >= 0) next_char();
        buf[bn] = 0;
        tok_type = TOK_STRING; tok_str = intern_string(buf, bn); return;
    }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$') {
        int n = 0;
        while ((peek_char() >= 'a' && peek_char() <= 'z') || (peek_char() >= 'A' && peek_char() <= 'Z') ||
               (peek_char() >= '0' && peek_char() <= '9') || peek_char() == '_' || peek_char() == '$') {
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
    if (c == '=') { if (peek_char() == '=') { next_char(); if (peek_char() == '=') next_char(); tok_type = TOK_EQ; } else tok_type = TOK_ASSIGN; return; }
    if (c == '!') { if (peek_char() == '=') { next_char(); if (peek_char() == '=') next_char(); tok_type = TOK_NE; } else tok_type = TOK_NOT; return; }
    if (c == '<') { if (peek_char() == '=') { next_char(); tok_type = TOK_LE; } else tok_type = TOK_LT; return; }
    if (c == '>') { if (peek_char() == '=') { next_char(); tok_type = TOK_GE; } else tok_type = TOK_GT; return; }
    if (c == '+') { tok_type = TOK_PLUS; return; }
    if (c == '-') { tok_type = TOK_MINUS; return; }
    if (c == '*') { tok_type = TOK_STAR; return; }
    if (c == '/') { tok_type = TOK_SLASH; return; }
    if (c == '&') { if (peek_char() == '&') { next_char(); tok_type = TOK_AND; } else tok_type = TOK_OTHER; return; }
    if (c == '|') { if (peek_char() == '|') { next_char(); tok_type = TOK_OR; } else tok_type = TOK_OTHER; return; }
    tok_type = TOK_OTHER;
}

static int parse_expression(int prec);
static int parse_statement();
static int parse_block();

static void skip_semi() { if (tok_type == TOK_SEMICOLON) next_token(); }

/* tok_type == pierwszy token po '(' */
static int parse_call_args() {
    int args = -1, last = -1;
    if (tok_type != TOK_RPAREN && tok_type != TOK_EOF) {
        while (1) {
            int cell = new_node();
            g_nodes[cell].kind = N_CALL_ARGS;
            if (args < 0) args = cell; else if (last != cell) g_nodes[last].b = cell;
            last = cell;
            g_nodes[cell].a = parse_expression(0);
            if (tok_type == TOK_COMMA) next_token(); else break;
        }
    }
    if (tok_type == TOK_RPAREN) next_token();
    return args;
}

/* tok_type == TOK_FUNCTION */
static int parse_function() {
    next_token();
    int fn = new_node();
    g_nodes[fn].kind = N_FN;
    if (tok_type == TOK_IDENT) { copy_str(g_nodes[fn].name, 64, tok_name); next_token(); }
    if (tok_type == TOK_LPAREN) next_token();
    int pc = 0;
    while (tok_type != TOK_RPAREN && tok_type != TOK_EOF) {
        if (tok_type == TOK_IDENT && pc < MAX_PARAMS) { copy_str(g_nodes[fn].params[pc], 32, tok_name); pc++; }
        next_token();
    }
    if (tok_type == TOK_RPAREN) next_token();
    g_nodes[fn].param_count = (uint8_t)pc;
    g_nodes[fn].c = parse_block();
    return fn;
}

static int parse_primary() {
    int n;
    if (tok_type == TOK_NUMBER) { n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].num = tok_num; next_token(); return n; }
    if (tok_type == TOK_STRING) { n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].str = tok_str; next_token(); return n; }
    if (tok_type == TOK_TRUE)   { n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].c = 1; g_nodes[n].num = 1; next_token(); return n; }
    if (tok_type == TOK_FALSE)  { n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].c = 1; g_nodes[n].num = 0; next_token(); return n; }
    if (tok_type == TOK_NULL)   { n = new_node(); g_nodes[n].kind = N_LIT; g_nodes[n].c = 2; next_token(); return n; }
    if (tok_type == TOK_IDENT) {
        int cur = new_node();
        g_nodes[cur].kind = N_ID;
        copy_str(g_nodes[cur].name, 64, tok_name);
        next_token();
        while (1) {
            if (tok_type == TOK_DOT) {
                next_token();
                if (tok_type != TOK_IDENT) break;
                int m = new_node();
                g_nodes[m].kind = N_MEMBER; g_nodes[m].a = cur;
                copy_str(g_nodes[m].name, 64, tok_name);
                next_token();
                cur = m;
            } else if (tok_type == TOK_LPAREN) {
                next_token();
                int args = parse_call_args();
                int c = new_node();
                g_nodes[c].kind = N_CALL; g_nodes[c].a = cur; g_nodes[c].b = args;
                cur = c;
            } else break;
        }
        return cur;
    }
    if (tok_type == TOK_LPAREN) {
        next_token();
        int e = parse_expression(0);
        if (tok_type == TOK_RPAREN) next_token();
        if (tok_type == TOK_LPAREN) {            /* (function(){...})(args) */
            next_token();
            int args = parse_call_args();
            int c = new_node();
            g_nodes[c].kind = N_CALL; g_nodes[c].a = e; g_nodes[c].b = args;
            return c;
        }
        return e;
    }
    if (tok_type == TOK_FUNCTION) return parse_function();
    next_token();                                 /* nieznany token: zawsze przesuwamy sie dalej */
    n = new_node();
    g_nodes[n].kind = N_LIT;
    g_nodes[n].c = 3;                             /* undefined */
    return n;
}

static int parse_unary() {
    if (tok_type == TOK_PLUS) { next_token(); return parse_unary(); }
    if (tok_type == TOK_MINUS) { next_token(); int e = parse_unary(); int n = new_node(); g_nodes[n].kind = N_UN; g_nodes[n].a = e; g_nodes[n].b = 1; return n; }
    if (tok_type == TOK_NOT)   { next_token(); int e = parse_unary(); int n = new_node(); g_nodes[n].kind = N_UN; g_nodes[n].a = e; g_nodes[n].b = 2; return n; }
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
        int n = new_node();
        g_nodes[n].kind = N_BIN; g_nodes[n].a = left; g_nodes[n].b = op; g_nodes[n].c = right;
        left = n;
    }
    return left;
}

static int parse_block() {
    int body = new_node();
    g_nodes[body].kind = N_BLOCK;
    if (tok_type == TOK_LBRACE) next_token();
    int last = -1;
    while (tok_type != TOK_RBRACE && tok_type != TOK_EOF) {
        int s = parse_statement();
        if (last < 0) g_nodes[body].a = s;
        else if (last != s) g_nodes[last].next = s;
        last = s;
    }
    if (tok_type == TOK_RBRACE) next_token();
    return body;
}

static int parse_statement() {
    if (tok_type == TOK_VAR || tok_type == TOK_LET || tok_type == TOK_CONST) {
        next_token();
        int n = new_node();
        g_nodes[n].kind = N_VAR;
        if (tok_type == TOK_IDENT) { copy_str(g_nodes[n].name, 64, tok_name); next_token(); }
        if (tok_type == TOK_ASSIGN) { next_token(); g_nodes[n].a = parse_expression(0); }
        skip_semi();
        return n;
    }
    if (tok_type == TOK_FUNCTION) {
        int fn = parse_function();
        int n = new_node();
        g_nodes[n].kind = N_VAR;
        copy_str(g_nodes[n].name, 64, g_nodes[fn].name);
        g_nodes[n].a = fn;
        skip_semi();
        return n;
    }
    if (tok_type == TOK_IF) {
        next_token();
        if (tok_type == TOK_LPAREN) next_token();
        int cond = parse_expression(0);
        if (tok_type == TOK_RPAREN) next_token();
        int tb = parse_statement();
        int f = -1;
        if (tok_type == TOK_ELSE) { next_token(); f = parse_statement(); }
        int n = new_node();
        g_nodes[n].kind = N_IF; g_nodes[n].a = cond; g_nodes[n].b = tb; g_nodes[n].c = f;
        return n;
    }
    if (tok_type == TOK_LBRACE) return parse_block();
    if (tok_type == TOK_RETURN) {
        next_token();
        int n = new_node();
        g_nodes[n].kind = N_RETURN;
        if (tok_type != TOK_SEMICOLON && tok_type != TOK_RBRACE && tok_type != TOK_EOF)
            g_nodes[n].a = parse_expression(0);
        skip_semi();
        return n;
    }
    int e = parse_expression(0);
    int n = new_node();
    if (tok_type == TOK_ASSIGN) {
        g_nodes[n].kind = N_ASSIGN;
        g_nodes[n].a = e;
        next_token();
        g_nodes[n].b = parse_expression(0);
    } else {
        g_nodes[n].kind = N_EXPR;
        g_nodes[n].a = e;
    }
    skip_semi();
    return n;
}

static int parse_program() {
    int n = new_node();
    g_nodes[n].kind = N_PROGRAM;
    int last = -1;
    while (tok_type != TOK_EOF) {
        int s = parse_statement();
        if (last < 0) g_nodes[n].a = s;
        else if (last != s) g_nodes[last].next = s;
        last = s;
    }
    return n;
}

static double to_number(Value v) {
    if (v.type == V_NUMBER) return v.num;
    if (v.type == V_BOOLEAN) return v.boolean ? 1.0 : 0.0;
    if (v.type == V_STRING) {
        const char *s = get_string(v.str);
        double val = 0; int i = 0;
        while (s[i] == ' ' || s[i] == '\t') i++;
        int neg = 0;
        if (s[i] == '-') { neg = 1; i++; } else if (s[i] == '+') i++;
        while (s[i] >= '0' && s[i] <= '9') { val = val * 10 + (s[i] - '0'); i++; }
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
    if (a.type == V_OBJECT) return a.obj == b.obj;
    return a.func == b.func;
}

static void append_text(char *buf, int *bn, const char *s) {
    while (*s && *bn < 255) buf[(*bn)++] = *s++;
}
static void append_value(char *buf, int *bn, Value v) {
    if (v.type == V_STRING) { append_text(buf, bn, get_string(v.str)); return; }
    if (v.type == V_BOOLEAN) { append_text(buf, bn, v.boolean ? "true" : "false"); return; }
    if (v.type == V_NULL) { append_text(buf, bn, "null"); return; }
    if (v.type == V_NUMBER) {
        double d = v.num;
        if (d > 2000000000.0) d = 2000000000.0;
        if (d < -2000000000.0) d = -2000000000.0;
        int iv = (int)d;
        char t[16]; int tn = 0;
        if (iv < 0) { if (*bn < 255) buf[(*bn)++] = '-'; iv = -iv; }
        if (iv == 0) t[tn++] = '0';
        while (iv > 0 && tn < 15) { t[tn++] = (char)('0' + iv % 10); iv /= 10; }
        while (tn > 0 && *bn < 255) buf[(*bn)++] = t[--tn];
        return;
    }
    append_text(buf, bn, "undefined");
}

static Value eval_bin(int op, Value l, Value r) {
    double ln = to_number(l), rn = to_number(r);
    switch (op) {
        case TOK_PLUS:
            if (l.type == V_STRING || r.type == V_STRING) {
                char buf[256]; int bn = 0;
                append_value(buf, &bn, l);
                append_value(buf, &bn, r);
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

static Value eval_node(int idx);

static Value exec_chain(int cur) {
    Value last = make_undefined();
    while (cur >= 0 && !g_returning && !g_abort) {
        if (++g_steps > STEP_LIMIT) { g_abort = 1; break; }
        last = eval_node(cur);
        cur = g_nodes[cur].next;
    }
    return last;
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
            for (int i = 0; i < g_elem_count; i++)
                if (str_eq(g_elems[i].id, id)) return make_object(g_elems[i].object_index);
            return make_null();
        }
        case 3: return make_null();
        case 4: {
            if (this_val.type == V_OBJECT && nac >= 2 && args[1].type == V_FUNCTION)
                obj_set(this_val.obj, "clickHandler", make_function(args[1].func));
            return make_undefined();
        }
    }
    return make_undefined();
}

static Value call_function(int f, Value this_val, Value *args, int nac) {
    if (f < 0 || f >= g_func_count) return make_undefined();
    if (g_funcs[f].native) return js_native_call(g_funcs[f].native_id, this_val, args, nac);
    if (scope_push() < 0) return make_undefined();
    for (int i = 0; i < g_funcs[f].param_count && i < nac; i++) scope_bind(g_funcs[f].params[i], args[i]);
    int body = g_funcs[f].body;
    g_returning = 0;
    if (body >= 0) exec_chain(g_nodes[body].a);
    Value ret = g_returning ? g_ret : make_undefined();
    g_returning = 0;
    scope_pop();
    return ret;
}

static Value eval_call(int idx) {
    Node *n = &g_nodes[idx];
    if (n->a < 0) return make_undefined();
    Value this_val = make_undefined();
    if (g_nodes[n->a].kind == N_MEMBER) this_val = eval_node(g_nodes[n->a].a);
    Value callee = eval_node(n->a);
    Value args[8]; int nac = 0;
    int ca = n->b;
    while (ca >= 0 && nac < 8) {
        args[nac++] = eval_node(g_nodes[ca].a);
        ca = g_nodes[ca].b;
    }
    if (callee.type == V_FUNCTION) return call_function(callee.func, this_val, args, nac);
    return make_undefined();
}

static Value eval_node(int idx) {
    if (idx < 0 || g_abort) return make_undefined();
    if (++g_steps > STEP_LIMIT || g_depth >= DEPTH_LIMIT) { g_abort = 1; return make_undefined(); }
    g_depth++;
    Node *n = &g_nodes[idx];
    Value r = make_undefined();
    switch (n->kind) {
        case N_PROGRAM:
        case N_BLOCK:
            r = exec_chain(n->a);
            break;
        case N_LIT:
            if (n->c == 1) r = make_boolean(n->num != 0);
            else if (n->c == 2) r = make_null();
            else if (n->c == 3) r = make_undefined();
            else if (n->str >= 0) r = make_string(n->str);
            else r = make_number(n->num);
            break;
        case N_ID:
            r = scope_get(n->name);
            break;
        case N_BIN: {
            Value l = eval_node(n->a);
            Value rr = eval_node(n->c);
            r = eval_bin(n->b, l, rr);
            break;
        }
        case N_UN: {
            Value v = eval_node(n->a);
            r = (n->b == 1) ? make_number(-to_number(v)) : make_boolean(!to_bool(v));
            break;
        }
        case N_MEMBER: {
            Value ov = eval_node(n->a);
            if (ov.type == V_OBJECT) r = obj_get(ov.obj, n->name);
            break;
        }
        case N_CALL:
            r = eval_call(idx);
            break;
        case N_FN: {
            int f = new_func();
            if (f >= 0) {
                g_funcs[f].body = n->c;
                g_funcs[f].param_count = n->param_count;
                for (int i = 0; i < n->param_count && i < MAX_PARAMS; i++) copy_str(g_funcs[f].params[i], 32, n->params[i]);
                r = make_function(f);
            }
            break;
        }
        case N_VAR: {
            Value v = n->a >= 0 ? eval_node(n->a) : make_undefined();
            if (n->name[0]) scope_set(n->name, v);
            r = v;
            break;
        }
        case N_IF: {
            Value c = eval_node(n->a);
            if (to_bool(c)) r = eval_node(n->b);
            else r = eval_node(n->c);
            break;
        }
        case N_RETURN:
            g_ret = n->a >= 0 ? eval_node(n->a) : make_undefined();
            g_returning = 1;
            r = g_ret;
            break;
        case N_EXPR:
            r = eval_node(n->a);
            break;
        case N_ASSIGN: {
            Value v = eval_node(n->b);
            Node *t = &g_nodes[n->a];
            if (t->kind == N_ID) scope_set(t->name, v);
            else if (t->kind == N_MEMBER) {
                Value ov = eval_node(t->a);
                if (ov.type == V_OBJECT) obj_set(ov.obj, t->name, v);
            }
            r = v;
            break;
        }
        default: break;
    }
    g_depth--;
    return r;
}

}

void js_init(void) {
    g_node_count = 0; g_value_count = 0; g_str_pool_len = 0;
    g_object_count = 0; g_func_count = 0; g_elem_count = 0; g_scope_depth = 0;
    g_overflow = 0; g_abort = 0; g_steps = 0; g_depth = 0; g_returning = 0;
    g_pos = 0;

    scope_push();
    int glob = new_object();
    int doc = new_object();
    obj_set(doc, "getElementById", make_native(2));
    obj_set(doc, "querySelector", make_native(3));
    obj_set(glob, "document", make_object(doc));
    obj_set(glob, "alert", make_native(0));
    int con = new_object();
    obj_set(con, "log", make_native(1));
    obj_set(glob, "console", make_object(con));
    scope_set("window", make_object(glob));
    scope_set("document", make_object(doc));
    scope_set("alert", make_native(0));
    scope_set("console", make_object(con));
}

int js_run(const char *source, int length) {
    g_src = source; g_src_len = length; g_pos = 0;
    g_overflow = 0; g_abort = 0; g_steps = 0; g_depth = 0; g_returning = 0;
    next_token();
    int program = parse_program();
    if (g_overflow) return 0;          /* skrypt za duzy dla interpretera - pomijamy */
    eval_node(program);
    g_returning = 0;
    return g_abort ? 0 : 1;
}

void js_register_element(const char *tag, const char *id, int page_index) {
    if (id[0] == 0) return;
    for (int i = 0; i < g_elem_count; i++) if (str_eq(g_elems[i].id, id)) return;
    if (g_elem_count >= MAX_ELEMS) return;
    int o = new_object();
    if (o < 0) return;
    ElemReg *e = &g_elems[g_elem_count++];
    copy_str(e->tag, 16, tag);
    copy_str(e->id, 64, id);
    e->page_index = page_index;
    obj_set(o, "tag", make_string(intern_string(tag, str_len(tag))));
    obj_set(o, "id", make_string(intern_string(id, str_len(id))));
    obj_set(o, "clickHandler", make_number(-1));
    obj_set(o, "innerText", make_string(intern_string("", 0)));
    int st = new_object();
    obj_set(st, "color", make_string(intern_string("", 0)));
    obj_set(o, "style", make_object(st));
    obj_set(o, "addEventListener", make_native(4));
    e->object_index = o;
}

int js_find_element_by_id(const char *id, char *tag_out, int tag_cap, int *page_index_out) {
    for (int i = 0; i < g_elem_count; i++) {
        if (str_eq(g_elems[i].id, id)) {
            if (tag_out && tag_cap > 0) copy_str(tag_out, tag_cap, g_elems[i].tag);
            if (page_index_out) *page_index_out = g_elems[i].page_index;
            return i;
        }
    }
    return -1;
}

void js_dispatch_click(const char *element_id) {
    for (int i = 0; i < g_elem_count; i++) {
        if (!str_eq(g_elems[i].id, element_id)) continue;
        Value h = obj_get(g_elems[i].object_index, "clickHandler");
        if (h.type == V_FUNCTION && h.func >= 0) {
            g_steps = 0; g_abort = 0; g_depth = 0; g_returning = 0;
            Value self = make_object(g_elems[i].object_index);
            call_function(h.func, self, 0, 0);
        }
        return;
    }
}