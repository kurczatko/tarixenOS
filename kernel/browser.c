#include "browser.h"
#include "tls_client.h"
#include "../drivers/net.h"
#include "../drivers/keyboard.h"
#include "../graficzny/okno.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/czcionka.h"
#include "browser_css.h"
#include "browser_js.h"

#define BROWSER_WIDTH 760
#define BROWSER_HEIGHT 570
#define BROWSER_RESPONSE_MAX 8192
#define BROWSER_PAGE_MAX 6000
#define BROWSER_LINKS_MAX 32
#define BROWSER_CSS_RULES 48
#define BROWSER_STYLE_DEPTH 32
#define BROWSER_FIELDS_MAX 16

#define FIELD_MARKER '\x02'
#define BULLET_MARKER '\x03'

struct browser_link {
    uint16_t start;
    uint16_t end;
    char url[256];
};

struct browser_css_rule {
    char selector[32];
    uint8_t selector_kind;
    struct browser_style style;
    uint8_t has_color;
};

struct browser_field {
    int x;
    int y;
    int width;
    int height;
    uint8_t type;
    uint8_t focused;
    char value[128];
    int cursor;
    char action[256];
    uint8_t method_get;
    uint32_t page_start;
    uint32_t page_width;
};

static struct okno browser_window;
static char address[128] = "http://textfiles.com/";
static char response[BROWSER_RESPONSE_MAX];
static char page[BROWSER_PAGE_MAX];
static uint32_t page_colors[BROWSER_PAGE_MAX];
static uint32_t page_backgrounds[BROWSER_PAGE_MAX];
static uint8_t page_bold[BROWSER_PAGE_MAX];
static uint8_t page_rozmiar[BROWSER_PAGE_MAX];
static struct browser_link links[BROWSER_LINKS_MAX];
static uint8_t link_count;
static struct browser_css_rule css_rules[BROWSER_CSS_RULES];
static uint8_t css_rule_count;
static struct browser_field fields[BROWSER_FIELDS_MAX];
static uint8_t field_count;
static int focused_field;
static uint32_t page_background = 0x00FFFFFF;
static char status[64] = "Wpisz adres i nacisnij IDZ";
static uint8_t address_focused = 1;
static uint8_t browser_opened;
static int page_scroll_line;
static uint8_t browser_use_https;
static char current_form_action[256];
static uint8_t current_form_method_get;

static char js_elem_id[32][64];
static int js_elem_page[32];
static int js_elem_count;

static void js_note_element(const char *tag, const char *id, int page_index)
{
    if (!id || !id[0]) return;
    for (int i = 0; i < js_elem_count; i++) {
        if (js_elem_page[i] == page_index) return;
    }
    if (js_elem_count >= 32) return;
    int n = 0;
    while (id[n] && n < 63) { js_elem_id[js_elem_count][n] = id[n]; n++; }
    js_elem_id[js_elem_count][n] = 0;
    js_elem_page[js_elem_count] = page_index;
    js_elem_count++;
    js_register_element(tag, id, page_index);
}

static const char *js_find_id_by_page(int page_index)
{
    for (int i = 0; i < js_elem_count; i++) {
        if (js_elem_page[i] == page_index) return js_elem_id[i];
    }
    return 0;
}

static void copy_text(char *destination, uint32_t capacity, const char *source)
{
    uint32_t i = 0;
    if (!capacity) return;
    while (source[i] && i + 1 < capacity) { destination[i] = source[i]; i++; }
    destination[i] = 0;
}

static int starts_with(const char *text, const char *prefix)
{
    while (*prefix) if (*text++ != *prefix++) return 0;
    return 1;
}

static void set_status(const char *text)
{
    copy_text(status, sizeof(status), text);
}

void js_host_alert(const char *msg)
{
    copy_text(status, sizeof(status), msg);
    if (browser_opened && browser_window.otwarte) okno_narysuj(&browser_window);
}

void js_host_log(const char *msg)
{
    (void)msg;
}

static int lower_char(char c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

static int text_equal_nocase(const char *left, const char *right)
{
    while (*left && *right) {
        if (lower_char(*left++) != lower_char(*right++)) return 0;
    }
    return !*left && !*right;
}

static int slice_equal_nocase(const char *text, int start, int length, const char *word)
{
    int i = 0;
    while (i < length && word[i]) {
        if (lower_char(text[start + i]) != word[i]) return 0;
        i++;
    }
    return i == length && word[i] == 0;
}

static int slice_contains_word_nocase(const char *text, int start, int length, const char *word)
{
    int word_length = 0;
    while (word[word_length]) word_length++;
    for (int position = 0; position + word_length <= length; position++) {
        int before_ok = position == 0 || text[start + position - 1] == ' ' || text[start + position - 1] == '\t';
        int after_ok = position + word_length == length || text[start + position + word_length] == ' ' ||
                       text[start + position + word_length] == '\t';
        if (before_ok && after_ok && slice_equal_nocase(text, start + position, word_length, word)) return 1;
    }
    return 0;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)lower_char(c);
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int parse_color_value(const char *value, int length, uint32_t *color)
{
    static const struct { const char *name; uint32_t color; } named[] = {
        {"black", 0x00000000}, {"white", 0x00FFFFFF}, {"red", 0x00FF0000},
        {"green", 0x00008000}, {"blue", 0x000000FF}, {"yellow", 0x00FFFF00},
        {"gray", 0x00808080}, {"grey", 0x00808080}, {"silver", 0x00C0C0C0},
        {"maroon", 0x00800000}, {"purple", 0x00800080}, {"teal", 0x00008080},
        {"navy", 0x00000080}, {"orange", 0x00FFA500}, {"lime", 0x0000FF00}
    };
    while (length > 0 && (value[0] == ' ' || value[0] == '\t' || value[0] == '\n')) { value++; length--; }
    while (length > 0 && (value[length - 1] == ' ' || value[length - 1] == '\t' || value[length - 1] == '\n')) length--;
    if (length == 4 && value[0] == '#') {
        int r = hex_value(value[1]), g = hex_value(value[2]), b = hex_value(value[3]);
        if (r < 0 || g < 0 || b < 0) return 0;
        *color = ((uint32_t)(r * 17) << 16) | ((uint32_t)(g * 17) << 8) | (uint32_t)(b * 17);
        return 1;
    }
    if (length == 7 && value[0] == '#') {
        uint32_t rgb = 0;
        for (int i = 1; i < 7; i++) {
            int digit = hex_value(value[i]);
            if (digit < 0) return 0;
            rgb = (rgb << 4) | (uint32_t)digit;
        }
        *color = rgb;
        return 1;
    }
    for (uint32_t i = 0; i < sizeof(named) / sizeof(named[0]); i++) {
        int name_length = 0;
        while (named[i].name[name_length]) name_length++;
        if (name_length != length) continue;
        int match = 1;
        for (int c = 0; c < length; c++) if (lower_char(value[c]) != named[i].name[c]) match = 0;
        if (match) { *color = named[i].color; return 1; }
    }
    return 0;
}

static void parse_style_declarations(const char *text, int start, int end,
                                     struct browser_style *style, uint8_t *has_color)
{
    int position = start;
    while (position < end) {
        while (position < end && (text[position] == ' ' || text[position] == '\t' ||
                                  text[position] == '\n' || text[position] == ';')) position++;
        int property_start = position;
        while (position < end && text[position] != ':' && text[position] != ';') position++;
        if (position >= end || text[position] != ':') {
            while (position < end && text[position] != ';') position++;
            continue;
        }
        int property_end = position++;
        while (property_end > property_start && text[property_end - 1] == ' ') property_end--;
        int value_start = position;
        while (position < end && text[position] != ';') position++;
        int value_end = position;
        uint32_t parsed_color;
        int property_length = property_end - property_start;
        if ((property_length == 5 && slice_equal_nocase(text, property_start, property_length, "color")) &&
            parse_color_value(text + value_start, value_end - value_start, &parsed_color)) {
            style->color = parsed_color;
            *has_color = 1;
        } else if (((property_length == 16 && slice_equal_nocase(text, property_start, property_length, "background-color")) ||
                (property_length == 10 && slice_equal_nocase(text, property_start, property_length, "background"))) &&
                   parse_color_value(text + value_start, value_end - value_start, &parsed_color)) {
            style->background = parsed_color;
            style->has_background = 1;
        } else if (property_length == 11 && slice_equal_nocase(text, property_start, property_length, "font-weight")) {
            int vw = value_end - value_start;
            int n = 0;
            int has_digits = 0;
            for (int v = value_start; v < value_end; v++) {
                char cc = text[v];
                if (cc >= '0' && cc <= '9') { n = n * 10 + (cc - '0'); has_digits = 1; }
            }
            if (has_digits) {
                if (n >= 600) style->bold = 1;
                else if (n <= 500) style->bold = 0;
            } else if (vw >= 4 && slice_equal_nocase(text, value_start, 4, "bold")) {
                style->bold = 1;
            } else if (vw >= 6 && slice_equal_nocase(text, value_start, 6, "normal")) {
                style->bold = 0;
            }
        } else if (property_length == 9 && slice_equal_nocase(text, property_start, property_length, "font-size")) {
            int n = 0;
            int has_digits = 0;
            for (int v = value_start; v < value_end; v++) {
                char cc = text[v];
                if (cc >= '0' && cc <= '9') { n = n * 10 + (cc - '0'); has_digits = 1; }
            }
            int vlen = value_end - value_start;
            if (has_digits) {
                if (n <= 10) style->rozmiar = 0;
                else if (n <= 14) style->rozmiar = 1;
                else style->rozmiar = 2;
            } else if (slice_equal_nocase(text, value_start, vlen, "xx-small") ||
                       slice_equal_nocase(text, value_start, vlen, "x-small")) {
                style->rozmiar = 0;
            } else if (slice_equal_nocase(text, value_start, vlen, "small") ||
                       slice_equal_nocase(text, value_start, vlen, "medium")) {
                style->rozmiar = 1;
            } else if (slice_equal_nocase(text, value_start, vlen, "large") ||
                       slice_equal_nocase(text, value_start, vlen, "x-large") ||
                       slice_equal_nocase(text, value_start, vlen, "xx-large")) {
                style->rozmiar = 2;
            }
        }
    }
}

static void parse_css_stylesheet(const char *text, int start, int end)
{
    int position = start;
    while (position < end && css_rule_count < BROWSER_CSS_RULES) {
        while (position < end && (text[position] == ' ' || text[position] == '\t' ||
                                  text[position] == '\n' || text[position] == '}')) position++;
        int selector_start = position;
        while (position < end && text[position] != '{') position++;
        if (position >= end) break;
        int selector_end = position++;
        while (selector_end > selector_start && text[selector_end - 1] == ' ') selector_end--;
        int name_start = selector_start;
        uint8_t selector_kind = 0;
        if (name_start < selector_end && text[name_start] == '.') { selector_kind = 1; name_start++; }
        else if (name_start < selector_end && text[name_start] == '#') { selector_kind = 2; name_start++; }
        int name_end = name_start;
        while (name_end < selector_end && text[name_end] != ' ' && text[name_end] != '.' &&
               text[name_end] != '#' && text[name_end] != ':' && text[name_end] != ',') name_end++;
        int name_length = name_end - name_start;
        int declarations_end = position;
        while (declarations_end < end && text[declarations_end] != '}') declarations_end++;
        if (name_length > 0 && name_length < (int)sizeof(css_rules[0].selector)) {
            struct browser_css_rule *rule = &css_rules[css_rule_count++];
            for (int i = 0; i < name_length; i++) rule->selector[i] = (char)lower_char(text[name_start + i]);
            rule->selector[name_length] = 0;
            rule->selector_kind = selector_kind;
            rule->style.color = 0x001E293B;
            rule->style.background = 0x00FFFFFF;
            rule->style.has_background = 0;
            rule->style.bold = 0;
            rule->style.rozmiar = 0;
            rule->has_color = 0;
            parse_style_declarations(text, position, declarations_end, &rule->style, &rule->has_color);
        }
        position = declarations_end < end ? declarations_end + 1 : end;
    }
}

static int append_page_character(char c, int *output, uint32_t color, uint32_t background,
                                 uint8_t bold, uint8_t rozmiar)
{
    if (*output >= BROWSER_PAGE_MAX - 1) return 0;
    page[*output] = c;
    page_colors[*output] = color;
    page_backgrounds[*output] = background;
    page_bold[*output] = bold;
    page_rozmiar[*output] = rozmiar;
    (*output)++;
    return 1;
}

static int slice_match_nocase(const char *text, int start, int length, const char *word)
{
    return slice_equal_nocase(text, start, length, word);
}

static void create_field(uint8_t type, const char *value, int width, const char *action, uint8_t method_get, int page_start, uint32_t page_width)
{
    if (field_count >= BROWSER_FIELDS_MAX) return;
    struct browser_field *f = &fields[field_count++];
    f->x = 0; f->y = 0; f->width = width; f->height = 8;
    f->type = type; f->focused = 0; f->cursor = 0;
    f->page_start = (uint32_t)page_start;
    f->page_width = page_width;
    copy_text(f->action, sizeof(f->action), action ? action : current_form_action);
    f->method_get = method_get ? current_form_method_get : 1;
    copy_text(f->value, sizeof(f->value), value ? value : "");
    f->cursor = 0;
    while (f->value[f->cursor]) f->cursor++;
}

static void page_from_response(int response_length)
{
    int body = 0;
    int output = 0;
    int in_script = 0;
    int in_style = 0;
    int anchor_active = 0;
    int anchor_start = 0;
    int style_depth = 0;
    struct browser_style current_style = {0};
    current_style.color = 0x001E293B;
    current_style.background = 0x00FFFFFF;
    struct browser_style style_stack[BROWSER_STYLE_DEPTH];
    char tag_stack[BROWSER_STYLE_DEPTH][16];
    char anchor_url[256];

    page[0] = 0;
    link_count = 0;
    css_rule_count = 0;
    field_count = 0;
    focused_field = -1;
    page_background = 0x00FFFFFF;
    page_scroll_line = 0;
    current_form_action[0] = 0;
    current_form_method_get = 1;
    while (body + 3 < response_length) {
        if (response[body] == '\r' && response[body + 1] == '\n' &&
            response[body + 2] == '\r' && response[body + 3] == '\n') {
            body += 4;
            break;
        }
        body++;
    }
    if (body >= response_length) {
        copy_text(page, sizeof(page), response);
        for (int i = 0; page[i] && i < BROWSER_PAGE_MAX; i++) {
            page_colors[i] = current_style.color;
            page_backgrounds[i] = current_style.background;
            page_bold[i] = current_style.bold;
            page_rozmiar[i] = current_style.rozmiar;
        }
        return;
    }

    for (int i = body; i < response_length && output + 2 < BROWSER_PAGE_MAX; i++) {
        char c = response[i];
        if (c == '<') {
            int tag_start = i + 1;
            int closing = response[tag_start] == '/';
            if (closing) tag_start++;
            int tag_end = tag_start;
            while (tag_end < response_length && response[tag_end] != '>') tag_end++;
            int name_end = tag_start;
            while (name_end < tag_end && response[name_end] != ' ' && response[name_end] != '\t' &&
                   response[name_end] != '/' && response[name_end] != '\n') name_end++;
            int name_length = name_end - tag_start;
            char tag[16];
            if (name_length >= (int)sizeof(tag)) name_length = sizeof(tag) - 1;
            for (int n = 0; n < name_length; n++) tag[n] = (char)lower_char(response[tag_start + n]);
            tag[name_length] = 0;

            if (text_equal_nocase(tag, "style")) {
                if (closing) in_style = 0;
                else {
                    int close_tag = tag_end + 1;
                    while (close_tag + 7 < response_length &&
                           !(response[close_tag] == '<' && response[close_tag + 1] == '/' &&
                             slice_equal_nocase(response, close_tag + 2, 5, "style"))) close_tag++;
                    parse_css_stylesheet(response, tag_end + 1, close_tag);
                    css_parse_stylesheet(response + tag_end + 1, close_tag - (tag_end + 1));
                    in_style = 1;
                }
                i = tag_end;
                continue;
            }
            if (in_style) { i = tag_end; continue; }
            if (text_equal_nocase(tag, "script")) {
                if (closing) { in_script = 0; i = tag_end; continue; }
                int s_start = tag_end + 1;
                int s_end = s_start;
                while (s_end + 7 < response_length &&
                       !(response[s_end] == '<' && response[s_end + 1] == '/' &&
                         slice_equal_nocase(response, s_end + 2, 6, "script"))) s_end++;
                if (s_end > s_start && s_end - s_start < 2048) {
                    static char js_buf[2048];
                    int jn = 0;
                    while (s_start < s_end && jn < 2047) { js_buf[jn++] = response[s_start++]; }
                    js_buf[jn] = 0;
                    js_run(js_buf, jn);
                }
                in_script = 1;
                i = s_end < response_length ? s_end : tag_end;
                continue;
            }

            if (closing) {
                if (text_equal_nocase(tag, "a") && anchor_active) {
                    if (link_count < BROWSER_LINKS_MAX && output > anchor_start) {
                        links[link_count].start = (uint16_t)anchor_start;
                        links[link_count].end = (uint16_t)output;
                        copy_text(links[link_count].url, sizeof(links[link_count].url), anchor_url);
                        link_count++;
                    }
                    anchor_active = 0;
                }
                if (text_equal_nocase(tag, "form")) {
                    current_form_action[0] = 0;
                    current_form_method_get = 1;
                }
                for (int depth = style_depth - 1; depth >= 0; depth--) {
                    if (text_equal_nocase(tag_stack[depth], tag)) {
                        current_style = style_stack[depth];
                        style_depth = depth;
                        break;
                    }
                }
            } else {
                int block_tag = text_equal_nocase(tag, "br") || text_equal_nocase(tag, "p") ||
                                text_equal_nocase(tag, "div") || text_equal_nocase(tag, "li") ||
                                text_equal_nocase(tag, "h1") || text_equal_nocase(tag, "h2") ||
                                text_equal_nocase(tag, "h3") || text_equal_nocase(tag, "h4") ||
                                text_equal_nocase(tag, "h5") || text_equal_nocase(tag, "h6") ||
                                text_equal_nocase(tag, "tr") || text_equal_nocase(tag, "blockquote") ||
                                text_equal_nocase(tag, "pre") || text_equal_nocase(tag, "center");
                if (block_tag && output > 0 && page[output - 1] != '\n')
                    append_page_character('\n', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                if (text_equal_nocase(tag, "li"))
                    append_page_character(BULLET_MARKER, &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                if (text_equal_nocase(tag, "hr")) {
                    append_page_character('\n', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                    append_page_character('\x01', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                    append_page_character('\n', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                }
                int skip_style = text_equal_nocase(tag, "br") || text_equal_nocase(tag, "hr") ||
                                 text_equal_nocase(tag, "img") || text_equal_nocase(tag, "meta") ||
                                 text_equal_nocase(tag, "link") || text_equal_nocase(tag, "input") ||
                                 text_equal_nocase(tag, "form") || text_equal_nocase(tag, "button") ||
                                 text_equal_nocase(tag, "textarea") || text_equal_nocase(tag, "select");
                if (style_depth < BROWSER_STYLE_DEPTH && !skip_style) {
                    style_stack[style_depth] = current_style;
                    copy_text(tag_stack[style_depth], sizeof(tag_stack[style_depth]), tag);
                    style_depth++;
                }
                int has_tag_color = 0;
                for (uint32_t rule_index = 0; rule_index < css_rule_count; rule_index++) {
                    if (css_rules[rule_index].selector_kind != 0 ||
                        !text_equal_nocase(css_rules[rule_index].selector, tag)) continue;
                    if (css_rules[rule_index].has_color) {
                        current_style.color = css_rules[rule_index].style.color;
                        has_tag_color = 1;
                    }
                    if (css_rules[rule_index].style.has_background) {
                        current_style.background = css_rules[rule_index].style.background;
                        current_style.has_background = 1;
                    }
                }
                if (text_equal_nocase(tag, "a") && !has_tag_color) current_style.color = 0x000000FF;
                if (text_equal_nocase(tag, "b") || text_equal_nocase(tag, "strong")) current_style.bold = 1;
                if (text_equal_nocase(tag, "h1") || text_equal_nocase(tag, "h2")) current_style.rozmiar = 2;
                else if (text_equal_nocase(tag, "h3")) current_style.rozmiar = 1;

                int class_start = -1, class_length = 0, id_start = -1, id_length = 0;
                char type_value[16] = {0};
                char val_value[128] = {0}; int val_length = 0;
                char action_value[256] = {0}; int action_length = 0;
                char method_value[16] = {0}; int method_length = 0;
                int attribute = name_end;
                while (attribute < tag_end) {
                    while (attribute < tag_end && (response[attribute] == ' ' || response[attribute] == '\t' || response[attribute] == '/')) attribute++;
                    int attribute_start = attribute;
                    while (attribute < tag_end && response[attribute] != '=' && response[attribute] != ' ' && response[attribute] != '\t') attribute++;
                    int attribute_length = attribute - attribute_start;
                    while (attribute < tag_end && (response[attribute] == ' ' || response[attribute] == '\t')) attribute++;
                    if (attribute >= tag_end || response[attribute] != '=') { while (attribute < tag_end && response[attribute] != ' ') attribute++; continue; }
                    attribute++;
                    while (attribute < tag_end && (response[attribute] == ' ' || response[attribute] == '\t')) attribute++;
                    if (attribute >= tag_end) continue;
                    int vs, ve;
                    if (response[attribute] == '\'' || response[attribute] == '"') {
                        char quote = response[attribute++];
                        vs = attribute;
                        while (attribute < tag_end && response[attribute] != quote) attribute++;
                        ve = attribute;
                        if (attribute < tag_end) attribute++;
                    } else {
                        vs = attribute;
                        while (attribute < tag_end && response[attribute] != ' ' &&
                               response[attribute] != '\t' && response[attribute] != '/') attribute++;
                        ve = attribute;
                    }
                    int val_len = ve - vs;
                    if (slice_match_nocase(response, attribute_start, attribute_length, "style")) {
                        uint8_t inline_color = 0;
                        parse_style_declarations(response, vs, ve, &current_style, &inline_color);
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "class")) {
                        class_start = vs; class_length = val_len;
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "id")) {
                        id_start = vs; id_length = val_len;
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "color") ||
                               slice_match_nocase(response, attribute_start, attribute_length, "bgcolor")) {
                        uint32_t color_value;
                        if (parse_color_value(response + vs, val_len, &color_value)) {
                            if (slice_match_nocase(response, attribute_start, attribute_length, "color")) current_style.color = color_value;
                            else { current_style.background = color_value; current_style.has_background = 1; }
                        }
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "href") &&
                               text_equal_nocase(tag, "a")) {
                        if (val_len > 0 && val_len < (int)sizeof(anchor_url)) {
                            for (int url_index = 0; url_index < val_len; url_index++) anchor_url[url_index] = response[vs + url_index];
                            anchor_url[val_len] = 0;
                            anchor_start = output;
                            anchor_active = 1;
                        }
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "action") &&
                               text_equal_nocase(tag, "form")) {
                        if (val_len > 0 && val_len < (int)sizeof(action_value)) {
                            for (int k = 0; k < val_len; k++) action_value[k] = response[vs + k];
                            action_value[val_len] = 0; action_length = val_len;
                        }
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "method") &&
                               text_equal_nocase(tag, "form")) {
                        if (val_len > 0 && val_len < (int)sizeof(method_value)) {
                            for (int k = 0; k < val_len; k++) method_value[k] = (char)lower_char(response[vs + k]);
                            method_value[val_len] = 0; method_length = val_len;
                        }
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "type") &&
                               (text_equal_nocase(tag, "input") || text_equal_nocase(tag, "button"))) {
                        if (val_len > 0 && val_len < (int)sizeof(type_value)) {
                            for (int k = 0; k < val_len; k++) type_value[k] = (char)lower_char(response[vs + k]);
                            type_value[val_len] = 0;
                        }
                    } else if (slice_match_nocase(response, attribute_start, attribute_length, "value") &&
                               (text_equal_nocase(tag, "input") || text_equal_nocase(tag, "button"))) {
                        if (val_len > 0 && val_len < (int)sizeof(val_value)) {
                            for (int k = 0; k < val_len; k++) val_value[k] = response[vs + k];
                            val_value[val_len] = 0; val_length = val_len;
                        }
                    }
                }
                if (id_start >= 0 && id_length > 0 && id_length < 64) {
                    char id_buf[64];
                    for (int k = 0; k < id_length; k++) id_buf[k] = response[id_start + k];
                    id_buf[id_length] = 0;
                    js_note_element(tag, id_buf, output);
                }
                if (text_equal_nocase(tag, "form") && !closing) {
                    if (action_length > 0) copy_text(current_form_action, sizeof(current_form_action), action_value);
                    else current_form_action[0] = 0;
                    current_form_method_get = (method_length > 0 && text_equal_nocase(method_value, "post")) ? 0 : 1;
                }
                if (text_equal_nocase(tag, "input") || text_equal_nocase(tag, "button")) {
                    uint8_t is_submit = text_equal_nocase(type_value, "submit") || text_equal_nocase(type_value, "button") ||
                                        text_equal_nocase(tag, "button");
                    if (is_submit) {
                        int btn_width = val_length > 0 ? val_length + 4 : 10;
                        if (btn_width > 30) btn_width = 30;
                        create_field(1, val_value, btn_width, action_value, current_form_method_get, output, (uint32_t)btn_width);
                        append_page_character(FIELD_MARKER, &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                        for (int w = 1; w < btn_width; w++) {
                            if (output + 1 < BROWSER_PAGE_MAX) { page[output] = ' '; page_colors[output] = current_style.color; page_backgrounds[output] = current_style.background; page_bold[output] = current_style.bold; page_rozmiar[output] = current_style.rozmiar; output++; }
                        }
                    } else {
                        int txt_width = 20;
                        create_field(0, val_value, txt_width, action_value, current_form_method_get, output, (uint32_t)txt_width);
                        append_page_character(FIELD_MARKER, &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
                        for (int w = 1; w < txt_width; w++) {
                            if (output + 1 < BROWSER_PAGE_MAX) { page[output] = ' '; page_colors[output] = current_style.color; page_backgrounds[output] = current_style.background; page_bold[output] = current_style.bold; page_rozmiar[output] = current_style.rozmiar; output++; }
                        }
                    }
                }
                for (uint32_t rule_index = 0; rule_index < css_rule_count; rule_index++) {
                    struct browser_css_rule *rule = &css_rules[rule_index];
                    int matches = (rule->selector_kind == 1 && class_start >= 0 &&
                                   slice_contains_word_nocase(response, class_start, class_length, rule->selector)) ||
                                  (rule->selector_kind == 2 && id_start >= 0 &&
                                   slice_equal_nocase(response, id_start, id_length, rule->selector));
                    if (!matches) continue;
                    if (rule->has_color) current_style.color = rule->style.color;
                    if (rule->style.has_background) {
                        current_style.background = rule->style.background;
                        current_style.has_background = 1;
                    }
                }
                {
                    char class_buf[32] = {0};
                    char id_buf[64] = {0};
                    if (class_start >= 0 && class_length > 0 && class_length < 32) {
                        for (int k = 0; k < class_length; k++) class_buf[k] = response[class_start + k];
                        class_buf[class_length] = 0;
                    }
                    if (id_start >= 0 && id_length > 0 && id_length < 64) {
                        for (int k = 0; k < id_length; k++) id_buf[k] = response[id_start + k];
                        id_buf[id_length] = 0;
                    }
                    struct browser_style cs;
                    css_compute(tag, class_start >= 0 ? class_buf : 0, id_start >= 0 ? id_buf : 0, "", &cs);
                    if (cs.has_border_color) { current_style.border_color = cs.border_color; current_style.has_border_color = 1; }
                    if (cs.has_border_width) { current_style.border_width = cs.border_width; current_style.has_border_width = 1; }
                    if (cs.has_padding) { current_style.padding = cs.padding; current_style.has_padding = 1; }
                    if (cs.has_margin) { current_style.margin = cs.margin; current_style.has_margin = 1; }
                    if (cs.has_width) { current_style.width = cs.width; current_style.has_width = 1; }
                    if (cs.has_cursor) { copy_text(current_style.cursor, sizeof(current_style.cursor), cs.cursor); current_style.has_cursor = 1; }
                    current_style.text_align = cs.text_align;
                    current_style.display = cs.display;
                }
                if (text_equal_nocase(tag, "body") && current_style.has_background)
                    page_background = current_style.background;
            }
            i = tag_end;
            continue;
        }
        if (in_script || in_style) continue;
        if (c == '\r') continue;
        if (c == '&') {
            if (starts_with(&response[i], "&nbsp;")) { append_page_character(' ', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar); i += 5; continue; }
            if (starts_with(&response[i], "&amp;")) { append_page_character('&', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar); i += 4; continue; }
            if (starts_with(&response[i], "&lt;")) { append_page_character('<', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar); i += 3; continue; }
            if (starts_with(&response[i], "&gt;")) { append_page_character('>', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar); i += 3; continue; }
            if (starts_with(&response[i], "&quot;")) { append_page_character('"', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar); i += 5; continue; }
        }
        if (c == '\n') {
            if (output > 0 && page[output - 1] != '\n') append_page_character('\n', &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
        } else if ((unsigned char)c >= 32) {
            append_page_character(c, &output, current_style.color, current_style.background, current_style.bold, current_style.rozmiar);
        }
    }
    while (output > 0 && page[output - 1] == '\n') output--;
    page[output] = 0;
}

static int copy_resolved_link(const char *link_url)
{
    const char *base;
    uint32_t output = 0;
    uint32_t host_length = 0;

    if (starts_with(link_url, "https://")) {
        copy_text(address, sizeof(address), link_url);
        return 1;
    }
    if (starts_with(link_url, "http://")) {
        copy_text(address, sizeof(address), link_url);
        return 1;
    }

    base = address;
    browser_use_https = starts_with(base, "https://");
    if (browser_use_https) base += 8;
    else if (starts_with(base, "http://")) base += 7;
    while (base[host_length] && base[host_length] != '/' && base[host_length] != ':' && host_length < 96)
        host_length++;
    if (!host_length || host_length + 10 >= sizeof(address)) return 0;
    copy_text(address, sizeof(address), browser_use_https ? "https://" : "http://");
    output = browser_use_https ? 8 : 7;
    for (uint32_t i = 0; i < host_length; i++) address[output++] = base[i];
    if (link_url[0] == '/') {
        while (*link_url && output + 1 < sizeof(address)) address[output++] = *link_url++;
    } else {
        uint32_t base_length = 0;
        while (base[base_length] && base[base_length] != '#' && base_length < 120) base_length++;
        while (base_length && base[base_length - 1] != '/') base_length--;
        for (uint32_t i = host_length; i < base_length && output + 1 < sizeof(address); i++)
            address[output++] = base[i];
        if (output == (browser_use_https ? 8 : 7) + host_length) address[output++] = '/';
        while (*link_url && output + 1 < sizeof(address)) address[output++] = *link_url++;
    }
    address[output] = 0;
    return 1;
}

static int extract_host_path(char *host, uint32_t host_capacity, char *path, uint32_t path_capacity)
{
    const char *cursor = address;
    uint32_t host_length = 0;
    uint32_t path_length = 0;
    browser_use_https = 0;
    if (starts_with(cursor, "http://")) { cursor += 7; }
    else if (starts_with(cursor, "https://")) { cursor += 8; browser_use_https = 1; }
    else return 0;

    while (cursor[host_length] && cursor[host_length] != '/' && cursor[host_length] != ':' &&
           host_length + 1 < host_capacity) {
        host[host_length] = cursor[host_length];
        host_length++;
    }
    host[host_length] = 0;
    if (cursor[host_length] == ':') return -3;
    if (!host_length) return 0;
    cursor += host_length;
    if (!*cursor) cursor = "/";
    while (cursor[path_length] && path_length + 1 < path_capacity) {
        path[path_length] = cursor[path_length];
        path_length++;
    }
    path[path_length] = 0;
    return 1;
}

static int response_location_is_https(void)
{
    for (int i = 0; response[i] && i + 9 < BROWSER_RESPONSE_MAX; i++) {
        if ((response[i] == 'L' || response[i] == 'l') &&
            starts_with(&response[i], "Location: https://")) return 1;
    }
    return 0;
}

static void browser_fetch(void)
{
    char host[96];
    char path[256];
    int parsed = extract_host_path(host, sizeof(host), path, sizeof(path));
    int length;
    if (parsed == -3) { set_status("Adres z portem nieobslugiwany"); page[0] = 0; okno_narysuj(&browser_window); return; }
    if (parsed == 0) { set_status("Nieprawidlowy adres"); okno_narysuj(&browser_window); return; }
    if (!net_ready()) { set_status("Brak sieci: sterownik/DHCP niedostepne"); okno_narysuj(&browser_window); return; }

    set_status("Laczenie... czekaj");
    okno_narysuj(&browser_window);

    if (browser_use_https) {
        length = tls13_https_get(host, path, response, sizeof(response));
    } else {
        length = net_http_get(host, path, response, sizeof(response));
    }
    if (length < 0) {
        if (browser_use_https) {
            if (length == -1) set_status("TLS: polaczenie TCP nieudane");
            else if (length == -2) set_status("TLS: blad wysylania ClientHello");
            else if (length == -3) set_status("TLS: blad odczytu ServerHello");
            else if (length == -4) set_status("TLS: blad parsowania ServerHello");
            else if (length == -5) set_status("TLS: blad wyprowadzania kluczy handshake");
            else if (length == -6) set_status("TLS: blad odczytu wiadomosci handshake");
            else if (length == -7) set_status("TLS: blad deszyfrowania handshake");
            else if (length == -8) set_status("TLS: blad wyprowadzania kluczy aplikacji");
            else if (length == -9) set_status("TLS: blad wysylania Finished");
            else if (length == -10) set_status("TLS: zadanie HTTP zbyt dlugie");
            else if (length == -11) set_status("TLS: blad wysylania zadania HTTP");
            else set_status("TLS: nieznany blad");
        } else {
            if (length == -2) set_status("Blad DNS");
            else if (length == -3) set_status("Nie mozna znalezc bramy ARP");
            else if (length == -4) set_status("Polaczenie TCP nieudane");
            else set_status("Blad pobierania strony");
        }
        page[0] = 0;
    } else if (response_location_is_https() && !browser_use_https) {
        set_status("Strona przekierowuje do HTTPS");
        page_from_response(length);
    } else if (starts_with(response, "HTTP/1.0 301") || starts_with(response, "HTTP/1.1 301") ||
               starts_with(response, "HTTP/1.0 302") || starts_with(response, "HTTP/1.1 302")) {
        set_status("Przekierowanie HTTP; otworz nowy adres recznie");
        page_from_response(length);
    } else {
        set_status("Pobrano odpowiedz");
        page_from_response(length);
    }
    okno_narysuj(&browser_window);
}

static void browser_submit_form(struct browser_field *f)
{
    char url[512];
    uint32_t u = 0;
    int first = 1;
    const char *action = f->action[0] ? f->action : address;
    char resolved[256];

    if (starts_with(action, "http://") || starts_with(action, "https://")) {
        copy_text(resolved, sizeof(resolved), action);
    } else if (action[0] == '/') {
        const char *base = address;
        uint32_t host_length = 0;
        browser_use_https = starts_with(base, "https://");
        if (browser_use_https) base += 8;
        else if (starts_with(base, "http://")) base += 7;
        while (base[host_length] && base[host_length] != '/' && base[host_length] != ':' && host_length < 96) host_length++;
        copy_text(resolved, sizeof(resolved), browser_use_https ? "https://" : "http://");
        u = browser_use_https ? 8 : 7;
        for (uint32_t i = 0; i < host_length; i++) resolved[u++] = base[i];
        resolved[u] = 0;
        copy_text(resolved + u, sizeof(resolved) - u, action);
    } else {
        copy_text(resolved, sizeof(resolved), address);
    }

    if (f->method_get) {
        uint32_t rlen = 0;
        while (resolved[rlen]) rlen++;
        if (rlen + 1 < sizeof(url)) { url[rlen++] = '?'; url[rlen] = 0; }
        for (uint32_t fi = 0; fi < field_count; fi++) {
            struct browser_field *ff = &fields[fi];
            if (ff->type != 0) continue;
            char encoded[256];
            uint32_t e = 0;
            for (uint32_t k = 0; ff->value[k] && e + 1 < sizeof(encoded); k++) {
                char ch = ff->value[k];
                if (ch == ' ') encoded[e++] = '+';
                else encoded[e++] = ch;
            }
            encoded[e] = 0;
            if (!first && rlen + 1 < sizeof(url)) url[rlen++] = '&';
            first = 0;
            const char *name = "pole";
            uint32_t nlen = 0;
            while (name[nlen] && rlen + 1 < sizeof(url)) url[rlen++] = name[nlen++];
            if (rlen + 1 < sizeof(url)) url[rlen++] = '=';
            uint32_t ellen = 0;
            while (encoded[ellen] && rlen + 1 < sizeof(url)) url[rlen++] = encoded[ellen++];
            url[rlen] = 0;
        }
        copy_text(address, sizeof(address), url);
    } else {
        copy_text(address, sizeof(address), resolved);
    }
    address_focused = 0;
    browser_fetch();
}

static void browser_draw(int x, int y, int width, int height)
{
    int line = 0;
    int char_x = 0;
    (void)width;
    rysuj_prostokat(x, y, 610, 24, address_focused ? 0x00FFF4CC : 0x00FFFFFF);
    rysuj_tekst_chicago(x + 5, y + 8, address, 0x00000000);
    rysuj_prostokat(x + 620, y, 72, 24, 0x00334155);
    rysuj_tekst_chicago(x + 636, y + 8, "IDZ", 0x00FFFFFF);
    rysuj_tekst_chicago(x, y + 30, status, 0x00000000);
    rysuj_prostokat(x, y + 48, width, height - 48, page_background);
    for (int i = 0; page[i]; i++) {
        char c = page[i];
        if (c == '\n' || char_x >= 84 * 8) {
            line++;
            char_x = 0;
            if (c == '\n') continue;
        }
        if (line < page_scroll_line) {
            if (c == FIELD_MARKER) char_x += 160;
            else if (c == BULLET_MARKER) char_x += 8;
            else char_x += znak_szerokosc(page_rozmiar[i]);
            continue;
        }
        if (y + 62 + (line - page_scroll_line) * 8 >= y + height - 8) break;
        int character_x = x + 4 + char_x;
        int character_y = y + 56 + (line - page_scroll_line) * 8;
        if (c == FIELD_MARKER) {
            struct browser_field *f = (struct browser_field *)0;
            for (uint32_t fi = 0; fi < field_count; fi++) {
                if (fields[fi].page_start == (uint32_t)i) { f = &fields[fi]; break; }
            }
            if (f) {
                int fw = f->page_width > 0 ? (int)f->page_width : 20;
                if (fw > 40) fw = 40;
                int box_x = character_x - 2;
                int box_y = character_y - 2;
                int box_w = fw * 8 + 4;
                int box_h = 10;
                rysuj_prostokat(box_x, box_y, box_w, box_h, f->focused ? 0x003B82F6 : 0x00FFFFFF);
                rysuj_prostokat(box_x + 1, box_y + 1, box_w - 2, box_h - 2, 0x00000000);
                rysuj_prostokat(box_x + 2, box_y + 2, box_w - 4, box_h - 4, 0x00FFFFFF);
                rysuj_tekst_chicago(box_x + 4, box_y + 2, f->value, 0x00000000);
                if (f->focused) {
                    int cx = box_x + 4 + f->cursor * 8;
                    if (cx > box_x + box_w - 8) cx = box_x + box_w - 8;
                    rysuj_prostokat(cx, box_y + 3, 2, 6, 0x00000000);
                }
                f->x = box_x; f->y = box_y; f->width = box_w; f->height = box_h;
                char_x += fw * 8;
                continue;
            }
            char_x += 8;
            continue;
        }
        if (c == BULLET_MARKER) {
            rysuj_prostokat(character_x, character_y + 3, 2, 2, page_colors[i]);
            char_x += 8;
            continue;
        }
        if (page[i] == '\x01') {
            rysuj_prostokat(x + 4, y + 59 + (line - page_scroll_line) * 8, 84 * 8, 1, page_colors[i]);
            char_x = 84 * 8;
            continue;
        }
        if (page_backgrounds[i] != page_background)
            rysuj_prostokat(character_x, character_y, znak_szerokosc(page_rozmiar[i]), 8, page_backgrounds[i]);
        rysuj_znak_styl(character_x, character_y, c, page_colors[i], page_rozmiar[i], page_bold[i] ? 1 : 0);
        char_x += znak_szerokosc(page_rozmiar[i]);
    }
}

static int browser_field_at(int x, int y)
{
    for (uint32_t fi = 0; fi < field_count; fi++) {
        struct browser_field *f = &fields[fi];
        if (x >= f->x && x < f->x + f->width && y >= f->y && y < f->y + f->height) return (int)fi;
    }
    return -1;
}

static int browser_link_at(int x, int y)
{
    int line = 0;
    int char_x = 0;
    int target_line = page_scroll_line + (y - 86) / 8;
    int target_px = x - 12;
    int page_index = -1;

    if (x < 12 || y < 86 || target_line < 0 || target_px < 0) return -1;
    for (int i = 0; page[i]; i++) {
        if (page[i] == '\n' || char_x >= 84 * 8) {
            line++;
            char_x = 0;
            if (page[i] == '\n') continue;
        }
        if (line == target_line && page[i] != FIELD_MARKER && page[i] != '\x01') {
            int w = znak_szerokosc(page_rozmiar[i]);
            if (target_px < char_x + w) {
                page_index = i;
                break;
            }
        }
        if (page[i] == '\x01') { char_x = 84 * 8; continue; }
        if (page[i] == FIELD_MARKER) char_x += 160;
        else if (page[i] == BULLET_MARKER) char_x += 8;
        else char_x += znak_szerokosc(page_rozmiar[i]);
    }
    if (page_index < 0) return -1;
    for (uint32_t link = 0; link < link_count; link++)
        if (page_index >= links[link].start && page_index < links[link].end) return (int)link;
    return -1;
}

void browser_scroll_wheel(int delta)
{
    int total_lines = 0;
    int char_x = 0;
    int visible_lines = (BROWSER_HEIGHT - 64) / 8;
    if (!browser_opened || !browser_window.otwarte || !okno_czy_aktywne(&browser_window) || !delta) return;
    for (int i = 0; page[i]; i++) {
        if (page[i] == '\n' || char_x >= 84 * 8) {
            total_lines++;
            char_x = 0;
            if (page[i] == '\n') continue;
        }
        if (page[i] == '\x01') { char_x = 84 * 8; continue; }
        if (page[i] == FIELD_MARKER) char_x += 160;
        else if (page[i] == BULLET_MARKER) char_x += 8;
        else char_x += znak_szerokosc(page_rozmiar[i]);
    }
    if (char_x) total_lines++;
    page_scroll_line -= delta * 3;
    if (page_scroll_line < 0) page_scroll_line = 0;
    if (page_scroll_line > total_lines - visible_lines)
        page_scroll_line = total_lines > visible_lines ? total_lines - visible_lines : 0;
    okno_narysuj(&browser_window);
}

static void browser_click(int x, int y)
{
    if (y >= 30 && y < 58) {
        if (x >= 628 && x < 700) {
            address_focused = 1;
            focused_field = -1;
            for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
            browser_fetch();
        } else if (x >= 8 && x < 618) {
            address_focused = 1;
            focused_field = -1;
            for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
        }
        okno_narysuj(&browser_window);
        return;
    }

    int field = browser_field_at(x, y);
    if (field >= 0) {
        address_focused = 0;
        for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
        focused_field = field;
        fields[field].focused = 1;
        okno_narysuj(&browser_window);
        {
            const char *jid = js_find_id_by_page(fields[field].page_start);
            if (jid) js_dispatch_click(jid);
        }
        return;
    }

    int link = browser_link_at(x, y);
    if (link >= 0) {
        const char *jid = js_find_id_by_page(links[link].start);
        if (jid) js_dispatch_click(jid);
        if (!copy_resolved_link(links[link].url)) {
            set_status("Nieprawidlowy adres linku");
            okno_narysuj(&browser_window);
            return;
        }
        address_focused = 0;
        focused_field = -1;
        for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
        browser_fetch();
    }
}

void browser_open(void)
{
    if (browser_opened && browser_window.otwarte) {
        okno_ustaw_aktywne(&browser_window);
        okno_narysuj(&browser_window);
        return;
    }
    css_init();
    js_init();
    browser_opened = 1;
    address_focused = 1;
    focused_field = -1;
    for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
    if (!net_ready()) set_status("Brak sieci: sterownik/DHCP niedostepne");
    else set_status("Wpisz adres i nacisnij IDZ");
    okno_stworz(&browser_window, 120, 80, BROWSER_WIDTH, BROWSER_HEIGHT,
                "PRZEGLADARKA HTTP/HTTPS", browser_draw);
    okno_ustaw_obsluge_klikniecia(&browser_window, browser_click);
}

void browser_handle_keyboard(void)
{
    char character;
    if (!browser_opened || !browser_window.otwarte || !okno_czy_aktywne(&browser_window) ||
        !keyboard_try_getchar(&character)) return;

    if (focused_field >= 0) {
        struct browser_field *f = &fields[focused_field];
        if (character == '\b') {
            if (f->cursor > 0) {
                f->cursor--;
                f->value[f->cursor] = 0;
            }
        } else if (character == '\n' || character == '\r') {
            browser_submit_form(f);
            return;
        } else if (character >= 32 && character <= 126 && f->cursor + 1 < (int)sizeof(f->value)) {
            f->value[f->cursor++] = character;
            f->value[f->cursor] = 0;
        }
        okno_narysuj(&browser_window);
        return;
    }

    if (!address_focused) return;
    uint32_t length = 0;
    while (address[length]) length++;
    if (character == '\b') {
        if (length) address[length - 1] = 0;
    } else if (character == '\n' || character == '\r') {
        address_focused = 0;
        focused_field = -1;
        for (uint32_t fi = 0; fi < field_count; fi++) fields[fi].focused = 0;
        browser_fetch();
        return;
    } else if (character >= 32 && character <= 126 && length + 1 < sizeof(address)) {
        address[length] = character;
        address[length + 1] = 0;
    }
    okno_narysuj(&browser_window);
}
