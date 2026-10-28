#include "browser_css.h"

namespace {

static int g_rule_count = 0;
static int g_pseudo_on = 0;
static char g_pseudo_target[64] = {0};
static char g_pseudo_name[16] = {0};

struct CssRule {
    char selector[64];
    uint8_t kind;
    char pseudo[16];
    struct browser_style style;
    uint8_t has[16];
};
static CssRule g_rules[64];

static int str_len(const char *s) {
    int n = 0;
    while (s && s[n]) n++;
    return n;
}

static int str_eq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

static int str_ncase(const char *a, const char *b) {
    while (*a && *b) {
        char x = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
        char y = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (x != y) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)(c >= 'A' && c <= 'Z' ? c + 32 : c);
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int parse_color(const char *v, int n, uint32_t *out) {
    while (n > 0 && (v[0] == ' ' || v[0] == '\t' || v[0] == '\n')) { v++; n--; }
    while (n > 0 && (v[n-1] == ' ' || v[n-1] == '\t' || v[n-1] == '\n')) n--;
    if (n == 4 && v[0] == '#') {
        int r = hex_val(v[1]), g = hex_val(v[2]), b = hex_val(v[3]);
        if (r < 0 || g < 0 || b < 0) return 0;
        *out = ((uint32_t)(r * 17) << 16) | ((uint32_t)(g * 17) << 8) | (uint32_t)(b * 17);
        return 1;
    }
    if (n == 7 && v[0] == '#') {
        uint32_t rgb = 0;
        for (int i = 1; i < 7; i++) {
            int d = hex_val(v[i]);
            if (d < 0) return 0;
            rgb = (rgb << 4) | (uint32_t)d;
        }
        *out = rgb;
        return 1;
    }
    static const char *names[] = {
        "black","white","red","green","blue","yellow","gray","grey",
        "silver","maroon","purple","teal","navy","orange","lime"
    };
    static const uint32_t cols[] = {
        0x00000000,0x00FFFFFF,0x00FF0000,0x00008000,0x000000FF,0x00FFFF00,
        0x00808080,0x00808080,0x00C0C0C0,0x00800000,0x00800080,0x00008080,
        0x00000080,0x00FFA500,0x0000FF00
    };
    for (uint32_t i = 0; i < sizeof(names)/sizeof(names[0]); i++) {
        if (str_ncase(v, names[i]) && names[i][n] == 0) { *out = cols[i]; return 1; }
    }
    return 0;
}

static void decls(const char *t, int s, int e, struct browser_style *st, uint8_t *has) {
    int p = s;
    while (p < e) {
        while (p < e && (t[p]==' '||t[p]=='\t'||t[p]=='\n'||t[p]==';')) p++;
        int ps = p;
        while (p < e && t[p]!=':' && t[p]!=';') p++;
        if (p >= e || t[p] != ':') { while (p < e && t[p] != ';') p++; continue; }
        int pe = p++;
        while (pe > ps && t[pe-1] == ' ') pe--;
        int vs = p;
        while (p < e && t[p] != ';') p++;
        int ve = p;
        int plen = pe - ps;
        uint32_t col = 0;
        if (plen == 5 && str_ncase(t + ps, "color") && parse_color(t + vs, ve - vs, &col)) {
            st->color = col; if (has) has[0] = 1;
        } else if ((plen == 16 && str_ncase(t + ps, "background-color")) ||
                   (plen == 10 && str_ncase(t + ps, "background"))) {
            if (parse_color(t + vs, ve - vs, &col)) {
                st->background = col; st->has_background = 1; if (has) has[1] = 1;
            }
        } else if (plen == 11 && str_ncase(t + ps, "font-weight")) {
            int n = 0, dig = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); dig = 1; } }
            int vl = ve - vs;
            if (dig) { if (n >= 600) st->bold = 1; else if (n <= 500) st->bold = 0; }
            else if (vl >= 4 && str_ncase(t + vs, "bold")) st->bold = 1;
            else if (vl >= 6 && str_ncase(t + vs, "normal")) st->bold = 0;
        } else if (plen == 9 && str_ncase(t + ps, "font-size")) {
            int n = 0, dig = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); dig = 1; } }
            int vl = ve - vs;
            if (dig) { if (n <= 10) st->rozmiar = 0; else if (n <= 14) st->rozmiar = 1; else st->rozmiar = 2; }
            else if (vl >= 7 && str_ncase(t + vs, "xx-small")) st->rozmiar = 0;
            else if (vl >= 6 && str_ncase(t + vs, "x-small")) st->rozmiar = 0;
            else if (vl >= 5 && str_ncase(t + vs, "small")) st->rozmiar = 1;
            else if (vl >= 6 && str_ncase(t + vs, "medium")) st->rozmiar = 1;
            else if (vl >= 5 && str_ncase(t + vs, "large")) st->rozmiar = 2;
            else if (vl >= 7 && str_ncase(t + vs, "x-large")) st->rozmiar = 2;
            else if (vl >= 8 && str_ncase(t + vs, "xx-large")) st->rozmiar = 2;
        } else if (plen == 12 && str_ncase(t + ps, "border-color")) {
            if (parse_color(t + vs, ve - vs, &col)) { st->border_color = col; st->has_border_color = 1; if (has) has[2] = 1; }
        } else if (plen == 12 && str_ncase(t + ps, "border-width")) {
            int n = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); }
            st->border_width = (uint16_t)n; st->has_border_width = 1; if (has) has[3] = 1;
        } else if (plen == 8 && str_ncase(t + ps, "padding")) {
            int n = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); }
            st->padding = (uint16_t)n; st->has_padding = 1; if (has) has[4] = 1;
        } else if (plen == 6 && str_ncase(t + ps, "margin")) {
            int n = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); }
            st->margin = (uint16_t)n; st->has_margin = 1; if (has) has[5] = 1;
        } else if (plen == 10 && str_ncase(t + ps, "text-align")) {
            int vl = ve - vs;
            if (vl >= 5 && str_ncase(t + vs, "center")) st->text_align = 1;
            else if (vl >= 5 && str_ncase(t + vs, "right")) st->text_align = 2;
            else st->text_align = 0;
        } else if (plen == 7 && str_ncase(t + ps, "display")) {
            int vl = ve - vs;
            if (vl >= 4 && str_ncase(t + vs, "none")) st->display = 2;
            else if (vl >= 5 && str_ncase(t + vs, "block")) st->display = 1;
            else st->display = 0;
        } else if (plen == 5 && str_ncase(t + ps, "width")) {
            int n = 0;
            for (int i = vs; i < ve; i++) { char c = t[i]; if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); }
            st->width = (uint16_t)n; st->has_width = 1; if (has) has[6] = 1;
        } else if (plen == 6 && str_ncase(t + ps, "cursor")) {
            int vl = ve - vs;
            if (vl > 0 && vl < 15) {
                for (int i = 0; i < vl; i++) st->cursor[i] = t[vs + i];
                st->cursor[vl] = 0; st->has_cursor = 1; if (has) has[7] = 1;
            }
        }
    }
}

static int selector_kind(const char *sel, char *kind_out) {
    if (sel[0] == '.') { *kind_out = 1; return 1; }
    if (sel[0] == '#') { *kind_out = 2; return 1; }
    *kind_out = 0;
    return str_len(sel);
}

}

void css_init(void) {
    g_rule_count = 0;
    g_pseudo_on = 0;
    g_pseudo_target[0] = 0;
    g_pseudo_name[0] = 0;
    for (int i = 0; i < 64; i++) {
        g_rules[i].kind = 0;
        g_rules[i].selector[0] = 0;
        g_rules[i].pseudo[0] = 0;
        g_rules[i].style.color = 0x001E293B;
        g_rules[i].style.background = 0x00FFFFFF;
        g_rules[i].style.has_background = 0;
        g_rules[i].style.bold = 0;
        g_rules[i].style.rozmiar = 0;
        g_rules[i].style.has_border_color = 0;
        g_rules[i].style.has_border_width = 0;
        g_rules[i].style.has_padding = 0;
        g_rules[i].style.has_margin = 0;
        g_rules[i].style.text_align = 0;
        g_rules[i].style.display = 0;
        g_rules[i].style.has_width = 0;
        g_rules[i].style.cursor[0] = 0;
        g_rules[i].style.has_cursor = 0;
        for (int j = 0; j < 16; j++) g_rules[i].has[j] = 0;
    }
}

void css_parse_stylesheet(const char *text, int length) {
    int p = 0;
    while (p < length && g_rule_count < 64) {
        while (p < length && (text[p]==' '||text[p]=='\t'||text[p]=='\n'||text[p]=='}')) p++;
        int ss = p;
        while (p < length && text[p] != '{') p++;
        if (p >= length) break;
        int se = p++;
        int de = p;
        while (de < length && text[de] != '}') de++;
        char sel[64]; int sn = 0;
        int k = 0;
        int si = ss;
        while (si < se && text[si] != ' ' && text[si] != '.' && text[si] != '#' &&
               text[si] != ':' && text[si] != ',' && sn < 63) sel[sn++] = text[si++];
        sel[sn] = 0;
        char ps[16] = {0}; int pn = 0;
        if (si < se && text[si] == ':') {
            si++;
            while (si < se && text[si] != ' ' && text[si] != ',' && pn < 15) ps[pn++] = text[si++];
            ps[pn] = 0;
        }
        if (sn > 0 && g_rule_count < 64) {
            CssRule *r = &g_rules[g_rule_count++];
            for (int i = 0; i < sn; i++) r->selector[i] = (char)(sel[i] >= 'A' && sel[i] <= 'Z' ? sel[i] + 32 : sel[i]);
            r->selector[sn] = 0;
            r->kind = 0;
            if (sn > 0 && sel[0] == '.') { r->kind = 1; for (int i = 0; i < sn - 1; i++) r->selector[i] = sel[i + 1]; r->selector[sn - 1] = 0; }
            else if (sn > 0 && sel[0] == '#') { r->kind = 2; for (int i = 0; i < sn - 1; i++) r->selector[i] = sel[i + 1]; r->selector[sn - 1] = 0; }
            for (int i = 0; ps[i] && i < 15; i++) r->pseudo[i] = ps[i];
            r->pseudo[15] = 0;
            r->style.color = 0x001E293B;
            r->style.background = 0x00FFFFFF;
            r->style.has_background = 0;
            r->style.bold = 0;
            r->style.rozmiar = 0;
            r->style.has_border_color = 0;
            r->style.has_border_width = 0;
            r->style.has_padding = 0;
            r->style.has_margin = 0;
            r->style.text_align = 0;
            r->style.display = 0;
            r->style.has_width = 0;
            r->style.cursor[0] = 0;
            r->style.has_cursor = 0;
            for (int j = 0; j < 16; j++) r->has[j] = 0;
            decls(text, p, de, &r->style, r->has);
        }
        p = de < length ? de + 1 : length;
    }
}

void css_set_pseudo(const char *target, const char *pseudo, int on) {
    g_pseudo_on = on ? 1 : 0;
    int n = str_len(target);
    if (n > 63) n = 63;
    for (int i = 0; i < n; i++) g_pseudo_target[i] = target[i];
    g_pseudo_target[n] = 0;
    int m = str_len(pseudo);
    if (m > 15) m = 15;
    for (int i = 0; i < m; i++) g_pseudo_name[i] = pseudo[i];
    g_pseudo_name[m] = 0;
}

static int matches(const CssRule *r, const char *tag, const char *class_name, const char *id, const char *pseudo) {
    if (r->pseudo[0]) {
        int pm = str_ncase(r->pseudo, pseudo) && pseudo[0] != 0;
        if (!pm) return 0;
        if (g_pseudo_on == 0) return 0;
        if (r->selector[0] && g_pseudo_target[0] && !str_eq(g_pseudo_target, id)) return 0;
    }
    if (r->kind == 0) return str_ncase(r->selector, tag) && tag[str_len(r->selector)] == 0;
    if (r->kind == 1) return class_name && str_ncase(r->selector, class_name);
    if (r->kind == 2) return id && str_ncase(r->selector, id);
    return 0;
}

void css_compute(const char *tag, const char *class_name, const char *id,
                 const char *pseudo, struct browser_style *out) {
    out->color = 0x001E293B;
    out->background = 0x00FFFFFF;
    out->has_background = 0;
    out->bold = 0;
    out->rozmiar = 0;
    out->has_border_color = 0;
    out->has_border_width = 0;
    out->has_padding = 0;
    out->has_margin = 0;
    out->text_align = 0;
    out->display = 0;
    out->has_width = 0;
    out->cursor[0] = 0;
    out->has_cursor = 0;
    for (int i = 0; i < g_rule_count; i++) {
        if (!matches(&g_rules[i], tag, class_name, id, pseudo)) continue;
        const struct browser_style *s = &g_rules[i].style;
        out->color = s->color;
        if (s->has_background) { out->background = s->background; out->has_background = 1; }
        if (s->bold) out->bold = 1;
        out->rozmiar = s->rozmiar;
        if (s->has_border_color) { out->border_color = s->border_color; out->has_border_color = 1; }
        if (s->has_border_width) { out->border_width = s->border_width; out->has_border_width = 1; }
        if (s->has_padding) { out->padding = s->padding; out->has_padding = 1; }
        if (s->has_margin) { out->margin = s->margin; out->has_margin = 1; }
        out->text_align = s->text_align;
        out->display = s->display;
        if (s->has_width) { out->width = s->width; out->has_width = 1; }
        if (s->has_cursor) { for (int j = 0; j < 15 && s->cursor[j]; j++) out->cursor[j] = s->cursor[j]; out->cursor[15] = 0; out->has_cursor = 1; }
    }
}
