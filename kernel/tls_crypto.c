#include "tls_crypto.h"

static const uint32_t sha256_round_constants[64] = {
    0x428A2F98,0x71374491,0xB5C0FBCF,0xE9B5DBA5,0x3956C25B,0x59F111F1,0x923F82A4,0xAB1C5ED5,
    0xD807AA98,0x12835B01,0x243185BE,0x550C7DC3,0x72BE5D74,0x80DEB1FE,0x9BDC06A7,0xC19BF174,
    0xE49B69C1,0xEFBE4786,0x0FC19DC6,0x240CA1CC,0x2DE92C6F,0x4A7484AA,0x5CB0A9DC,0x76F988DA,
    0x983E5152,0xA831C66D,0xB00327C8,0xBF597FC7,0xC6E00BF3,0xD5A79147,0x06CA6351,0x14292967,
    0x27B70A85,0x2E1B2138,0x4D2C6DFC,0x53380D13,0x650A7354,0x766A0ABB,0x81C2C92E,0x92722C85,
    0xA2BFE8A1,0xA81A664B,0xC24B8B70,0xC76C51A3,0xD192E819,0xD6990624,0xF40E3585,0x106AA070,
    0x19A4C116,0x1E376C08,0x2748774C,0x34B0BCB5,0x391C0CB3,0x4ED8AA4A,0x5B9CCA4F,0x682E6FF3,
    0x748F82EE,0x78A5636F,0x84C87814,0x8CC70208,0x90BEFFFA,0xA4506CEB,0xBEF9A3F7,0xC67178F2
};

static uint32_t rotate_right(uint32_t value, uint32_t count)
{
    return (value >> count) | (value << (32 - count));
}

static uint32_t load_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

static void store_be32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 24);
    data[1] = (uint8_t)(value >> 16);
    data[2] = (uint8_t)(value >> 8);
    data[3] = (uint8_t)value;
}

static void store_be64(uint8_t *data, uint64_t value)
{
    for (uint32_t i = 0; i < 8; i++) data[7 - i] = (uint8_t)(value >> (i * 8));
}

static void clear_bytes(uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) data[i] = 0;
}

static void copy_bytes(uint8_t *destination, const uint8_t *source, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) destination[i] = source[i];
}

static void sha256_transform(struct sha256_context *context, const uint8_t block[64])
{
    uint32_t words[64];
    uint32_t a, b, c, d, e, f, g, h;
    for (uint32_t i = 0; i < 16; i++) words[i] = load_be32(block + i * 4);
    for (uint32_t i = 16; i < 64; i++) {
        uint32_t s0 = rotate_right(words[i - 15], 7) ^ rotate_right(words[i - 15], 18) ^ (words[i - 15] >> 3);
        uint32_t s1 = rotate_right(words[i - 2], 17) ^ rotate_right(words[i - 2], 19) ^ (words[i - 2] >> 10);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }
    a = context->state[0]; b = context->state[1]; c = context->state[2]; d = context->state[3];
    e = context->state[4]; f = context->state[5]; g = context->state[6]; h = context->state[7];
    for (uint32_t i = 0; i < 64; i++) {
        uint32_t sum1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25);
        uint32_t choice = (e & f) ^ (~e & g);
        uint32_t temp1 = h + sum1 + choice + sha256_round_constants[i] + words[i];
        uint32_t sum0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22);
        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = sum0 + majority;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    context->state[0] += a; context->state[1] += b; context->state[2] += c; context->state[3] += d;
    context->state[4] += e; context->state[5] += f; context->state[6] += g; context->state[7] += h;
}

void sha256_init(struct sha256_context *context)
{
    static const uint32_t initial[8] = {
        0x6A09E667,0xBB67AE85,0x3C6EF372,0xA54FF53A,
        0x510E527F,0x9B05688C,0x1F83D9AB,0x5BE0CD19
    };
    for (uint32_t i = 0; i < 8; i++) context->state[i] = initial[i];
    context->total_bytes = 0;
    context->block_used = 0;
}

void sha256_update(struct sha256_context *context, const uint8_t *data, uint32_t length)
{
    context->total_bytes += length;
    while (length) {
        uint32_t available = 64 - context->block_used;
        uint32_t take = length < available ? length : available;
        copy_bytes(context->block + context->block_used, data, take);
        context->block_used += take;
        data += take;
        length -= take;
        if (context->block_used == 64) {
            sha256_transform(context, context->block);
            context->block_used = 0;
        }
    }
}

void sha256_final(struct sha256_context *context, uint8_t digest[32])
{
    uint64_t bit_length = context->total_bytes * 8;
    context->block[context->block_used++] = 0x80;
    if (context->block_used > 56) {
        while (context->block_used < 64) context->block[context->block_used++] = 0;
        sha256_transform(context, context->block);
        context->block_used = 0;
    }
    while (context->block_used < 56) context->block[context->block_used++] = 0;
    store_be64(context->block + 56, bit_length);
    sha256_transform(context, context->block);
    for (uint32_t i = 0; i < 8; i++) store_be32(digest + i * 4, context->state[i]);
    clear_bytes((uint8_t *)context, sizeof(*context));
}

static void hmac_init(struct sha256_context *context, const uint8_t *key, uint32_t key_length,
                      uint8_t outer_pad[64])
{
    uint8_t normalized_key[64];
    uint8_t inner_pad[64];
    uint8_t hashed_key[32];
    clear_bytes(normalized_key, sizeof(normalized_key));
    if (key_length > 64) {
        struct sha256_context key_context;
        sha256_init(&key_context);
        sha256_update(&key_context, key, key_length);
        sha256_final(&key_context, hashed_key);
        copy_bytes(normalized_key, hashed_key, sizeof(hashed_key));
    } else if (key_length) {
        copy_bytes(normalized_key, key, key_length);
    }
    for (uint32_t i = 0; i < 64; i++) {
        inner_pad[i] = normalized_key[i] ^ 0x36;
        outer_pad[i] = normalized_key[i] ^ 0x5C;
    }
    sha256_init(context);
    sha256_update(context, inner_pad, sizeof(inner_pad));
    clear_bytes(normalized_key, sizeof(normalized_key));
    clear_bytes(inner_pad, sizeof(inner_pad));
    clear_bytes(hashed_key, sizeof(hashed_key));
}

static void hmac_finish(struct sha256_context *inner, const uint8_t outer_pad[64], uint8_t digest[32])
{
    uint8_t inner_digest[32];
    struct sha256_context outer;
    sha256_final(inner, inner_digest);
    sha256_init(&outer);
    sha256_update(&outer, outer_pad, 64);
    sha256_update(&outer, inner_digest, sizeof(inner_digest));
    sha256_final(&outer, digest);
    clear_bytes(inner_digest, sizeof(inner_digest));
}

void hmac_sha256(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[32])
{
    uint8_t outer_pad[64];
    struct sha256_context context;
    hmac_init(&context, key, key_length, outer_pad);
    sha256_update(&context, data, data_length);
    hmac_finish(&context, outer_pad, digest);
    clear_bytes(outer_pad, sizeof(outer_pad));
}

void tls12_prf_sha256(const uint8_t *secret, uint32_t secret_length,
                      const char *label, const uint8_t *seed, uint32_t seed_length,
                      uint8_t *output, uint32_t output_length)
{
    uint8_t combined_seed[256];
    uint8_t a_value[32];
    uint8_t hmac_input[288];
    uint8_t block[32];
    uint32_t label_length = 0;
    uint32_t produced = 0;
    while (label[label_length]) label_length++;
    if (label_length + seed_length > sizeof(combined_seed)) {
        clear_bytes(output, output_length);
        return;
    }
    copy_bytes(combined_seed, (const uint8_t *)label, label_length);
    copy_bytes(combined_seed + label_length, seed, seed_length);
    hmac_sha256(secret, secret_length, combined_seed, label_length + seed_length, a_value);
    while (produced < output_length) {
        copy_bytes(hmac_input, a_value, sizeof(a_value));
        copy_bytes(hmac_input + sizeof(a_value), combined_seed, label_length + seed_length);
        hmac_sha256(secret, secret_length, hmac_input, sizeof(a_value) + label_length + seed_length, block);
        uint32_t take = output_length - produced < sizeof(block) ? output_length - produced : sizeof(block);
        copy_bytes(output + produced, block, take);
        produced += take;
        hmac_sha256(secret, secret_length, a_value, sizeof(a_value), a_value);
    }
    clear_bytes(combined_seed, sizeof(combined_seed));
    clear_bytes(a_value, sizeof(a_value));
    clear_bytes(hmac_input, sizeof(hmac_input));
    clear_bytes(block, sizeof(block));
}

void hkdf_extract_sha256(const uint8_t *salt, uint32_t salt_length,
                         const uint8_t *input, uint32_t input_length, uint8_t output[32])
{
    uint8_t zero_salt[32];
    if (!salt) {
        clear_bytes(zero_salt, sizeof(zero_salt));
        hmac_sha256(zero_salt, sizeof(zero_salt), input, input_length, output);
        clear_bytes(zero_salt, sizeof(zero_salt));
    } else hmac_sha256(salt, salt_length, input, input_length, output);
}

void hkdf_expand_sha256(const uint8_t secret[32], const uint8_t *info, uint32_t info_length,
                        uint8_t *output, uint32_t output_length)
{
    uint8_t previous[32], input[288];
    uint32_t used = 0, previous_length = 0;
    uint8_t counter = 1;
    if (info_length > 255 || output_length > 255 * 32) return;
    while (used < output_length) {
        uint32_t p = 0;
        copy_bytes(input + p, previous, previous_length); p += previous_length;
        copy_bytes(input + p, info, info_length); p += info_length;
        input[p++] = counter++;
        hmac_sha256(secret, 32, input, p, previous);
        uint32_t take = output_length - used < 32 ? output_length - used : 32;
        copy_bytes(output + used, previous, take);
        used += take;
        previous_length = 32;
    }
    clear_bytes(previous, sizeof(previous));
    clear_bytes(input, sizeof(input));
}

void tls13_expand_label(const uint8_t secret[32], const char *label,
                        const uint8_t *context, uint32_t context_length,
                        uint8_t *output, uint32_t output_length)
{
    uint8_t info[256];
    uint32_t label_length = 0, p = 0;
    while (label[label_length]) label_length++;
    if (label_length + 6 > 255 || context_length > 255 || output_length > 65535) return;
    info[p++] = (uint8_t)(output_length >> 8); info[p++] = (uint8_t)output_length;
    info[p++] = (uint8_t)(label_length + 6);
    copy_bytes(info + p, (const uint8_t *)"tls13 ", 6); p += 6;
    copy_bytes(info + p, (const uint8_t *)label, label_length); p += label_length;
    info[p++] = (uint8_t)context_length;
    if (context_length) { copy_bytes(info + p, context, context_length); p += context_length; }
    hkdf_expand_sha256(secret, info, p, output, output_length);
    clear_bytes(info, sizeof(info));
}

static const uint8_t aes_sbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16 };

static uint8_t aes_xtime(uint8_t value) { return (uint8_t)((value << 1) ^ ((value >> 7) * 0x1b)); }
static void aes128_block(const uint8_t key[16], const uint8_t input[16], uint8_t output[16])
{
    uint8_t state[16], round_key[16], temporary[16];
    static const uint8_t rcon[10] = {1,2,4,8,16,32,64,128,27,54};
    copy_bytes(state, input, 16); copy_bytes(round_key, key, 16);
    for (uint32_t i = 0; i < 16; i++) state[i] ^= round_key[i];
    for (uint32_t round = 0; round < 10; round++) {
        uint8_t last[4] = {round_key[13], round_key[14], round_key[15], round_key[12]};
        for (uint32_t i = 0; i < 4; i++) last[i] = aes_sbox[last[i]];
        last[0] ^= rcon[round];
        for (uint32_t i = 0; i < 4; i++) round_key[i] ^= last[i];
        for (uint32_t i = 4; i < 16; i++) round_key[i] ^= round_key[i - 4];
        for (uint32_t i = 0; i < 16; i++) state[i] = aes_sbox[state[i]];
        for (uint32_t row = 0; row < 4; row++) for (uint32_t column = 0; column < 4; column++)
            temporary[row + 4 * column] = state[row + 4 * ((column + row) & 3)];
        copy_bytes(state, temporary, 16);
        if (round != 9) for (uint32_t column = 0; column < 4; column++) {
            uint8_t *s = state + 4 * column, a = s[0], b = s[1], c = s[2], d = s[3];
            s[0] = aes_xtime(a) ^ (aes_xtime(b) ^ b) ^ c ^ d;
            s[1] = a ^ aes_xtime(b) ^ (aes_xtime(c) ^ c) ^ d;
            s[2] = a ^ b ^ aes_xtime(c) ^ (aes_xtime(d) ^ d);
            s[3] = (aes_xtime(a) ^ a) ^ b ^ c ^ aes_xtime(d);
        }
        for (uint32_t i = 0; i < 16; i++) state[i] ^= round_key[i];
    }
    copy_bytes(output, state, 16); clear_bytes(state, 16); clear_bytes(round_key, 16); clear_bytes(temporary, 16);
}

static void xor_block(uint8_t left[16], const uint8_t right[16]) { for (uint32_t i = 0; i < 16; i++) left[i] ^= right[i]; }
static void ghash_multiply(uint8_t value[16], const uint8_t hash[16])
{
    uint8_t result[16], factor[16]; clear_bytes(result, 16); copy_bytes(factor, hash, 16);
    for (uint32_t bit = 0; bit < 128; bit++) {
        uint8_t selected = (value[bit / 8] >> (7 - (bit & 7))) & 1;
        if (selected) xor_block(result, factor);
        uint8_t low = factor[15] & 1;
        for (int i = 15; i > 0; i--) factor[i] = (uint8_t)((factor[i] >> 1) | (factor[i - 1] << 7));
        factor[0] >>= 1; if (low) factor[0] ^= 0xe1;
    }
    copy_bytes(value, result, 16); clear_bytes(result, 16); clear_bytes(factor, 16);
}

static void ghash_update(uint8_t state[16], const uint8_t hash[16], const uint8_t *data, uint32_t length)
{
    uint8_t block[16];
    while (length) { uint32_t take = length < 16 ? length : 16; clear_bytes(block, 16); copy_bytes(block, data, take); xor_block(state, block); ghash_multiply(state, hash); data += take; length -= take; }
    clear_bytes(block, 16);
}

static void gcm_counter(uint8_t counter[16]) { for (int i = 15; i >= 12; i--) if (++counter[i]) break; }
static int constant_equal(const uint8_t *left, const uint8_t *right, uint32_t length) { uint8_t difference = 0; for (uint32_t i = 0; i < length; i++) difference |= left[i] ^ right[i]; return difference == 0; }
static void gcm_tag(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                    const uint8_t *ciphertext, uint32_t length, uint8_t tag[16])
{
    uint8_t hash[16], state[16], initial[16], lengths[16]; clear_bytes(hash, 16); clear_bytes(state, 16); clear_bytes(initial, 16);
    aes128_block(key, hash, hash); copy_bytes(initial, nonce, 12); initial[15] = 1;
    ghash_update(state, hash, additional, additional_length); ghash_update(state, hash, ciphertext, length);
    clear_bytes(lengths, 16); store_be64(lengths, (uint64_t)additional_length * 8); store_be64(lengths + 8, (uint64_t)length * 8);
    xor_block(state, lengths); ghash_multiply(state, hash); aes128_block(key, initial, tag); xor_block(tag, state);
    clear_bytes(hash, 16); clear_bytes(state, 16); clear_bytes(initial, 16); clear_bytes(lengths, 16);
}

static void gcm_crypt(const uint8_t key[16], const uint8_t nonce[12], uint8_t *data, uint32_t length)
{
    uint8_t counter[16], stream[16]; clear_bytes(counter, 16); copy_bytes(counter, nonce, 12); counter[15] = 1;
    while (length) { uint32_t take = length < 16 ? length : 16; gcm_counter(counter); aes128_block(key, counter, stream); for (uint32_t i = 0; i < take; i++) data[i] ^= stream[i]; data += take; length -= take; }
    clear_bytes(counter, 16); clear_bytes(stream, 16);
}

int aes128_gcm_encrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16]) { gcm_crypt(key, nonce, data, length); gcm_tag(key, nonce, additional, additional_length, data, length, tag); return 1; }
int aes128_gcm_decrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16]) { uint8_t expected[16]; gcm_tag(key, nonce, additional, additional_length, data, length, expected); if (!constant_equal(expected, tag, 16)) { clear_bytes(expected, 16); return 0; } clear_bytes(expected, 16); gcm_crypt(key, nonce, data, length); return 1; }

typedef long long curve_limb;
typedef curve_limb curve_field[16];
static void curve_car(curve_field value)
{
    for (int i = 0; i < 16; i++) {
        value[i] += 65536;
        curve_limb carry = value[i];
        value[(i + 1) & 15] += (carry >> 16) - 1 + (i == 15 ? 37 * ((carry >> 16) - 1) : 0);
        value[i] -= (carry >> 16) << 16;
    }
}
static void curve_select(curve_field left, curve_field right, int select)
{
    curve_limb mask = ~(curve_limb)(select - 1);
    for (int i = 0; i < 16; i++) { curve_limb changed = mask & (left[i] ^ right[i]); left[i] ^= changed; right[i] ^= changed; }
}
static void curve_pack(uint8_t output[32], curve_field input)
{
    curve_field copy, candidate;
    for (int i = 0; i < 16; i++) copy[i] = input[i];
    curve_car(copy); curve_car(copy); curve_car(copy);
    for (int pass = 0; pass < 2; pass++) {
        candidate[0] = copy[0] - 0xffed;
        for (int i = 1; i < 15; i++) { candidate[i] = copy[i] - 0xffff - ((candidate[i - 1] >> 16) & 1); candidate[i - 1] &= 0xffff; }
        candidate[15] = copy[15] - 0x7fff - ((candidate[14] >> 16) & 1);
        int select = (int)((candidate[15] >> 16) & 1);
        candidate[14] &= 0xffff;
        curve_select(copy, candidate, 1 - select);
    }
    for (int i = 0; i < 16; i++) { output[2 * i] = (uint8_t)copy[i]; output[2 * i + 1] = (uint8_t)(copy[i] >> 8); }
    clear_bytes((uint8_t *)copy, sizeof(copy)); clear_bytes((uint8_t *)candidate, sizeof(candidate));
}
static void curve_unpack(curve_field output, const uint8_t input[32])
{
    for (int i = 0; i < 16; i++) output[i] = input[2 * i] + ((curve_limb)input[2 * i + 1] << 8);
    output[15] &= 0x7fff;
}
static void curve_add(curve_field output, const curve_field left, const curve_field right) { for (int i = 0; i < 16; i++) output[i] = left[i] + right[i]; }
static void curve_sub(curve_field output, const curve_field left, const curve_field right) { for (int i = 0; i < 16; i++) output[i] = left[i] - right[i]; }
static void curve_mul(curve_field output, const curve_field left, const curve_field right)
{
    curve_limb temporary[31];
    for (int i = 0; i < 31; i++) temporary[i] = 0;
    for (int i = 0; i < 16; i++) for (int j = 0; j < 16; j++) temporary[i + j] += left[i] * right[j];
    for (int i = 0; i < 15; i++) temporary[i] += 38 * temporary[i + 16];
    for (int i = 0; i < 16; i++) output[i] = temporary[i];
    curve_car(output); curve_car(output); clear_bytes((uint8_t *)temporary, sizeof(temporary));
}
static void curve_square(curve_field output, const curve_field input) { curve_mul(output, input, input); }
static void curve_inverse(curve_field output, const curve_field input)
{
    curve_field value; for (int i = 0; i < 16; i++) value[i] = input[i];
    for (int bit = 253; bit >= 0; bit--) { curve_square(value, value); if (bit != 2 && bit != 4) curve_mul(value, value, input); }
    for (int i = 0; i < 16; i++) output[i] = value[i];
    clear_bytes((uint8_t *)value, sizeof(value));
}
static void curve_scalar_mult(uint8_t output[32], const uint8_t scalar_input[32], const uint8_t point_input[32])
{
    uint8_t scalar[32]; curve_field point, a, b, c, d, e, f;
    copy_bytes(scalar, scalar_input, 32); scalar[0] &= 248; scalar[31] &= 127; scalar[31] |= 64;
    curve_unpack(point, point_input);
    for (int i = 0; i < 16; i++) { a[i] = d[i] = 0; b[i] = point[i]; c[i] = 0; }
    a[0] = d[0] = 1;
    for (int bit = 254; bit >= 0; bit--) {
        int selected = (scalar[bit >> 3] >> (bit & 7)) & 1;
        curve_select(a, b, selected); curve_select(c, d, selected);
        curve_add(e, a, c); curve_sub(a, a, c); curve_add(c, b, d); curve_sub(b, b, d);
        curve_square(d, e); curve_square(f, a); curve_mul(a, c, a); curve_mul(c, b, e);
        curve_add(e, a, c); curve_sub(a, a, c); curve_square(b, a); curve_sub(c, d, f);
        for (int i = 0; i < 16; i++) a[i] = c[i] * 121665;
        curve_add(a, a, d); curve_mul(c, c, a); curve_mul(a, d, f); curve_mul(d, b, point); curve_square(b, e);
        curve_select(a, b, selected); curve_select(c, d, selected);
    }
    curve_inverse(c, c); curve_mul(a, a, c); curve_pack(output, a);
    clear_bytes(scalar, sizeof(scalar)); clear_bytes((uint8_t *)point, sizeof(point)); clear_bytes((uint8_t *)a, sizeof(a));
    clear_bytes((uint8_t *)b, sizeof(b)); clear_bytes((uint8_t *)c, sizeof(c)); clear_bytes((uint8_t *)d, sizeof(d));
    clear_bytes((uint8_t *)e, sizeof(e)); clear_bytes((uint8_t *)f, sizeof(f));
}
void x25519_public_key(const uint8_t private_key[32], uint8_t public_key[32])
{
    static const uint8_t base_point[32] = {9}; curve_scalar_mult(public_key, private_key, base_point);
}
int x25519_shared_secret(const uint8_t private_key[32], const uint8_t peer_key[32], uint8_t shared_secret[32])
{
    uint8_t zero = 0; curve_scalar_mult(shared_secret, private_key, peer_key); for (int i = 0; i < 32; i++) zero |= shared_secret[i]; return zero != 0;
}
