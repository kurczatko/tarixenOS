#ifndef TLS_CRYPTO_H
#define TLS_CRYPTO_H

#include "../include/printf/types.h"

struct sha256_context {
    uint32_t state[8];
    uint64_t total_bytes;
    uint8_t block[64];
    uint32_t block_used;
};

void sha256_init(struct sha256_context *context);
void sha256_update(struct sha256_context *context, const uint8_t *data, uint32_t length);
void sha256_final(struct sha256_context *context, uint8_t digest[32]);
void hmac_sha256(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[32]);
void tls12_prf_sha256(const uint8_t *secret, uint32_t secret_length,
                      const char *label, const uint8_t *seed, uint32_t seed_length,
                      uint8_t *output, uint32_t output_length);

void hkdf_extract_sha256(const uint8_t *salt, uint32_t salt_length,
                         const uint8_t *input, uint32_t input_length, uint8_t output[32]);
void hkdf_expand_sha256(const uint8_t secret[32], const uint8_t *info, uint32_t info_length,
                        uint8_t *output, uint32_t output_length);
void tls13_expand_label(const uint8_t secret[32], const char *label,
                        const uint8_t *context, uint32_t context_length,
                        uint8_t *output, uint32_t output_length);
int aes128_gcm_encrypt(const uint8_t key[16], const uint8_t nonce[12],
                       const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16]);
int aes128_gcm_decrypt(const uint8_t key[16], const uint8_t nonce[12],
                       const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16]);
void x25519_public_key(const uint8_t private_key[32], uint8_t public_key[32]);
int x25519_shared_secret(const uint8_t private_key[32], const uint8_t peer_key[32],
                         uint8_t shared_secret[32]);

#endif
