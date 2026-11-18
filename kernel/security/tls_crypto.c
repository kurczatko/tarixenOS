/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/security/tls_crypto.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#define aes256_gcm_decrypt tlsx_aes256_gcm_decrypt
#define aes256_gcm_encrypt tlsx_aes256_gcm_encrypt
#define aes_encrypt_block tlsx_aes_encrypt_block
#define aes_gcm_init tlsx_aes_gcm_init
#define aes_gcm_open tlsx_aes_gcm_open
#define aes_gcm_seal tlsx_aes_gcm_seal
#define aes_set_key tlsx_aes_set_key
#define chacha20_poly1305_open tlsx_chacha20_poly1305_open
#define chacha20_poly1305_seal tlsx_chacha20_poly1305_seal
#define chacha20_xor tlsx_chacha20_xor
#define crypto_copy tlsx_crypto_copy
#define crypto_equal tlsx_crypto_equal
#define crypto_random_bytes tlsx_crypto_random_bytes
#define crypto_set_entropy_source tlsx_crypto_set_entropy_source
#define crypto_string_length tlsx_crypto_string_length
#define crypto_zero tlsx_crypto_zero
#define hash_block_size tlsx_hash_block_size
#define hash_digest tlsx_hash_digest
#define hash_digest_size tlsx_hash_digest_size
#define hash_final tlsx_hash_final
#define hash_init tlsx_hash_init
#define hash_snapshot tlsx_hash_snapshot
#define hash_update tlsx_hash_update
#define hkdf_expand tlsx_hkdf_expand
#define hkdf_extract tlsx_hkdf_extract
#define hmac_begin tlsx_hmac_begin
#define hmac_compute tlsx_hmac_compute
#define hmac_drbg_generate tlsx_hmac_drbg_generate
#define hmac_drbg_instantiate tlsx_hmac_drbg_instantiate
#define hmac_drbg_reseed tlsx_hmac_drbg_reseed
#define hmac_end tlsx_hmac_end
#define hmac_feed tlsx_hmac_feed
#define hmac_sha384 tlsx_hmac_sha384
#define poly1305_final tlsx_poly1305_final
#define poly1305_init tlsx_poly1305_init
#define poly1305_update tlsx_poly1305_update
#define sha256_digest tlsx_sha256_digest
#define sha384_init tlsx_sha384_init
#define sha512_final tlsx_sha512_final
#define sha512_init tlsx_sha512_init
#define sha512_update tlsx_sha512_update
#define tls12_prf tlsx_tls12_prf
#define tls13_derive_secret tlsx_tls13_derive_secret
#define tls13_expand_label_ex tlsx_tls13_expand_label_ex

#ifndef SHA256_DIGEST_SIZE
#define SHA256_DIGEST_SIZE 32
#endif
#ifndef SHA384_DIGEST_SIZE
#define SHA384_DIGEST_SIZE 48
#endif
#ifndef SHA512_DIGEST_SIZE
#define SHA512_DIGEST_SIZE 64
#endif
#define HASH_MAX_DIGEST_SIZE 64
#define HASH_MAX_BLOCK_SIZE 128
#define AES_MAX_ROUNDS 14
#define AES_BLOCK_SIZE 16
#define GCM_NONCE_SIZE 12
#define GCM_TAG_SIZE 16
#define CHACHA20_KEY_SIZE 32
#define CHACHA20_NONCE_SIZE 12
#define POLY1305_TAG_SIZE 16
#define X25519_KEY_SIZE 32

enum hash_algorithm {
    HASH_NONE = 0,
    HASH_SHA256 = 1,
    HASH_SHA384 = 2,
    HASH_SHA512 = 3
};

struct sha512_context {
    uint64_t state[8];
    uint64_t total_bytes;
    uint8_t block[128];
    uint32_t block_used;
    uint32_t digest_length;
};

struct hash_context {
    enum hash_algorithm algorithm;
    struct sha256_context sha256;
    struct sha512_context sha512;
};

struct hmac_context {
    struct hash_context inner;
    struct hash_context outer;
};

struct aes_context {
    uint8_t round_keys[16 * (AES_MAX_ROUNDS + 1)];
    uint32_t rounds;
};

struct aes_gcm_context {
    struct aes_context aes;
    uint8_t hash_key[16];
};

struct poly1305_context {
    uint32_t r[5];
    uint32_t h[5];
    uint32_t pad[4];
    uint32_t leftover;
    uint8_t buffer[16];
};

struct hmac_drbg_context {
    uint8_t key[32];
    uint8_t value[32];
    uint32_t generate_count;
    uint32_t seeded;
};

typedef int (*crypto_entropy_source)(uint8_t *output, uint32_t length);

void crypto_zero(void *data, uint32_t length);
void crypto_copy(void *destination, const void *source, uint32_t length);
int crypto_equal(const void *left, const void *right, uint32_t length);
uint32_t crypto_string_length(const char *text);

void sha256_init(struct sha256_context *context);
void sha256_update(struct sha256_context *context, const uint8_t *data, uint32_t length);
void sha256_final(struct sha256_context *context, uint8_t digest[32]);
void sha256_digest(const uint8_t *data, uint32_t length, uint8_t digest[32]);

void sha512_init(struct sha512_context *context);
void sha384_init(struct sha512_context *context);
void sha512_update(struct sha512_context *context, const uint8_t *data, uint32_t length);
void sha512_final(struct sha512_context *context, uint8_t *digest);

uint32_t hash_digest_size(enum hash_algorithm algorithm);
uint32_t hash_block_size(enum hash_algorithm algorithm);
int hash_init(struct hash_context *context, enum hash_algorithm algorithm);
void hash_update(struct hash_context *context, const uint8_t *data, uint32_t length);
void hash_final(struct hash_context *context, uint8_t *digest);
void hash_snapshot(const struct hash_context *context, uint8_t *digest);
int hash_digest(enum hash_algorithm algorithm, const uint8_t *data, uint32_t length, uint8_t *digest);

int hmac_begin(struct hmac_context *context, enum hash_algorithm algorithm, const uint8_t *key, uint32_t key_length);
void hmac_feed(struct hmac_context *context, const uint8_t *data, uint32_t length);
void hmac_end(struct hmac_context *context, uint8_t *digest);
int hmac_compute(enum hash_algorithm algorithm, const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t *digest);
void hmac_sha256(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[32]);
void hmac_sha384(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[48]);

int tls12_prf(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
              const char *label, const uint8_t *seed, uint32_t seed_length,
              uint8_t *output, uint32_t output_length);
void tls12_prf_sha256(const uint8_t *secret, uint32_t secret_length,
                      const char *label, const uint8_t *seed, uint32_t seed_length,
                      uint8_t *output, uint32_t output_length);

int hkdf_extract(enum hash_algorithm algorithm, const uint8_t *salt, uint32_t salt_length,
                 const uint8_t *input, uint32_t input_length, uint8_t *output);
int hkdf_expand(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                const uint8_t *info, uint32_t info_length, uint8_t *output, uint32_t output_length);
void hkdf_extract_sha256(const uint8_t *salt, uint32_t salt_length,
                         const uint8_t *input, uint32_t input_length, uint8_t output[32]);
void hkdf_expand_sha256(const uint8_t secret[32], const uint8_t *info, uint32_t info_length,
                        uint8_t *output, uint32_t output_length);

int tls13_expand_label_ex(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                          const char *label, const uint8_t *context, uint32_t context_length,
                          uint8_t *output, uint32_t output_length);
int tls13_derive_secret(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                        const char *label, const uint8_t *transcript_hash, uint8_t *output);
void tls13_expand_label(const uint8_t secret[32], const char *label,
                        const uint8_t *context, uint32_t context_length,
                        uint8_t *output, uint32_t output_length);

int aes_set_key(struct aes_context *context, const uint8_t *key, uint32_t key_length);
void aes_encrypt_block(const struct aes_context *context, const uint8_t input[16], uint8_t output[16]);

int aes_gcm_init(struct aes_gcm_context *context, const uint8_t *key, uint32_t key_length);
void aes_gcm_seal(const struct aes_gcm_context *context, const uint8_t nonce[12],
                  const uint8_t *additional, uint32_t additional_length,
                  uint8_t *data, uint32_t length, uint8_t tag[16]);
int aes_gcm_open(const struct aes_gcm_context *context, const uint8_t nonce[12],
                 const uint8_t *additional, uint32_t additional_length,
                 uint8_t *data, uint32_t length, const uint8_t tag[16]);

int aes128_gcm_encrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16]);
int aes128_gcm_decrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16]);
int aes256_gcm_encrypt(const uint8_t key[32], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16]);
int aes256_gcm_decrypt(const uint8_t key[32], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16]);

void chacha20_xor(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12], uint8_t *data, uint32_t length);
void poly1305_init(struct poly1305_context *context, const uint8_t key[32]);
void poly1305_update(struct poly1305_context *context, const uint8_t *data, uint32_t length);
void poly1305_final(struct poly1305_context *context, uint8_t tag[16]);
void chacha20_poly1305_seal(const uint8_t key[32], const uint8_t nonce[12],
                            const uint8_t *additional, uint32_t additional_length,
                            uint8_t *data, uint32_t length, uint8_t tag[16]);
int chacha20_poly1305_open(const uint8_t key[32], const uint8_t nonce[12],
                           const uint8_t *additional, uint32_t additional_length,
                           uint8_t *data, uint32_t length, const uint8_t tag[16]);

void x25519_public_key(const uint8_t private_key[32], uint8_t public_key[32]);
int x25519_shared_secret(const uint8_t private_key[32], const uint8_t peer_key[32], uint8_t shared_secret[32]);

void hmac_drbg_instantiate(struct hmac_drbg_context *context, const uint8_t *seed, uint32_t seed_length);
void hmac_drbg_reseed(struct hmac_drbg_context *context, const uint8_t *seed, uint32_t seed_length);
void hmac_drbg_generate(struct hmac_drbg_context *context, uint8_t *output, uint32_t length);

void crypto_set_entropy_source(crypto_entropy_source source);
int crypto_random_bytes(uint8_t *output, uint32_t length);

#define TLS_OK 0
#define TLS_ERROR_TRANSPORT (-1)
#define TLS_ERROR_PROTOCOL (-2)
#define TLS_ERROR_UNSUPPORTED (-3)
#define TLS_ERROR_DECRYPT (-4)
#define TLS_ERROR_CERTIFICATE (-5)
#define TLS_ERROR_HANDSHAKE (-6)
#define TLS_ERROR_ALERT (-7)
#define TLS_ERROR_BUFFER (-8)
#define TLS_ERROR_ENTROPY (-9)
#define TLS_ERROR_STATE (-10)
#define TLS_ERROR_FINISHED (-11)
#define TLS_ERROR_TRUNCATED (-12)
#define TLS_ERROR_ARGUMENT (-13)

#define TLS_MAX_PLAINTEXT 16384
#define TLS_MAX_CIPHERTEXT 16640
#define TLS_RECORD_HEADER_SIZE 5
#define TLS_RECORD_BUFFER_SIZE (TLS_RECORD_HEADER_SIZE + TLS_MAX_CIPHERTEXT)
#define TLS_HANDSHAKE_BUFFER_SIZE 20480
#define TLS_CERTIFICATE_BUFFER_SIZE 16384
#define TLS_CLIENT_HELLO_BUFFER_SIZE 1024
#define TLS_TRUST_STORE_MAX 192
#define TLS_MAX_CHAIN 10
#define TLS_MAX_SECRET_SIZE 64
#define TLS_SERVER_NAME_MAX 253

#define TLS_SCHEME_RSA_PKCS1_SHA256 0x0401
#define TLS_SCHEME_RSA_PKCS1_SHA384 0x0501
#define TLS_SCHEME_RSA_PKCS1_SHA512 0x0601
#define TLS_SCHEME_ECDSA_SECP256R1_SHA256 0x0403
#define TLS_SCHEME_ECDSA_SECP384R1_SHA384 0x0503
#define TLS_SCHEME_RSA_PSS_RSAE_SHA256 0x0804
#define TLS_SCHEME_RSA_PSS_RSAE_SHA384 0x0805
#define TLS_SCHEME_RSA_PSS_RSAE_SHA512 0x0806
#define TLS_SCHEME_ED25519 0x0807

#define TLS_FLAG_ALLOW_INSECURE 0x00000001u
#define TLS_FLAG_DISABLE_CHACHA20 0x00000002u
#define TLS_FLAG_DISABLE_AES256 0x00000004u
#define TLS_FLAG_DISABLE_TLS12 0x00000008u

struct x509_key {
    uint32_t type;
    const uint8_t *modulus;
    uint32_t modulus_length;
    const uint8_t *exponent;
    uint32_t exponent_length;
    const uint8_t *point;
    uint32_t point_length;
};

struct x509_certificate {
    const uint8_t *der;
    uint32_t der_length;
    const uint8_t *tbs;
    uint32_t tbs_length;
    const uint8_t *issuer;
    uint32_t issuer_length;
    const uint8_t *subject;
    uint32_t subject_length;
    const uint8_t *spki;
    uint32_t spki_length;
    uint32_t signature_algorithm;
    const uint8_t *signature;
    uint32_t signature_length;
    uint64_t not_before;
    uint64_t not_after;
    struct x509_key key;
    uint32_t basic_constraints_present;
    uint32_t is_ca;
    int path_length;
    uint32_t key_usage_present;
    uint32_t key_usage;
    const uint8_t *alt_names;
    uint32_t alt_names_length;
    uint32_t ext_usage_present;
    uint32_t server_auth;
};

struct tls_trust_anchor {
    const uint8_t *subject;
    uint32_t subject_length;
    const uint8_t *spki;
    uint32_t spki_length;
};

struct tls_trust_store {
    struct tls_trust_anchor anchors[TLS_TRUST_STORE_MAX];
    uint32_t count;
};

struct tls_transport {
    int (*send)(void *context, const uint8_t *data, uint32_t length);
    int (*receive)(void *context, uint8_t *data, uint32_t capacity);
    void *context;
};

typedef int (*tls_verify_callback)(void *user_context, const char *server_name,
                                   const uint8_t *certificate_list, uint32_t certificate_list_length,
                                   uint16_t signature_scheme,
                                   const uint8_t *signature, uint32_t signature_length,
                                   const uint8_t *signed_content, uint32_t signed_content_length);

struct tls_config {
    const char *server_name;
    tls_verify_callback verify;
    void *verify_context;
    uint32_t flags;
    const uint16_t *signature_schemes;
    uint32_t signature_scheme_count;
    const struct tls_trust_store *trust_store;
    uint64_t unix_time;
};

enum tls_state {
    TLS_STATE_IDLE = 0,
    TLS_STATE_WAIT_SERVER_HELLO,
    TLS_STATE_WAIT_ENCRYPTED_EXTENSIONS,
    TLS_STATE_WAIT_CERTIFICATE_OR_REQUEST,
    TLS_STATE_WAIT_CERTIFICATE,
    TLS_STATE_WAIT_CERTIFICATE_VERIFY,
    TLS_STATE_WAIT_FINISHED,
    TLS_STATE_CONNECTED,
    TLS_STATE_CLOSED,
    TLS_STATE_FAILED
};

enum tls_aead_kind {
    TLS_AEAD_AES_GCM = 1,
    TLS_AEAD_CHACHA20_POLY1305 = 2
};

struct tls_cipher_suite {
    uint16_t identifier;
    enum tls_aead_kind aead;
    uint32_t key_length;
    enum hash_algorithm hash;
    uint32_t tls12;
};

struct tls_cipher_state {
    uint8_t secret[TLS_MAX_SECRET_SIZE];
    uint8_t key[32];
    uint8_t iv[12];
    uint64_t sequence;
    struct aes_gcm_context gcm;
    uint32_t active;
};

struct tls_client {
    struct tls_transport transport;
    struct tls_config config;
    char server_name[TLS_SERVER_NAME_MAX + 1];
    enum tls_state state;
    const struct tls_cipher_suite *suite;
    struct hash_context transcript;
    struct tls_cipher_state read;
    struct tls_cipher_state write;
    uint8_t master_secret[TLS_MAX_SECRET_SIZE];
    uint8_t private_key[X25519_KEY_SIZE];
    uint8_t ec_private[48];
    uint8_t client_random[32];
    uint8_t server_random[32];
    uint8_t session_id[32];
    uint8_t client_hello[TLS_CLIENT_HELLO_BUFFER_SIZE];
    uint32_t client_hello_length;
    const struct tls_cipher_suite *retry_suite;
    uint32_t retry_group;
    uint8_t cookie[256];
    uint32_t cookie_length;
    uint8_t server_share[97];
    uint32_t server_share_length;
    uint32_t server_group;
    uint32_t version12;
    uint32_t extended_master_secret;
    uint32_t payload_offset;
    uint8_t certificate_request_context[256];
    uint32_t certificate_request_context_length;
    uint32_t certificate_requested;
    uint8_t *certificate_list;
    uint32_t certificate_list_length;
    uint8_t record[TLS_RECORD_BUFFER_SIZE];
    uint8_t send_record[TLS_RECORD_BUFFER_SIZE];
    uint8_t handshake[TLS_HANDSHAKE_BUFFER_SIZE];
    uint32_t handshake_length;
    uint8_t certificate_storage[TLS_CERTIFICATE_BUFFER_SIZE];
    uint32_t plain_offset;
    uint32_t plain_length;
    uint32_t peer_closed;
    int last_alert;
};

int tls_client_connect(struct tls_client *client, const struct tls_transport *transport,
                       const struct tls_config *config);
int tls_client_send(struct tls_client *client, const uint8_t *data, uint32_t length);
int tls_client_receive(struct tls_client *client, uint8_t *buffer, uint32_t capacity);
int tls_client_close(struct tls_client *client);
int tls_client_is_connected(const struct tls_client *client);
uint16_t tls_client_cipher_suite(const struct tls_client *client);
int tls_client_last_alert(const struct tls_client *client);
uint16_t tls_client_version(const struct tls_client *client);
const char *tls_error_string(int error);

int x509_parse_certificate(const uint8_t *der, uint32_t length, struct x509_certificate *certificate);
int x509_verify_signature(const struct x509_certificate *certificate, const struct x509_key *issuer_key);
int x509_verify_handshake_signature(const struct x509_key *key, uint16_t scheme, const uint8_t *message,
                                    uint32_t message_length, const uint8_t *signature, uint32_t signature_length);
int x509_matches_host(const struct x509_certificate *certificate, const char *host);
void tls_trust_store_init(struct tls_trust_store *store);
int tls_trust_store_add_anchor(struct tls_trust_store *store, const uint8_t *subject, uint32_t subject_length,
                               const uint8_t *spki, uint32_t spki_length);
int tls_trust_store_add_certificate(struct tls_trust_store *store, const uint8_t *der, uint32_t length);
int tls_trust_store_add_builtin(struct tls_trust_store *store);
const struct tls_trust_store *tls_default_trust_store(void);
int tls_verify_certificate_chain(const uint8_t *list, uint32_t list_length, const char *server_name,
                                 uint64_t unix_time, const struct tls_trust_store *store, struct x509_key *leaf_key);
int tls_certificate_list_next(const uint8_t *list, uint32_t list_length, uint32_t *offset,
                              const uint8_t **certificate, uint32_t *certificate_length);

#define HTTPS_ERROR_ARGUMENT (-100)
#define HTTPS_ERROR_MALFORMED (-101)
#define HTTPS_ERROR_HEADER_TOO_LARGE (-102)
#define HTTPS_ERROR_STATE (-103)
#define HTTPS_ERROR_URL (-104)

#define HTTPS_IO_BUFFER_SIZE 4096
#define HTTPS_HOST_MAX 253
#define HTTPS_PATH_MAX 1024
#define HTTPS_LOCATION_MAX 1024
#define HTTPS_CONTENT_TYPE_MAX 96
#define HTTPS_UNKNOWN_LENGTH 0xFFFFFFFFu

struct https_url {
    char host[HTTPS_HOST_MAX + 1];
    char path[HTTPS_PATH_MAX];
    uint16_t port;
};

struct https_connection {
    struct tls_client tls;
    uint8_t io[HTTPS_IO_BUFFER_SIZE];
    uint32_t io_offset;
    uint32_t io_length;
    uint32_t end_of_stream;
};

struct https_request_params {
    const char *method;
    const char *host;
    const char *path;
    const char *user_agent;
    const char *extra_headers;
    const uint8_t *body;
    uint32_t body_length;
    uint8_t *response_buffer;
    uint32_t response_capacity;
    uint32_t close_connection;
};

struct https_response {
    int status_code;
    uint32_t content_length;
    uint32_t body_length;
    uint32_t body_total;
    uint32_t truncated;
    uint32_t chunked;
    uint32_t connection_close;
    char location[HTTPS_LOCATION_MAX];
    char content_type[HTTPS_CONTENT_TYPE_MAX];
};

int https_parse_url(const char *url, struct https_url *result);
int https_connect(struct https_connection *connection, const struct tls_transport *transport,
                  const struct tls_config *config);
int https_request(struct https_connection *connection, const struct https_request_params *params,
                  struct https_response *response);
int https_get(struct https_connection *connection, const char *host, const char *path,
              uint8_t *buffer, uint32_t capacity, struct https_response *response);
struct https_dialer {
    int (*open)(void *context, const char *host, uint16_t port, struct tls_transport *transport);
    void (*close)(void *context, struct tls_transport *transport);
    void *context;
};

int https_fetch(struct https_connection *connection, const struct https_dialer *dialer,
                const struct tls_config *config, const char *url, uint8_t *buffer, uint32_t capacity,
                struct https_response *response, uint32_t max_redirects);
int https_close(struct https_connection *connection);
const char *https_error_string(int error);

#ifndef TLS_CRYPTO_EXPORT_ONLY

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

static const uint64_t sha512_round_constants[80] = {
    0x428A2F98D728AE22ULL,0x7137449123EF65CDULL,0xB5C0FBCFEC4D3B2FULL,0xE9B5DBA58189DBBCULL,
    0x3956C25BF348B538ULL,0x59F111F1B605D019ULL,0x923F82A4AF194F9BULL,0xAB1C5ED5DA6D8118ULL,
    0xD807AA98A3030242ULL,0x12835B0145706FBEULL,0x243185BE4EE4B28CULL,0x550C7DC3D5FFB4E2ULL,
    0x72BE5D74F27B896FULL,0x80DEB1FE3B1696B1ULL,0x9BDC06A725C71235ULL,0xC19BF174CF692694ULL,
    0xE49B69C19EF14AD2ULL,0xEFBE4786384F25E3ULL,0x0FC19DC68B8CD5B5ULL,0x240CA1CC77AC9C65ULL,
    0x2DE92C6F592B0275ULL,0x4A7484AA6EA6E483ULL,0x5CB0A9DCBD41FBD4ULL,0x76F988DA831153B5ULL,
    0x983E5152EE66DFABULL,0xA831C66D2DB43210ULL,0xB00327C898FB213FULL,0xBF597FC7BEEF0EE4ULL,
    0xC6E00BF33DA88FC2ULL,0xD5A79147930AA725ULL,0x06CA6351E003826FULL,0x142929670A0E6E70ULL,
    0x27B70A8546D22FFCULL,0x2E1B21385C26C926ULL,0x4D2C6DFC5AC42AEDULL,0x53380D139D95B3DFULL,
    0x650A73548BAF63DEULL,0x766A0ABB3C77B2A8ULL,0x81C2C92E47EDAEE6ULL,0x92722C851482353BULL,
    0xA2BFE8A14CF10364ULL,0xA81A664BBC423001ULL,0xC24B8B70D0F89791ULL,0xC76C51A30654BE30ULL,
    0xD192E819D6EF5218ULL,0xD69906245565A910ULL,0xF40E35855771202AULL,0x106AA07032BBD1B8ULL,
    0x19A4C116B8D2D0C8ULL,0x1E376C085141AB53ULL,0x2748774CDF8EEB99ULL,0x34B0BCB5E19B48A8ULL,
    0x391C0CB3C5C95A63ULL,0x4ED8AA4AE3418ACBULL,0x5B9CCA4F7763E373ULL,0x682E6FF3D6B2B8A3ULL,
    0x748F82EE5DEFB2FCULL,0x78A5636F43172F60ULL,0x84C87814A1F0AB72ULL,0x8CC702081A6439ECULL,
    0x90BEFFFA23631E28ULL,0xA4506CEBDE82BDE9ULL,0xBEF9A3F7B2C67915ULL,0xC67178F2E372532BULL,
    0xCA273ECEEA26619CULL,0xD186B8C721C0C207ULL,0xEADA7DD6CDE0EB1EULL,0xF57D4F7FEE6ED178ULL,
    0x06F067AA72176FBAULL,0x0A637DC5A2C898A6ULL,0x113F9804BEF90DAEULL,0x1B710B35131C471BULL,
    0x28DB77F523047D84ULL,0x32CAAB7B40C72493ULL,0x3C9EBE0A15C9BEBCULL,0x431D67C49C100D4CULL,
    0x4CC5D4BECB3E42B6ULL,0x597F299CFC657E2AULL,0x5FCB6FAB3AD6FAECULL,0x6C44198C4A475817ULL
};

static const uint64_t sha512_initial[8] = {
    0x6A09E667F3BCC908ULL,0xBB67AE8584CAA73BULL,0x3C6EF372FE94F82BULL,0xA54FF53A5F1D36F1ULL,
    0x510E527FADE682D1ULL,0x9B05688C2B3E6C1FULL,0x1F83D9ABFB41BD6BULL,0x5BE0CD19137E2179ULL
};

static const uint64_t sha384_initial[8] = {
    0xCBBB9D5DC1059ED8ULL,0x629A292A367CD507ULL,0x9159015A3070DD17ULL,0x152FECD8F70E5939ULL,
    0x67332667FFC00B31ULL,0x8EB44A8768581511ULL,0xDB0C2E0D64F98FA7ULL,0x47B5481DBEFA4FA4ULL
};

void crypto_zero(void *data, uint32_t length)
{
    volatile uint8_t *bytes = (volatile uint8_t *)data;
    while (length--) *bytes++ = 0;
}

void crypto_copy(void *destination, const void *source, uint32_t length)
{
    uint8_t *to = (uint8_t *)destination;
    const uint8_t *from = (const uint8_t *)source;
    for (uint32_t i = 0; i < length; i++) to[i] = from[i];
}

int crypto_equal(const void *left, const void *right, uint32_t length)
{
    const uint8_t *a = (const uint8_t *)left;
    const uint8_t *b = (const uint8_t *)right;
    uint8_t difference = 0;
    for (uint32_t i = 0; i < length; i++) difference |= (uint8_t)(a[i] ^ b[i]);
    return difference == 0;
}

uint32_t crypto_string_length(const char *text)
{
    uint32_t length = 0;
    while (text[length]) length++;
    return length;
}

static uint32_t rotate_right(uint32_t value, uint32_t count)
{
    return (value >> count) | (value << (32 - count));
}

static uint64_t rotate_right64(uint64_t value, uint32_t count)
{
    return (value >> count) | (value << (64 - count));
}

static uint32_t rotate_left(uint32_t value, uint32_t count)
{
    return (value << count) | (value >> (32 - count));
}

static uint32_t load_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

static uint64_t load_be64(const uint8_t *data)
{
    return ((uint64_t)load_be32(data) << 32) | load_be32(data + 4);
}

static uint32_t load_le32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
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

static void store_le32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value >> 16);
    data[3] = (uint8_t)(value >> 24);
}

static void store_le64(uint8_t *data, uint64_t value)
{
    for (uint32_t i = 0; i < 8; i++) data[i] = (uint8_t)(value >> (i * 8));
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
    crypto_zero(words, sizeof(words));
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
        crypto_copy(context->block + context->block_used, data, take);
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
    crypto_zero(context, sizeof(*context));
}

void sha256_digest(const uint8_t *data, uint32_t length, uint8_t digest[32])
{
    struct sha256_context context;
    sha256_init(&context);
    sha256_update(&context, data, length);
    sha256_final(&context, digest);
}

static void sha512_transform(struct sha512_context *context, const uint8_t block[128])
{
    uint64_t words[80];
    uint64_t a, b, c, d, e, f, g, h;
    for (uint32_t i = 0; i < 16; i++) words[i] = load_be64(block + i * 8);
    for (uint32_t i = 16; i < 80; i++) {
        uint64_t s0 = rotate_right64(words[i - 15], 1) ^ rotate_right64(words[i - 15], 8) ^ (words[i - 15] >> 7);
        uint64_t s1 = rotate_right64(words[i - 2], 19) ^ rotate_right64(words[i - 2], 61) ^ (words[i - 2] >> 6);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }
    a = context->state[0]; b = context->state[1]; c = context->state[2]; d = context->state[3];
    e = context->state[4]; f = context->state[5]; g = context->state[6]; h = context->state[7];
    for (uint32_t i = 0; i < 80; i++) {
        uint64_t sum1 = rotate_right64(e, 14) ^ rotate_right64(e, 18) ^ rotate_right64(e, 41);
        uint64_t choice = (e & f) ^ (~e & g);
        uint64_t temp1 = h + sum1 + choice + sha512_round_constants[i] + words[i];
        uint64_t sum0 = rotate_right64(a, 28) ^ rotate_right64(a, 34) ^ rotate_right64(a, 39);
        uint64_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint64_t temp2 = sum0 + majority;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    context->state[0] += a; context->state[1] += b; context->state[2] += c; context->state[3] += d;
    context->state[4] += e; context->state[5] += f; context->state[6] += g; context->state[7] += h;
    crypto_zero(words, sizeof(words));
}

void sha512_init(struct sha512_context *context)
{
    for (uint32_t i = 0; i < 8; i++) context->state[i] = sha512_initial[i];
    context->total_bytes = 0;
    context->block_used = 0;
    context->digest_length = 64;
}

void sha384_init(struct sha512_context *context)
{
    for (uint32_t i = 0; i < 8; i++) context->state[i] = sha384_initial[i];
    context->total_bytes = 0;
    context->block_used = 0;
    context->digest_length = 48;
}

void sha512_update(struct sha512_context *context, const uint8_t *data, uint32_t length)
{
    context->total_bytes += length;
    while (length) {
        uint32_t available = 128 - context->block_used;
        uint32_t take = length < available ? length : available;
        crypto_copy(context->block + context->block_used, data, take);
        context->block_used += take;
        data += take;
        length -= take;
        if (context->block_used == 128) {
            sha512_transform(context, context->block);
            context->block_used = 0;
        }
    }
}

void sha512_final(struct sha512_context *context, uint8_t *digest)
{
    uint8_t full[64];
    uint32_t digest_length = context->digest_length;
    uint64_t bit_low = context->total_bytes << 3;
    uint64_t bit_high = context->total_bytes >> 61;
    context->block[context->block_used++] = 0x80;
    if (context->block_used > 112) {
        while (context->block_used < 128) context->block[context->block_used++] = 0;
        sha512_transform(context, context->block);
        context->block_used = 0;
    }
    while (context->block_used < 112) context->block[context->block_used++] = 0;
    store_be64(context->block + 112, bit_high);
    store_be64(context->block + 120, bit_low);
    sha512_transform(context, context->block);
    for (uint32_t i = 0; i < 8; i++) store_be64(full + i * 8, context->state[i]);
    crypto_copy(digest, full, digest_length);
    crypto_zero(full, sizeof(full));
    crypto_zero(context, sizeof(*context));
}

uint32_t hash_digest_size(enum hash_algorithm algorithm)
{
    switch (algorithm) {
    case HASH_SHA256: return SHA256_DIGEST_SIZE;
    case HASH_SHA384: return SHA384_DIGEST_SIZE;
    case HASH_SHA512: return SHA512_DIGEST_SIZE;
    default: return 0;
    }
}

uint32_t hash_block_size(enum hash_algorithm algorithm)
{
    switch (algorithm) {
    case HASH_SHA256: return 64;
    case HASH_SHA384: return 128;
    case HASH_SHA512: return 128;
    default: return 0;
    }
}

int hash_init(struct hash_context *context, enum hash_algorithm algorithm)
{
    crypto_zero(context, sizeof(*context));
    switch (algorithm) {
    case HASH_SHA256: sha256_init(&context->sha256); break;
    case HASH_SHA384: sha384_init(&context->sha512); break;
    case HASH_SHA512: sha512_init(&context->sha512); break;
    default: return -1;
    }
    context->algorithm = algorithm;
    return 0;
}

void hash_update(struct hash_context *context, const uint8_t *data, uint32_t length)
{
    if (context->algorithm == HASH_SHA256) sha256_update(&context->sha256, data, length);
    else if (context->algorithm != HASH_NONE) sha512_update(&context->sha512, data, length);
}

void hash_final(struct hash_context *context, uint8_t *digest)
{
    if (context->algorithm == HASH_SHA256) sha256_final(&context->sha256, digest);
    else if (context->algorithm != HASH_NONE) sha512_final(&context->sha512, digest);
}

void hash_snapshot(const struct hash_context *context, uint8_t *digest)
{
    struct hash_context copy;
    crypto_copy(&copy, context, sizeof(copy));
    hash_final(&copy, digest);
    crypto_zero(&copy, sizeof(copy));
}

int hash_digest(enum hash_algorithm algorithm, const uint8_t *data, uint32_t length, uint8_t *digest)
{
    struct hash_context context;
    if (hash_init(&context, algorithm)) return -1;
    hash_update(&context, data, length);
    hash_final(&context, digest);
    return 0;
}

int hmac_begin(struct hmac_context *context, enum hash_algorithm algorithm, const uint8_t *key, uint32_t key_length)
{
    uint8_t normalized_key[HASH_MAX_BLOCK_SIZE];
    uint8_t pad[HASH_MAX_BLOCK_SIZE];
    uint32_t block_size = hash_block_size(algorithm);
    if (!block_size) return -1;
    crypto_zero(normalized_key, sizeof(normalized_key));
    if (key_length > block_size) hash_digest(algorithm, key, key_length, normalized_key);
    else if (key_length) crypto_copy(normalized_key, key, key_length);
    for (uint32_t i = 0; i < block_size; i++) pad[i] = normalized_key[i] ^ 0x36;
    hash_init(&context->inner, algorithm);
    hash_update(&context->inner, pad, block_size);
    for (uint32_t i = 0; i < block_size; i++) pad[i] = normalized_key[i] ^ 0x5C;
    hash_init(&context->outer, algorithm);
    hash_update(&context->outer, pad, block_size);
    crypto_zero(normalized_key, sizeof(normalized_key));
    crypto_zero(pad, sizeof(pad));
    return 0;
}

void hmac_feed(struct hmac_context *context, const uint8_t *data, uint32_t length)
{
    hash_update(&context->inner, data, length);
}

void hmac_end(struct hmac_context *context, uint8_t *digest)
{
    uint8_t inner_digest[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(context->inner.algorithm);
    hash_final(&context->inner, inner_digest);
    hash_update(&context->outer, inner_digest, digest_size);
    hash_final(&context->outer, digest);
    crypto_zero(inner_digest, sizeof(inner_digest));
    crypto_zero(context, sizeof(*context));
}

int hmac_compute(enum hash_algorithm algorithm, const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t *digest)
{
    struct hmac_context context;
    if (hmac_begin(&context, algorithm, key, key_length)) return -1;
    hmac_feed(&context, data, data_length);
    hmac_end(&context, digest);
    return 0;
}

void hmac_sha256(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[32])
{
    hmac_compute(HASH_SHA256, key, key_length, data, data_length, digest);
}

void hmac_sha384(const uint8_t *key, uint32_t key_length,
                 const uint8_t *data, uint32_t data_length, uint8_t digest[48])
{
    hmac_compute(HASH_SHA384, key, key_length, data, data_length, digest);
}

int tls12_prf(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
              const char *label, const uint8_t *seed, uint32_t seed_length,
              uint8_t *output, uint32_t output_length)
{
    struct hmac_context context;
    uint8_t a_value[HASH_MAX_DIGEST_SIZE];
    uint8_t block[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(algorithm);
    uint32_t label_length = crypto_string_length(label);
    uint32_t produced = 0;
    if (!digest_size) return -1;
    hmac_begin(&context, algorithm, secret, secret_length);
    hmac_feed(&context, (const uint8_t *)label, label_length);
    hmac_feed(&context, seed, seed_length);
    hmac_end(&context, a_value);
    while (produced < output_length) {
        uint32_t take = output_length - produced < digest_size ? output_length - produced : digest_size;
        hmac_begin(&context, algorithm, secret, secret_length);
        hmac_feed(&context, a_value, digest_size);
        hmac_feed(&context, (const uint8_t *)label, label_length);
        hmac_feed(&context, seed, seed_length);
        hmac_end(&context, block);
        crypto_copy(output + produced, block, take);
        produced += take;
        hmac_compute(algorithm, secret, secret_length, a_value, digest_size, a_value);
    }
    crypto_zero(a_value, sizeof(a_value));
    crypto_zero(block, sizeof(block));
    return 0;
}

void tls12_prf_sha256(const uint8_t *secret, uint32_t secret_length,
                      const char *label, const uint8_t *seed, uint32_t seed_length,
                      uint8_t *output, uint32_t output_length)
{
    if (tls12_prf(HASH_SHA256, secret, secret_length, label, seed, seed_length, output, output_length))
        crypto_zero(output, output_length);
}

int hkdf_extract(enum hash_algorithm algorithm, const uint8_t *salt, uint32_t salt_length,
                 const uint8_t *input, uint32_t input_length, uint8_t *output)
{
    if (!salt) salt_length = 0;
    return hmac_compute(algorithm, salt, salt_length, input, input_length, output);
}

int hkdf_expand(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                const uint8_t *info, uint32_t info_length, uint8_t *output, uint32_t output_length)
{
    struct hmac_context context;
    uint8_t previous[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(algorithm);
    uint32_t previous_length = 0;
    uint32_t used = 0;
    uint8_t counter = 1;
    if (!digest_size || output_length > 255 * digest_size) return -1;
    while (used < output_length) {
        uint32_t take = output_length - used < digest_size ? output_length - used : digest_size;
        hmac_begin(&context, algorithm, secret, secret_length);
        hmac_feed(&context, previous, previous_length);
        if (info_length) hmac_feed(&context, info, info_length);
        hmac_feed(&context, &counter, 1);
        hmac_end(&context, previous);
        crypto_copy(output + used, previous, take);
        used += take;
        previous_length = digest_size;
        counter++;
    }
    crypto_zero(previous, sizeof(previous));
    return 0;
}

void hkdf_extract_sha256(const uint8_t *salt, uint32_t salt_length,
                         const uint8_t *input, uint32_t input_length, uint8_t output[32])
{
    hkdf_extract(HASH_SHA256, salt, salt_length, input, input_length, output);
}

void hkdf_expand_sha256(const uint8_t secret[32], const uint8_t *info, uint32_t info_length,
                        uint8_t *output, uint32_t output_length)
{
    if (hkdf_expand(HASH_SHA256, secret, 32, info, info_length, output, output_length))
        crypto_zero(output, output_length);
}

int tls13_expand_label_ex(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                          const char *label, const uint8_t *context, uint32_t context_length,
                          uint8_t *output, uint32_t output_length)
{
    uint8_t info[2 + 1 + 255 + 1 + 255];
    uint32_t label_length = crypto_string_length(label);
    uint32_t p = 0;
    int result;
    if (label_length + 6 > 255 || context_length > 255 || output_length > 65535) return -1;
    info[p++] = (uint8_t)(output_length >> 8);
    info[p++] = (uint8_t)output_length;
    info[p++] = (uint8_t)(label_length + 6);
    crypto_copy(info + p, "tls13 ", 6); p += 6;
    crypto_copy(info + p, label, label_length); p += label_length;
    info[p++] = (uint8_t)context_length;
    if (context_length) { crypto_copy(info + p, context, context_length); p += context_length; }
    result = hkdf_expand(algorithm, secret, secret_length, info, p, output, output_length);
    crypto_zero(info, sizeof(info));
    return result;
}

int tls13_derive_secret(enum hash_algorithm algorithm, const uint8_t *secret, uint32_t secret_length,
                        const char *label, const uint8_t *transcript_hash, uint8_t *output)
{
    uint32_t digest_size = hash_digest_size(algorithm);
    if (!digest_size) return -1;
    return tls13_expand_label_ex(algorithm, secret, secret_length, label, transcript_hash, digest_size,
                                 output, digest_size);
}

void tls13_expand_label(const uint8_t secret[32], const char *label,
                        const uint8_t *context, uint32_t context_length,
                        uint8_t *output, uint32_t output_length)
{
    if (tls13_expand_label_ex(HASH_SHA256, secret, 32, label, context, context_length, output, output_length))
        crypto_zero(output, output_length);
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

static uint8_t aes_xtime(uint8_t value)
{
    return (uint8_t)((value << 1) ^ (0x1b & (uint8_t)(0 - (value >> 7))));
}

int aes_set_key(struct aes_context *context, const uint8_t *key, uint32_t key_length)
{
    uint32_t words_in_key;
    uint32_t total_words;
    uint8_t rcon = 1;
    if (key_length != 16 && key_length != 24 && key_length != 32) return -1;
    words_in_key = key_length / 4;
    context->rounds = words_in_key + 6;
    total_words = 4 * (context->rounds + 1);
    crypto_copy(context->round_keys, key, key_length);
    for (uint32_t i = words_in_key; i < total_words; i++) {
        uint8_t temporary[4];
        crypto_copy(temporary, context->round_keys + 4 * (i - 1), 4);
        if (i % words_in_key == 0) {
            uint8_t first = temporary[0];
            temporary[0] = aes_sbox[temporary[1]] ^ rcon;
            temporary[1] = aes_sbox[temporary[2]];
            temporary[2] = aes_sbox[temporary[3]];
            temporary[3] = aes_sbox[first];
            rcon = aes_xtime(rcon);
        } else if (words_in_key > 6 && i % words_in_key == 4) {
            for (uint32_t j = 0; j < 4; j++) temporary[j] = aes_sbox[temporary[j]];
        }
        for (uint32_t j = 0; j < 4; j++)
            context->round_keys[4 * i + j] = context->round_keys[4 * (i - words_in_key) + j] ^ temporary[j];
    }
    return 0;
}

void aes_encrypt_block(const struct aes_context *context, const uint8_t input[16], uint8_t output[16])
{
    uint8_t state[16], temporary[16];
    crypto_copy(state, input, 16);
    for (uint32_t i = 0; i < 16; i++) state[i] ^= context->round_keys[i];
    for (uint32_t round = 1; round <= context->rounds; round++) {
        for (uint32_t i = 0; i < 16; i++) state[i] = aes_sbox[state[i]];
        for (uint32_t row = 0; row < 4; row++)
            for (uint32_t column = 0; column < 4; column++)
                temporary[row + 4 * column] = state[row + 4 * ((column + row) & 3)];
        crypto_copy(state, temporary, 16);
        if (round != context->rounds) {
            for (uint32_t column = 0; column < 4; column++) {
                uint8_t *s = state + 4 * column;
                uint8_t a = s[0], b = s[1], c = s[2], d = s[3];
                s[0] = aes_xtime(a) ^ (aes_xtime(b) ^ b) ^ c ^ d;
                s[1] = a ^ aes_xtime(b) ^ (aes_xtime(c) ^ c) ^ d;
                s[2] = a ^ b ^ aes_xtime(c) ^ (aes_xtime(d) ^ d);
                s[3] = (aes_xtime(a) ^ a) ^ b ^ c ^ aes_xtime(d);
            }
        }
        for (uint32_t i = 0; i < 16; i++) state[i] ^= context->round_keys[round * 16 + i];
    }
    crypto_copy(output, state, 16);
    crypto_zero(state, sizeof(state));
    crypto_zero(temporary, sizeof(temporary));
}

static void xor_block(uint8_t left[16], const uint8_t right[16])
{
    for (uint32_t i = 0; i < 16; i++) left[i] ^= right[i];
}

static void ghash_multiply(uint8_t value[16], const uint8_t hash[16])
{
    uint8_t result[16], factor[16];
    crypto_zero(result, 16);
    crypto_copy(factor, hash, 16);
    for (uint32_t bit = 0; bit < 128; bit++) {
        uint8_t selected = (value[bit / 8] >> (7 - (bit & 7))) & 1;
        uint8_t mask = (uint8_t)(0 - selected);
        uint8_t low = factor[15] & 1;
        for (uint32_t i = 0; i < 16; i++) result[i] ^= factor[i] & mask;
        for (int i = 15; i > 0; i--) factor[i] = (uint8_t)((factor[i] >> 1) | (factor[i - 1] << 7));
        factor[0] >>= 1;
        factor[0] ^= (uint8_t)(0xe1 & (uint8_t)(0 - low));
    }
    crypto_copy(value, result, 16);
    crypto_zero(result, 16);
    crypto_zero(factor, 16);
}

static void ghash_update(uint8_t state[16], const uint8_t hash[16], const uint8_t *data, uint32_t length)
{
    uint8_t block[16];
    while (length) {
        uint32_t take = length < 16 ? length : 16;
        crypto_zero(block, 16);
        crypto_copy(block, data, take);
        xor_block(state, block);
        ghash_multiply(state, hash);
        data += take;
        length -= take;
    }
    crypto_zero(block, 16);
}

static void gcm_counter(uint8_t counter[16])
{
    for (int i = 15; i >= 12; i--) if (++counter[i]) break;
}

static void gcm_tag(const struct aes_gcm_context *context, const uint8_t nonce[12],
                    const uint8_t *additional, uint32_t additional_length,
                    const uint8_t *ciphertext, uint32_t length, uint8_t tag[16])
{
    uint8_t state[16], initial[16], lengths[16];
    crypto_zero(state, 16);
    crypto_copy(initial, nonce, 12);
    initial[12] = 0; initial[13] = 0; initial[14] = 0; initial[15] = 1;
    ghash_update(state, context->hash_key, additional, additional_length);
    ghash_update(state, context->hash_key, ciphertext, length);
    store_be64(lengths, (uint64_t)additional_length * 8);
    store_be64(lengths + 8, (uint64_t)length * 8);
    xor_block(state, lengths);
    ghash_multiply(state, context->hash_key);
    aes_encrypt_block(&context->aes, initial, tag);
    xor_block(tag, state);
    crypto_zero(state, 16);
    crypto_zero(initial, 16);
    crypto_zero(lengths, 16);
}

static void gcm_crypt(const struct aes_gcm_context *context, const uint8_t nonce[12], uint8_t *data, uint32_t length)
{
    uint8_t counter[16], stream[16];
    crypto_copy(counter, nonce, 12);
    counter[12] = 0; counter[13] = 0; counter[14] = 0; counter[15] = 1;
    while (length) {
        uint32_t take = length < 16 ? length : 16;
        gcm_counter(counter);
        aes_encrypt_block(&context->aes, counter, stream);
        for (uint32_t i = 0; i < take; i++) data[i] ^= stream[i];
        data += take;
        length -= take;
    }
    crypto_zero(counter, 16);
    crypto_zero(stream, 16);
}

int aes_gcm_init(struct aes_gcm_context *context, const uint8_t *key, uint32_t key_length)
{
    uint8_t zero[16];
    if (key_length != 16 && key_length != 32) return -1;
    if (aes_set_key(&context->aes, key, key_length)) return -1;
    crypto_zero(zero, 16);
    aes_encrypt_block(&context->aes, zero, context->hash_key);
    return 0;
}

void aes_gcm_seal(const struct aes_gcm_context *context, const uint8_t nonce[12],
                  const uint8_t *additional, uint32_t additional_length,
                  uint8_t *data, uint32_t length, uint8_t tag[16])
{
    gcm_crypt(context, nonce, data, length);
    gcm_tag(context, nonce, additional, additional_length, data, length, tag);
}

int aes_gcm_open(const struct aes_gcm_context *context, const uint8_t nonce[12],
                 const uint8_t *additional, uint32_t additional_length,
                 uint8_t *data, uint32_t length, const uint8_t tag[16])
{
    uint8_t expected[16];
    int valid;
    gcm_tag(context, nonce, additional, additional_length, data, length, expected);
    valid = crypto_equal(expected, tag, 16);
    crypto_zero(expected, 16);
    if (!valid) return 0;
    gcm_crypt(context, nonce, data, length);
    return 1;
}

int aes128_gcm_encrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16])
{
    struct aes_gcm_context context;
    if (aes_gcm_init(&context, key, 16)) return 0;
    aes_gcm_seal(&context, nonce, additional, additional_length, data, length, tag);
    crypto_zero(&context, sizeof(context));
    return 1;
}

int aes128_gcm_decrypt(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16])
{
    struct aes_gcm_context context;
    int result;
    if (aes_gcm_init(&context, key, 16)) return 0;
    result = aes_gcm_open(&context, nonce, additional, additional_length, data, length, tag);
    crypto_zero(&context, sizeof(context));
    return result;
}

int aes256_gcm_encrypt(const uint8_t key[32], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, uint8_t tag[16])
{
    struct aes_gcm_context context;
    if (aes_gcm_init(&context, key, 32)) return 0;
    aes_gcm_seal(&context, nonce, additional, additional_length, data, length, tag);
    crypto_zero(&context, sizeof(context));
    return 1;
}

int aes256_gcm_decrypt(const uint8_t key[32], const uint8_t nonce[12], const uint8_t *additional, uint32_t additional_length,
                       uint8_t *data, uint32_t length, const uint8_t tag[16])
{
    struct aes_gcm_context context;
    int result;
    if (aes_gcm_init(&context, key, 32)) return 0;
    result = aes_gcm_open(&context, nonce, additional, additional_length, data, length, tag);
    crypto_zero(&context, sizeof(context));
    return result;
}

static void chacha20_quarter(uint32_t *x, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    x[a] += x[b]; x[d] ^= x[a]; x[d] = rotate_left(x[d], 16);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = rotate_left(x[b], 12);
    x[a] += x[b]; x[d] ^= x[a]; x[d] = rotate_left(x[d], 8);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = rotate_left(x[b], 7);
}

static void chacha20_block(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12], uint8_t output[64])
{
    uint32_t initial[16], x[16];
    initial[0] = 0x61707865; initial[1] = 0x3320646e; initial[2] = 0x79622d32; initial[3] = 0x6b206574;
    for (uint32_t i = 0; i < 8; i++) initial[4 + i] = load_le32(key + i * 4);
    initial[12] = counter;
    for (uint32_t i = 0; i < 3; i++) initial[13 + i] = load_le32(nonce + i * 4);
    for (uint32_t i = 0; i < 16; i++) x[i] = initial[i];
    for (uint32_t round = 0; round < 10; round++) {
        chacha20_quarter(x, 0, 4, 8, 12); chacha20_quarter(x, 1, 5, 9, 13);
        chacha20_quarter(x, 2, 6, 10, 14); chacha20_quarter(x, 3, 7, 11, 15);
        chacha20_quarter(x, 0, 5, 10, 15); chacha20_quarter(x, 1, 6, 11, 12);
        chacha20_quarter(x, 2, 7, 8, 13); chacha20_quarter(x, 3, 4, 9, 14);
    }
    for (uint32_t i = 0; i < 16; i++) store_le32(output + i * 4, x[i] + initial[i]);
    crypto_zero(initial, sizeof(initial));
    crypto_zero(x, sizeof(x));
}

void chacha20_xor(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12], uint8_t *data, uint32_t length)
{
    uint8_t stream[64];
    while (length) {
        uint32_t take = length < 64 ? length : 64;
        chacha20_block(key, counter++, nonce, stream);
        for (uint32_t i = 0; i < take; i++) data[i] ^= stream[i];
        data += take;
        length -= take;
    }
    crypto_zero(stream, sizeof(stream));
}

void poly1305_init(struct poly1305_context *context, const uint8_t key[32])
{
    context->r[0] = load_le32(key + 0) & 0x3ffffff;
    context->r[1] = (load_le32(key + 3) >> 2) & 0x3ffff03;
    context->r[2] = (load_le32(key + 6) >> 4) & 0x3ffc0ff;
    context->r[3] = (load_le32(key + 9) >> 6) & 0x3f03fff;
    context->r[4] = (load_le32(key + 12) >> 8) & 0x00fffff;
    for (uint32_t i = 0; i < 5; i++) context->h[i] = 0;
    for (uint32_t i = 0; i < 4; i++) context->pad[i] = load_le32(key + 16 + i * 4);
    context->leftover = 0;
}

static void poly1305_blocks(struct poly1305_context *context, const uint8_t *data, uint32_t length, uint32_t high_bit)
{
    uint32_t r0 = context->r[0], r1 = context->r[1], r2 = context->r[2], r3 = context->r[3], r4 = context->r[4];
    uint32_t s1 = r1 * 5, s2 = r2 * 5, s3 = r3 * 5, s4 = r4 * 5;
    uint32_t h0 = context->h[0], h1 = context->h[1], h2 = context->h[2], h3 = context->h[3], h4 = context->h[4];
    while (length >= 16) {
        uint64_t d0, d1, d2, d3, d4;
        uint32_t carry;
        h0 += load_le32(data + 0) & 0x3ffffff;
        h1 += (load_le32(data + 3) >> 2) & 0x3ffffff;
        h2 += (load_le32(data + 6) >> 4) & 0x3ffffff;
        h3 += (load_le32(data + 9) >> 6) & 0x3ffffff;
        h4 += (load_le32(data + 12) >> 8) | high_bit;
        d0 = (uint64_t)h0 * r0 + (uint64_t)h1 * s4 + (uint64_t)h2 * s3 + (uint64_t)h3 * s2 + (uint64_t)h4 * s1;
        d1 = (uint64_t)h0 * r1 + (uint64_t)h1 * r0 + (uint64_t)h2 * s4 + (uint64_t)h3 * s3 + (uint64_t)h4 * s2;
        d2 = (uint64_t)h0 * r2 + (uint64_t)h1 * r1 + (uint64_t)h2 * r0 + (uint64_t)h3 * s4 + (uint64_t)h4 * s3;
        d3 = (uint64_t)h0 * r3 + (uint64_t)h1 * r2 + (uint64_t)h2 * r1 + (uint64_t)h3 * r0 + (uint64_t)h4 * s4;
        d4 = (uint64_t)h0 * r4 + (uint64_t)h1 * r3 + (uint64_t)h2 * r2 + (uint64_t)h3 * r1 + (uint64_t)h4 * r0;
        carry = (uint32_t)(d0 >> 26); h0 = (uint32_t)d0 & 0x3ffffff;
        d1 += carry; carry = (uint32_t)(d1 >> 26); h1 = (uint32_t)d1 & 0x3ffffff;
        d2 += carry; carry = (uint32_t)(d2 >> 26); h2 = (uint32_t)d2 & 0x3ffffff;
        d3 += carry; carry = (uint32_t)(d3 >> 26); h3 = (uint32_t)d3 & 0x3ffffff;
        d4 += carry; carry = (uint32_t)(d4 >> 26); h4 = (uint32_t)d4 & 0x3ffffff;
        h0 += carry * 5; carry = h0 >> 26; h0 &= 0x3ffffff;
        h1 += carry;
        data += 16;
        length -= 16;
    }
    context->h[0] = h0; context->h[1] = h1; context->h[2] = h2; context->h[3] = h3; context->h[4] = h4;
}

void poly1305_update(struct poly1305_context *context, const uint8_t *data, uint32_t length)
{
    if (context->leftover) {
        uint32_t want = 16 - context->leftover;
        uint32_t take = length < want ? length : want;
        crypto_copy(context->buffer + context->leftover, data, take);
        context->leftover += take;
        data += take;
        length -= take;
        if (context->leftover < 16) return;
        poly1305_blocks(context, context->buffer, 16, 1u << 24);
        context->leftover = 0;
    }
    if (length >= 16) {
        uint32_t whole = length & ~(uint32_t)15;
        poly1305_blocks(context, data, whole, 1u << 24);
        data += whole;
        length -= whole;
    }
    if (length) {
        crypto_copy(context->buffer, data, length);
        context->leftover = length;
    }
}

void poly1305_final(struct poly1305_context *context, uint8_t tag[16])
{
    uint32_t h0, h1, h2, h3, h4, g0, g1, g2, g3, g4, carry, mask;
    uint64_t f;
    if (context->leftover) {
        context->buffer[context->leftover++] = 1;
        while (context->leftover < 16) context->buffer[context->leftover++] = 0;
        poly1305_blocks(context, context->buffer, 16, 0);
    }
    h0 = context->h[0]; h1 = context->h[1]; h2 = context->h[2]; h3 = context->h[3]; h4 = context->h[4];
    carry = h1 >> 26; h1 &= 0x3ffffff;
    h2 += carry; carry = h2 >> 26; h2 &= 0x3ffffff;
    h3 += carry; carry = h3 >> 26; h3 &= 0x3ffffff;
    h4 += carry; carry = h4 >> 26; h4 &= 0x3ffffff;
    h0 += carry * 5; carry = h0 >> 26; h0 &= 0x3ffffff;
    h1 += carry;
    g0 = h0 + 5; carry = g0 >> 26; g0 &= 0x3ffffff;
    g1 = h1 + carry; carry = g1 >> 26; g1 &= 0x3ffffff;
    g2 = h2 + carry; carry = g2 >> 26; g2 &= 0x3ffffff;
    g3 = h3 + carry; carry = g3 >> 26; g3 &= 0x3ffffff;
    g4 = h4 + carry - (1u << 26);
    mask = (g4 >> 31) - 1;
    g0 &= mask; g1 &= mask; g2 &= mask; g3 &= mask; g4 &= mask;
    mask = ~mask;
    h0 = (h0 & mask) | g0; h1 = (h1 & mask) | g1; h2 = (h2 & mask) | g2;
    h3 = (h3 & mask) | g3; h4 = (h4 & mask) | g4;
    h0 = (h0 | (h1 << 26));
    h1 = ((h1 >> 6) | (h2 << 20));
    h2 = ((h2 >> 12) | (h3 << 14));
    h3 = ((h3 >> 18) | (h4 << 8));
    f = (uint64_t)h0 + context->pad[0]; store_le32(tag + 0, (uint32_t)f);
    f = (uint64_t)h1 + context->pad[1] + (f >> 32); store_le32(tag + 4, (uint32_t)f);
    f = (uint64_t)h2 + context->pad[2] + (f >> 32); store_le32(tag + 8, (uint32_t)f);
    f = (uint64_t)h3 + context->pad[3] + (f >> 32); store_le32(tag + 12, (uint32_t)f);
    crypto_zero(context, sizeof(*context));
}

static void poly1305_pad16(struct poly1305_context *context, uint32_t length)
{
    static const uint8_t zeros[16] = {0};
    if (length & 15) poly1305_update(context, zeros, 16 - (length & 15));
}

static void chacha20_poly1305_mac(const uint8_t key[32], const uint8_t nonce[12],
                                  const uint8_t *additional, uint32_t additional_length,
                                  const uint8_t *ciphertext, uint32_t length, uint8_t tag[16])
{
    struct poly1305_context mac;
    uint8_t block[64];
    uint8_t lengths[16];
    chacha20_block(key, 0, nonce, block);
    poly1305_init(&mac, block);
    poly1305_update(&mac, additional, additional_length);
    poly1305_pad16(&mac, additional_length);
    poly1305_update(&mac, ciphertext, length);
    poly1305_pad16(&mac, length);
    store_le64(lengths, additional_length);
    store_le64(lengths + 8, length);
    poly1305_update(&mac, lengths, 16);
    poly1305_final(&mac, tag);
    crypto_zero(block, sizeof(block));
}

void chacha20_poly1305_seal(const uint8_t key[32], const uint8_t nonce[12],
                            const uint8_t *additional, uint32_t additional_length,
                            uint8_t *data, uint32_t length, uint8_t tag[16])
{
    chacha20_xor(key, 1, nonce, data, length);
    chacha20_poly1305_mac(key, nonce, additional, additional_length, data, length, tag);
}

int chacha20_poly1305_open(const uint8_t key[32], const uint8_t nonce[12],
                           const uint8_t *additional, uint32_t additional_length,
                           uint8_t *data, uint32_t length, const uint8_t tag[16])
{
    uint8_t expected[16];
    int valid;
    chacha20_poly1305_mac(key, nonce, additional, additional_length, data, length, expected);
    valid = crypto_equal(expected, tag, 16);
    crypto_zero(expected, 16);
    if (!valid) return 0;
    chacha20_xor(key, 1, nonce, data, length);
    return 1;
}

typedef long long curve_limb;
typedef curve_limb curve_field[16];

static const curve_field curve_constant_121665 = {0xDB41, 1};

static void curve_carry(curve_field value)
{
    for (int i = 0; i < 16; i++) {
        curve_limb carry;
        value[i] += 65536;
        carry = value[i] >> 16;
        value[(i + 1) & 15] += carry - 1 + (i == 15 ? 37 * (carry - 1) : 0);
        value[i] -= carry * 65536;
    }
}

static void curve_select(curve_field left, curve_field right, int select)
{
    curve_limb mask = ~(curve_limb)(select - 1);
    for (int i = 0; i < 16; i++) {
        curve_limb changed = mask & (left[i] ^ right[i]);
        left[i] ^= changed;
        right[i] ^= changed;
    }
}

static void curve_pack(uint8_t output[32], curve_field input)
{
    curve_field copy, candidate;
    for (int i = 0; i < 16; i++) copy[i] = input[i];
    curve_carry(copy); curve_carry(copy); curve_carry(copy);
    for (int pass = 0; pass < 2; pass++) {
        int select;
        candidate[0] = copy[0] - 0xffed;
        for (int i = 1; i < 15; i++) {
            candidate[i] = copy[i] - 0xffff - ((candidate[i - 1] >> 16) & 1);
            candidate[i - 1] &= 0xffff;
        }
        candidate[15] = copy[15] - 0x7fff - ((candidate[14] >> 16) & 1);
        select = (int)((candidate[15] >> 16) & 1);
        candidate[14] &= 0xffff;
        curve_select(copy, candidate, 1 - select);
    }
    for (int i = 0; i < 16; i++) {
        output[2 * i] = (uint8_t)copy[i];
        output[2 * i + 1] = (uint8_t)(copy[i] >> 8);
    }
    crypto_zero(copy, sizeof(copy));
    crypto_zero(candidate, sizeof(candidate));
}

static void curve_unpack(curve_field output, const uint8_t input[32])
{
    for (int i = 0; i < 16; i++) output[i] = input[2 * i] + ((curve_limb)input[2 * i + 1] << 8);
    output[15] &= 0x7fff;
}

static void curve_add(curve_field output, const curve_field left, const curve_field right)
{
    for (int i = 0; i < 16; i++) output[i] = left[i] + right[i];
}

static void curve_sub(curve_field output, const curve_field left, const curve_field right)
{
    for (int i = 0; i < 16; i++) output[i] = left[i] - right[i];
}

static void curve_mul(curve_field output, const curve_field left, const curve_field right)
{
    curve_limb temporary[31];
    for (int i = 0; i < 31; i++) temporary[i] = 0;
    for (int i = 0; i < 16; i++) for (int j = 0; j < 16; j++) temporary[i + j] += left[i] * right[j];
    for (int i = 0; i < 15; i++) temporary[i] += 38 * temporary[i + 16];
    for (int i = 0; i < 16; i++) output[i] = temporary[i];
    curve_carry(output);
    curve_carry(output);
    crypto_zero(temporary, sizeof(temporary));
}

static void curve_square(curve_field output, const curve_field input)
{
    curve_mul(output, input, input);
}

static void curve_inverse(curve_field output, const curve_field input)
{
    curve_field value;
    for (int i = 0; i < 16; i++) value[i] = input[i];
    for (int bit = 253; bit >= 0; bit--) {
        curve_square(value, value);
        if (bit != 2 && bit != 4) curve_mul(value, value, input);
    }
    for (int i = 0; i < 16; i++) output[i] = value[i];
    crypto_zero(value, sizeof(value));
}

static void curve_scalar_mult(uint8_t output[32], const uint8_t scalar_input[32], const uint8_t point_input[32])
{
    uint8_t scalar[32];
    curve_field point, a, b, c, d, e, f;
    crypto_copy(scalar, scalar_input, 32);
    scalar[0] &= 248;
    scalar[31] &= 127;
    scalar[31] |= 64;
    curve_unpack(point, point_input);
    for (int i = 0; i < 16; i++) { a[i] = 0; d[i] = 0; b[i] = point[i]; c[i] = 0; e[i] = 0; f[i] = 0; }
    a[0] = 1;
    d[0] = 1;
    for (int bit = 254; bit >= 0; bit--) {
        int selected = (scalar[bit >> 3] >> (bit & 7)) & 1;
        curve_select(a, b, selected);
        curve_select(c, d, selected);
        curve_add(e, a, c);
        curve_sub(a, a, c);
        curve_add(c, b, d);
        curve_sub(b, b, d);
        curve_square(d, e);
        curve_square(f, a);
        curve_mul(a, c, a);
        curve_mul(c, b, e);
        curve_add(e, a, c);
        curve_sub(a, a, c);
        curve_square(b, a);
        curve_sub(c, d, f);
        curve_mul(a, c, curve_constant_121665);
        curve_add(a, a, d);
        curve_mul(c, c, a);
        curve_mul(a, d, f);
        curve_mul(d, b, point);
        curve_square(b, e);
        curve_select(a, b, selected);
        curve_select(c, d, selected);
    }
    curve_inverse(c, c);
    curve_mul(a, a, c);
    curve_pack(output, a);
    crypto_zero(scalar, sizeof(scalar));
    crypto_zero(point, sizeof(point));
    crypto_zero(a, sizeof(a));
    crypto_zero(b, sizeof(b));
    crypto_zero(c, sizeof(c));
    crypto_zero(d, sizeof(d));
    crypto_zero(e, sizeof(e));
    crypto_zero(f, sizeof(f));
}

void x25519_public_key(const uint8_t private_key[32], uint8_t public_key[32])
{
    static const uint8_t base_point[32] = {9};
    curve_scalar_mult(public_key, private_key, base_point);
}

int x25519_shared_secret(const uint8_t private_key[32], const uint8_t peer_key[32], uint8_t shared_secret[32])
{
    uint8_t accumulated = 0;
    curve_scalar_mult(shared_secret, private_key, peer_key);
    for (int i = 0; i < 32; i++) accumulated |= shared_secret[i];
    if (!accumulated) return 0;
    return 1;
}

static void drbg_update(struct hmac_drbg_context *context, const uint8_t *data, uint32_t length)
{
    struct hmac_context hmac;
    for (uint8_t separator = 0; separator < 2; separator++) {
        if (separator == 1 && !length) break;
        hmac_begin(&hmac, HASH_SHA256, context->key, 32);
        hmac_feed(&hmac, context->value, 32);
        hmac_feed(&hmac, &separator, 1);
        if (length) hmac_feed(&hmac, data, length);
        hmac_end(&hmac, context->key);
        hmac_compute(HASH_SHA256, context->key, 32, context->value, 32, context->value);
    }
}

void hmac_drbg_instantiate(struct hmac_drbg_context *context, const uint8_t *seed, uint32_t seed_length)
{
    for (uint32_t i = 0; i < 32; i++) { context->key[i] = 0; context->value[i] = 1; }
    drbg_update(context, seed, seed_length);
    context->generate_count = 0;
    context->seeded = 1;
}

void hmac_drbg_reseed(struct hmac_drbg_context *context, const uint8_t *seed, uint32_t seed_length)
{
    drbg_update(context, seed, seed_length);
    context->generate_count = 0;
}

void hmac_drbg_generate(struct hmac_drbg_context *context, uint8_t *output, uint32_t length)
{
    while (length) {
        uint32_t take = length < 32 ? length : 32;
        hmac_compute(HASH_SHA256, context->key, 32, context->value, 32, context->value);
        crypto_copy(output, context->value, take);
        output += take;
        length -= take;
    }
    drbg_update(context, 0, 0);
    context->generate_count++;
}

static struct hmac_drbg_context global_drbg;
static crypto_entropy_source global_entropy_source;

void crypto_set_entropy_source(crypto_entropy_source source)
{
    global_entropy_source = source;
    crypto_zero(&global_drbg, sizeof(global_drbg));
}

int crypto_random_bytes(uint8_t *output, uint32_t length)
{
    uint8_t seed[48];
    if (!global_entropy_source) return -1;
    if (!global_drbg.seeded) {
        if (!global_entropy_source(seed, sizeof(seed))) return -1;
        hmac_drbg_instantiate(&global_drbg, seed, sizeof(seed));
        crypto_zero(seed, sizeof(seed));
    } else if (global_drbg.generate_count >= 4096) {
        if (global_entropy_source(seed, 32)) hmac_drbg_reseed(&global_drbg, seed, 32);
        crypto_zero(seed, sizeof(seed));
    }
    hmac_drbg_generate(&global_drbg, output, length);
    return 0;
}

#define BN_MAX_LIMBS 132
#define EC_MAX_LIMBS 12

struct bn_mont {
    uint32_t limbs;
    uint32_t n0inv;
    uint32_t n[BN_MAX_LIMBS];
    uint32_t one[BN_MAX_LIMBS];
    uint32_t rr[BN_MAX_LIMBS];
};

static int bn_load(uint32_t *out, uint32_t limbs, const uint8_t *data, uint32_t length)
{
    while (length && *data == 0) { data++; length--; }
    if (length > limbs * 4) return -1;
    for (uint32_t i = 0; i < limbs; i++) out[i] = 0;
    for (uint32_t i = 0; i < length; i++) out[i / 4] |= (uint32_t)data[length - 1 - i] << (8 * (i % 4));
    return 0;
}

static void bn_store(uint8_t *out, uint32_t length, const uint32_t *value, uint32_t limbs)
{
    for (uint32_t i = 0; i < length; i++)
        out[length - 1 - i] = (i / 4 < limbs) ? (uint8_t)(value[i / 4] >> (8 * (i % 4))) : 0;
}

static int bn_compare(const uint32_t *left, const uint32_t *right, uint32_t limbs)
{
    for (uint32_t i = limbs; i > 0; i--) {
        if (left[i - 1] > right[i - 1]) return 1;
        if (left[i - 1] < right[i - 1]) return -1;
    }
    return 0;
}

static int bn_is_zero(const uint32_t *value, uint32_t limbs)
{
    uint32_t accumulated = 0;
    for (uint32_t i = 0; i < limbs; i++) accumulated |= value[i];
    return accumulated == 0;
}

static uint32_t bn_sub(uint32_t *result, const uint32_t *left, const uint32_t *right, uint32_t limbs)
{
    uint32_t borrow = 0;
    for (uint32_t i = 0; i < limbs; i++) {
        uint64_t difference = (uint64_t)left[i] - right[i] - borrow;
        result[i] = (uint32_t)difference;
        borrow = (uint32_t)(difference >> 32) & 1;
    }
    return borrow;
}

static uint32_t bn_add(uint32_t *result, const uint32_t *left, const uint32_t *right, uint32_t limbs)
{
    uint32_t carry = 0;
    for (uint32_t i = 0; i < limbs; i++) {
        uint64_t sum = (uint64_t)left[i] + right[i] + carry;
        result[i] = (uint32_t)sum;
        carry = (uint32_t)(sum >> 32);
    }
    return carry;
}

static void bn_copy(uint32_t *destination, const uint32_t *source, uint32_t limbs)
{
    for (uint32_t i = 0; i < limbs; i++) destination[i] = source[i];
}

static void bn_double_modulo(uint32_t *value, const uint32_t *modulus, uint32_t limbs)
{
    uint32_t carry = value[limbs - 1] >> 31;
    for (uint32_t i = limbs - 1; i > 0; i--) value[i] = (value[i] << 1) | (value[i - 1] >> 31);
    value[0] <<= 1;
    if (carry || bn_compare(value, modulus, limbs) >= 0) bn_sub(value, value, modulus, limbs);
}

static int mont_init(struct bn_mont *mont, const uint32_t *modulus, uint32_t limbs)
{
    uint32_t inverse;
    if (limbs == 0 || limbs > BN_MAX_LIMBS || !(modulus[0] & 1) || modulus[limbs - 1] == 0) return -1;
    mont->limbs = limbs;
    bn_copy(mont->n, modulus, limbs);
    inverse = modulus[0];
    for (uint32_t i = 0; i < 5; i++) inverse *= 2 - modulus[0] * inverse;
    mont->n0inv = (uint32_t)0 - inverse;
    for (uint32_t i = 0; i < limbs; i++) mont->one[i] = 0;
    mont->one[0] = 1;
    for (uint32_t i = 0; i < limbs * 32; i++) bn_double_modulo(mont->one, mont->n, limbs);
    bn_copy(mont->rr, mont->one, limbs);
    for (uint32_t i = 0; i < limbs * 32; i++) bn_double_modulo(mont->rr, mont->n, limbs);
    return 0;
}

static void mont_mul(const struct bn_mont *mont, uint32_t *result, const uint32_t *left, const uint32_t *right)
{
    uint32_t t[BN_MAX_LIMBS + 2];
    uint32_t limbs = mont->limbs;
    for (uint32_t i = 0; i < limbs + 2; i++) t[i] = 0;
    for (uint32_t i = 0; i < limbs; i++) {
        uint64_t sum;
        uint64_t carry = 0;
        uint32_t multiplier;
        for (uint32_t j = 0; j < limbs; j++) {
            sum = (uint64_t)t[j] + (uint64_t)left[j] * right[i] + carry;
            t[j] = (uint32_t)sum;
            carry = sum >> 32;
        }
        sum = (uint64_t)t[limbs] + carry;
        t[limbs] = (uint32_t)sum;
        t[limbs + 1] = (uint32_t)(sum >> 32);
        multiplier = t[0] * mont->n0inv;
        sum = (uint64_t)t[0] + (uint64_t)multiplier * mont->n[0];
        carry = sum >> 32;
        for (uint32_t j = 1; j < limbs; j++) {
            sum = (uint64_t)t[j] + (uint64_t)multiplier * mont->n[j] + carry;
            t[j - 1] = (uint32_t)sum;
            carry = sum >> 32;
        }
        sum = (uint64_t)t[limbs] + carry;
        t[limbs - 1] = (uint32_t)sum;
        t[limbs] = t[limbs + 1] + (uint32_t)(sum >> 32);
    }
    if (t[limbs] || bn_compare(t, mont->n, limbs) >= 0) bn_sub(t, t, mont->n, limbs);
    bn_copy(result, t, limbs);
}

static void mont_to(const struct bn_mont *mont, uint32_t *result, const uint32_t *value)
{
    mont_mul(mont, result, value, mont->rr);
}

static void mont_from(const struct bn_mont *mont, uint32_t *result, const uint32_t *value)
{
    uint32_t plain_one[BN_MAX_LIMBS];
    for (uint32_t i = 0; i < mont->limbs; i++) plain_one[i] = 0;
    plain_one[0] = 1;
    mont_mul(mont, result, value, plain_one);
}

static void mont_pow(const struct bn_mont *mont, uint32_t *result, const uint32_t *base,
                     const uint8_t *exponent, uint32_t exponent_length)
{
    uint32_t accumulator[BN_MAX_LIMBS];
    bn_copy(accumulator, mont->one, mont->limbs);
    for (uint32_t i = 0; i < exponent_length; i++)
        for (int bit = 7; bit >= 0; bit--) {
            mont_mul(mont, accumulator, accumulator, accumulator);
            if ((exponent[i] >> bit) & 1) mont_mul(mont, accumulator, accumulator, base);
        }
    bn_copy(result, accumulator, mont->limbs);
}

static uint32_t strip_zeros(const uint8_t **data, uint32_t length)
{
    while (length && **data == 0) { (*data)++; length--; }
    return length;
}

static int rsa_raw_public(const uint8_t *modulus, uint32_t modulus_length, const uint8_t *exponent,
                          uint32_t exponent_length, const uint8_t *signature, uint32_t signature_length,
                          uint8_t *output)
{
    struct bn_mont mont;
    uint32_t n[BN_MAX_LIMBS], x[BN_MAX_LIMBS], b[BN_MAX_LIMBS];
    uint32_t limbs;
    modulus_length = strip_zeros(&modulus, modulus_length);
    exponent_length = strip_zeros(&exponent, exponent_length);
    if (modulus_length < 256 || modulus_length > BN_MAX_LIMBS * 4 - 4) return -1;
    if (exponent_length == 0 || exponent_length > 4 || !(exponent[exponent_length - 1] & 1)) return -1;
    if (exponent_length == 1 && exponent[0] < 3) return -1;
    if (signature_length != modulus_length) return -1;
    limbs = (modulus_length + 3) / 4;
    if (bn_load(n, limbs, modulus, modulus_length) || bn_load(x, limbs, signature, signature_length)) return -1;
    if (mont_init(&mont, n, limbs)) return -1;
    if (bn_compare(x, n, limbs) >= 0) return -1;
    mont_to(&mont, b, x);
    mont_pow(&mont, x, b, exponent, exponent_length);
    mont_from(&mont, x, x);
    bn_store(output, modulus_length, x, limbs);
    return (int)modulus_length;
}

static int rsa_verify_pkcs1(const uint8_t *modulus, uint32_t modulus_length, const uint8_t *exponent,
                            uint32_t exponent_length, const uint8_t *signature, uint32_t signature_length,
                            enum hash_algorithm algorithm, const uint8_t *digest)
{
    static const uint8_t prefix256[19] = {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};
    static const uint8_t prefix384[19] = {0x30,0x41,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,0x05,0x00,0x04,0x30};
    static const uint8_t prefix512[19] = {0x30,0x51,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0x04,0x40};
    uint8_t encoded[BN_MAX_LIMBS * 4];
    uint8_t expected[BN_MAX_LIMBS * 4];
    const uint8_t *prefix;
    uint32_t digest_size = hash_digest_size(algorithm);
    uint32_t total;
    int length;
    if (algorithm == HASH_SHA256) prefix = prefix256;
    else if (algorithm == HASH_SHA384) prefix = prefix384;
    else if (algorithm == HASH_SHA512) prefix = prefix512;
    else return 0;
    length = rsa_raw_public(modulus, modulus_length, exponent, exponent_length, signature, signature_length, encoded);
    if (length < 0) return 0;
    total = 19 + digest_size;
    if ((uint32_t)length < total + 11) return 0;
    expected[0] = 0;
    expected[1] = 1;
    for (uint32_t i = 2; i < (uint32_t)length - total - 1; i++) expected[i] = 0xff;
    expected[length - total - 1] = 0;
    crypto_copy(expected + length - total, prefix, 19);
    crypto_copy(expected + length - digest_size, digest, digest_size);
    return crypto_equal(encoded, expected, (uint32_t)length);
}

static void mgf1(enum hash_algorithm algorithm, const uint8_t *seed, uint32_t seed_length,
                 uint8_t *output, uint32_t output_length)
{
    uint8_t block[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(algorithm);
    uint32_t produced = 0;
    for (uint32_t counter = 0; produced < output_length; counter++) {
        struct hash_context context;
        uint8_t counter_bytes[4];
        uint32_t take = output_length - produced < digest_size ? output_length - produced : digest_size;
        counter_bytes[0] = (uint8_t)(counter >> 24); counter_bytes[1] = (uint8_t)(counter >> 16);
        counter_bytes[2] = (uint8_t)(counter >> 8); counter_bytes[3] = (uint8_t)counter;
        hash_init(&context, algorithm);
        hash_update(&context, seed, seed_length);
        hash_update(&context, counter_bytes, 4);
        hash_final(&context, block);
        crypto_copy(output + produced, block, take);
        produced += take;
    }
}

static int rsa_verify_pss(const uint8_t *modulus, uint32_t modulus_length, const uint8_t *exponent,
                          uint32_t exponent_length, const uint8_t *signature, uint32_t signature_length,
                          enum hash_algorithm algorithm, const uint8_t *message_hash)
{
    uint8_t full[BN_MAX_LIMBS * 4];
    uint8_t database[BN_MAX_LIMBS * 4];
    uint8_t check[8 + 2 * HASH_MAX_DIGEST_SIZE];
    uint8_t expected[HASH_MAX_DIGEST_SIZE];
    struct hash_context context;
    const uint8_t *stripped = modulus;
    uint32_t stripped_length = strip_zeros(&stripped, modulus_length);
    uint32_t digest_size = hash_digest_size(algorithm);
    uint32_t modulus_bits, em_bits, em_length, top_bits, database_length;
    const uint8_t *em;
    int length;
    uint8_t top = stripped[0];
    if (!digest_size) return 0;
    length = rsa_raw_public(modulus, modulus_length, exponent, exponent_length, signature, signature_length, full);
    if (length < 0) return 0;
    modulus_bits = (stripped_length - 1) * 8;
    for (uint32_t i = 0; i < 8; i++) if (top >> i) modulus_bits++;
    em_bits = modulus_bits - 1;
    em_length = (em_bits + 7) / 8;
    em = full + (stripped_length - em_length);
    for (uint32_t i = 0; i < stripped_length - em_length; i++) if (full[i]) return 0;
    if (em_length < digest_size * 2 + 2 || em[em_length - 1] != 0xbc) return 0;
    database_length = em_length - digest_size - 1;
    top_bits = 8 * em_length - em_bits;
    if (top_bits && (em[0] >> (8 - top_bits))) return 0;
    mgf1(algorithm, em + database_length, digest_size, database, database_length);
    for (uint32_t i = 0; i < database_length; i++) database[i] ^= em[i];
    if (top_bits) database[0] &= (uint8_t)(0xff >> top_bits);
    for (uint32_t i = 0; i + digest_size + 1 < database_length; i++) if (database[i]) return 0;
    if (database[database_length - digest_size - 1] != 1) return 0;
    crypto_zero(check, 8);
    crypto_copy(check + 8, message_hash, digest_size);
    crypto_copy(check + 8 + digest_size, database + database_length - digest_size, digest_size);
    hash_init(&context, algorithm);
    hash_update(&context, check, 8 + 2 * digest_size);
    hash_final(&context, expected);
    return crypto_equal(expected, em + database_length, digest_size);
}

struct ec_curve {
    uint32_t size;
    const uint8_t *p;
    const uint8_t *n;
    const uint8_t *b;
    const uint8_t *gx;
    const uint8_t *gy;
};

struct ec_point {
    uint32_t x[EC_MAX_LIMBS];
    uint32_t y[EC_MAX_LIMBS];
    uint32_t z[EC_MAX_LIMBS];
};

struct ec_context {
    const struct ec_curve *curve;
    uint32_t limbs;
    struct bn_mont fp;
    struct bn_mont fn;
    uint32_t b[EC_MAX_LIMBS];
    uint32_t gx[EC_MAX_LIMBS];
    uint32_t gy[EC_MAX_LIMBS];
};

static const uint8_t p256_p[32] = {0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static const uint8_t p256_n[32] = {0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xbc,0xe6,0xfa,0xad,0xa7,0x17,0x9e,0x84,0xf3,0xb9,0xca,0xc2,0xfc,0x63,0x25,0x51};
static const uint8_t p256_b[32] = {0x5a,0xc6,0x35,0xd8,0xaa,0x3a,0x93,0xe7,0xb3,0xeb,0xbd,0x55,0x76,0x98,0x86,0xbc,0x65,0x1d,0x06,0xb0,0xcc,0x53,0xb0,0xf6,0x3b,0xce,0x3c,0x3e,0x27,0xd2,0x60,0x4b};
static const uint8_t p256_gx[32] = {0x6b,0x17,0xd1,0xf2,0xe1,0x2c,0x42,0x47,0xf8,0xbc,0xe6,0xe5,0x63,0xa4,0x40,0xf2,0x77,0x03,0x7d,0x81,0x2d,0xeb,0x33,0xa0,0xf4,0xa1,0x39,0x45,0xd8,0x98,0xc2,0x96};
static const uint8_t p256_gy[32] = {0x4f,0xe3,0x42,0xe2,0xfe,0x1a,0x7f,0x9b,0x8e,0xe7,0xeb,0x4a,0x7c,0x0f,0x9e,0x16,0x2b,0xce,0x33,0x57,0x6b,0x31,0x5e,0xce,0xcb,0xb6,0x40,0x68,0x37,0xbf,0x51,0xf5};
static const struct ec_curve ec_p256 = {32, p256_p, p256_n, p256_b, p256_gx, p256_gy};
static const uint8_t p384_p[48] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t p384_n[48] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc7,0x63,0x4d,0x81,0xf4,0x37,0x2d,0xdf,0x58,0x1a,0x0d,0xb2,0x48,0xb0,0xa7,0x7a,0xec,0xec,0x19,0x6a,0xcc,0xc5,0x29,0x73};
static const uint8_t p384_b[48] = {0xb3,0x31,0x2f,0xa7,0xe2,0x3e,0xe7,0xe4,0x98,0x8e,0x05,0x6b,0xe3,0xf8,0x2d,0x19,0x18,0x1d,0x9c,0x6e,0xfe,0x81,0x41,0x12,0x03,0x14,0x08,0x8f,0x50,0x13,0x87,0x5a,0xc6,0x56,0x39,0x8d,0x8a,0x2e,0xd1,0x9d,0x2a,0x85,0xc8,0xed,0xd3,0xec,0x2a,0xef};
static const uint8_t p384_gx[48] = {0xaa,0x87,0xca,0x22,0xbe,0x8b,0x05,0x37,0x8e,0xb1,0xc7,0x1e,0xf3,0x20,0xad,0x74,0x6e,0x1d,0x3b,0x62,0x8b,0xa7,0x9b,0x98,0x59,0xf7,0x41,0xe0,0x82,0x54,0x2a,0x38,0x55,0x02,0xf2,0x5d,0xbf,0x55,0x29,0x6c,0x3a,0x54,0x5e,0x38,0x72,0x76,0x0a,0xb7};
static const uint8_t p384_gy[48] = {0x36,0x17,0xde,0x4a,0x96,0x26,0x2c,0x6f,0x5d,0x9e,0x98,0xbf,0x92,0x92,0xdc,0x29,0xf8,0xf4,0x1d,0xbd,0x28,0x9a,0x14,0x7c,0xe9,0xda,0x31,0x13,0xb5,0xf0,0xb8,0xc0,0x0a,0x60,0xb1,0xce,0x1d,0x7e,0x81,0x9d,0x7a,0x43,0x1d,0x7c,0x90,0xea,0x0e,0x5f};
static const struct ec_curve ec_p384 = {48, p384_p, p384_n, p384_b, p384_gx, p384_gy};

static int ec_init(struct ec_context *context, const struct ec_curve *curve)
{
    uint32_t value[EC_MAX_LIMBS];
    uint32_t limbs = curve->size / 4;
    context->curve = curve;
    context->limbs = limbs;
    if (bn_load(value, limbs, curve->p, curve->size) || mont_init(&context->fp, value, limbs)) return -1;
    if (bn_load(value, limbs, curve->n, curve->size) || mont_init(&context->fn, value, limbs)) return -1;
    bn_load(value, limbs, curve->b, curve->size); mont_to(&context->fp, context->b, value);
    bn_load(value, limbs, curve->gx, curve->size); mont_to(&context->fp, context->gx, value);
    bn_load(value, limbs, curve->gy, curve->size); mont_to(&context->fp, context->gy, value);
    return 0;
}

static void fe_add(const struct bn_mont *m, uint32_t *r, const uint32_t *a, const uint32_t *b)
{
    uint32_t t[EC_MAX_LIMBS];
    uint32_t carry = bn_add(t, a, b, m->limbs);
    if (carry || bn_compare(t, m->n, m->limbs) >= 0) bn_sub(t, t, m->n, m->limbs);
    bn_copy(r, t, m->limbs);
}

static void fe_sub(const struct bn_mont *m, uint32_t *r, const uint32_t *a, const uint32_t *b)
{
    uint32_t t[EC_MAX_LIMBS];
    if (bn_sub(t, a, b, m->limbs)) bn_add(t, t, m->n, m->limbs);
    bn_copy(r, t, m->limbs);
}

static void point_copy(struct ec_point *destination, const struct ec_point *source, uint32_t limbs)
{
    bn_copy(destination->x, source->x, limbs);
    bn_copy(destination->y, source->y, limbs);
    bn_copy(destination->z, source->z, limbs);
}

static void point_infinity(struct ec_context *context, struct ec_point *point)
{
    bn_copy(point->x, context->fp.one, context->limbs);
    bn_copy(point->y, context->fp.one, context->limbs);
    for (uint32_t i = 0; i < context->limbs; i++) point->z[i] = 0;
}

static void ec_double(const struct ec_context *c, struct ec_point *r, const struct ec_point *p)
{
    const struct bn_mont *m = &c->fp;
    uint32_t delta[EC_MAX_LIMBS], gamma[EC_MAX_LIMBS], beta[EC_MAX_LIMBS], alpha[EC_MAX_LIMBS];
    uint32_t t1[EC_MAX_LIMBS], t2[EC_MAX_LIMBS], x3[EC_MAX_LIMBS], y3[EC_MAX_LIMBS], z3[EC_MAX_LIMBS];
    if (bn_is_zero(p->z, c->limbs)) { point_copy(r, p, c->limbs); return; }
    mont_mul(m, delta, p->z, p->z);
    mont_mul(m, gamma, p->y, p->y);
    mont_mul(m, beta, p->x, gamma);
    fe_sub(m, t1, p->x, delta);
    fe_add(m, t2, p->x, delta);
    mont_mul(m, alpha, t1, t2);
    fe_add(m, t1, alpha, alpha);
    fe_add(m, alpha, t1, alpha);
    mont_mul(m, x3, alpha, alpha);
    fe_add(m, t1, beta, beta);
    fe_add(m, t1, t1, t1);
    fe_add(m, t2, t1, t1);
    fe_sub(m, x3, x3, t2);
    fe_add(m, z3, p->y, p->z);
    mont_mul(m, z3, z3, z3);
    fe_sub(m, z3, z3, gamma);
    fe_sub(m, z3, z3, delta);
    fe_sub(m, t1, t1, x3);
    mont_mul(m, y3, alpha, t1);
    mont_mul(m, t2, gamma, gamma);
    fe_add(m, t2, t2, t2);
    fe_add(m, t2, t2, t2);
    fe_add(m, t2, t2, t2);
    fe_sub(m, y3, y3, t2);
    bn_copy(r->x, x3, c->limbs);
    bn_copy(r->y, y3, c->limbs);
    bn_copy(r->z, z3, c->limbs);
}

static void ec_add(struct ec_context *c, struct ec_point *r, const struct ec_point *p, const struct ec_point *q)
{
    const struct bn_mont *m = &c->fp;
    uint32_t z1z1[EC_MAX_LIMBS], z2z2[EC_MAX_LIMBS], u1[EC_MAX_LIMBS], u2[EC_MAX_LIMBS];
    uint32_t s1[EC_MAX_LIMBS], s2[EC_MAX_LIMBS], h[EC_MAX_LIMBS], i[EC_MAX_LIMBS], j[EC_MAX_LIMBS];
    uint32_t rr[EC_MAX_LIMBS], v[EC_MAX_LIMBS], x3[EC_MAX_LIMBS], y3[EC_MAX_LIMBS], z3[EC_MAX_LIMBS], t[EC_MAX_LIMBS];
    if (bn_is_zero(p->z, c->limbs)) { point_copy(r, q, c->limbs); return; }
    if (bn_is_zero(q->z, c->limbs)) { point_copy(r, p, c->limbs); return; }
    mont_mul(m, z1z1, p->z, p->z);
    mont_mul(m, z2z2, q->z, q->z);
    mont_mul(m, u1, p->x, z2z2);
    mont_mul(m, u2, q->x, z1z1);
    mont_mul(m, t, q->z, z2z2);
    mont_mul(m, s1, p->y, t);
    mont_mul(m, t, p->z, z1z1);
    mont_mul(m, s2, q->y, t);
    fe_sub(m, h, u2, u1);
    fe_sub(m, rr, s2, s1);
    if (bn_is_zero(h, c->limbs)) {
        if (bn_is_zero(rr, c->limbs)) ec_double(c, r, p);
        else point_infinity(c, r);
        return;
    }
    fe_add(m, rr, rr, rr);
    fe_add(m, t, h, h);
    mont_mul(m, i, t, t);
    mont_mul(m, j, h, i);
    mont_mul(m, v, u1, i);
    mont_mul(m, x3, rr, rr);
    fe_sub(m, x3, x3, j);
    fe_sub(m, x3, x3, v);
    fe_sub(m, x3, x3, v);
    fe_sub(m, t, v, x3);
    mont_mul(m, y3, rr, t);
    mont_mul(m, t, s1, j);
    fe_add(m, t, t, t);
    fe_sub(m, y3, y3, t);
    fe_add(m, z3, p->z, q->z);
    mont_mul(m, z3, z3, z3);
    fe_sub(m, z3, z3, z1z1);
    fe_sub(m, z3, z3, z2z2);
    mont_mul(m, z3, z3, h);
    bn_copy(r->x, x3, c->limbs);
    bn_copy(r->y, y3, c->limbs);
    bn_copy(r->z, z3, c->limbs);
}

static void ec_multiply(struct ec_context *c, struct ec_point *r, const uint32_t *scalar, const struct ec_point *p)
{
    struct ec_point accumulator;
    point_infinity(c, &accumulator);
    for (uint32_t bit = c->limbs * 32; bit > 0; bit--) {
        ec_double(c, &accumulator, &accumulator);
        if ((scalar[(bit - 1) / 32] >> ((bit - 1) % 32)) & 1) ec_add(c, &accumulator, &accumulator, p);
    }
    point_copy(r, &accumulator, c->limbs);
}

static void bytes_subtract_small(uint8_t *value, uint32_t length, uint32_t amount)
{
    for (uint32_t i = length; i > 0 && amount; i--) {
        uint32_t current = value[i - 1];
        if (current >= amount) { value[i - 1] = (uint8_t)(current - amount); amount = 0; }
        else { value[i - 1] = (uint8_t)(current + 256 - amount); amount = 1; }
    }
}

static int ec_to_affine_x(struct ec_context *c, uint32_t *x_out, const struct ec_point *p)
{
    uint8_t exponent[48];
    uint32_t inverse[EC_MAX_LIMBS], square[EC_MAX_LIMBS], x[EC_MAX_LIMBS];
    if (bn_is_zero(p->z, c->limbs)) return -1;
    crypto_copy(exponent, c->curve->p, c->curve->size);
    bytes_subtract_small(exponent, c->curve->size, 2);
    mont_pow(&c->fp, inverse, p->z, exponent, c->curve->size);
    mont_mul(&c->fp, square, inverse, inverse);
    mont_mul(&c->fp, x, p->x, square);
    mont_from(&c->fp, x_out, x);
    return 0;
}

static int der_integer_to_fixed(const uint8_t *data, uint32_t length, uint8_t *out, uint32_t size)
{
    if (length == 0 || (data[0] & 0x80)) return -1;
    length = strip_zeros(&data, length);
    if (length == 0 || length > size) return -1;
    crypto_zero(out, size);
    crypto_copy(out + size - length, data, length);
    return 0;
}

struct der_item {
    uint8_t tag;
    const uint8_t *content;
    uint32_t length;
    const uint8_t *raw;
    uint32_t raw_length;
};

static int der_next(const uint8_t *data, uint32_t length, uint32_t *offset, struct der_item *item)
{
    uint32_t p = *offset;
    uint32_t header;
    uint32_t value;
    if (p + 2 > length || (data[p] & 0x1f) == 0x1f) return -1;
    item->tag = data[p];
    if (data[p + 1] < 0x80) {
        value = data[p + 1];
        header = 2;
    } else {
        uint32_t count = data[p + 1] & 0x7f;
        if (count == 0 || count > 3 || p + 2 + count > length || data[p + 2] == 0) return -1;
        value = 0;
        for (uint32_t i = 0; i < count; i++) value = (value << 8) | data[p + 2 + i];
        if (value < 0x80) return -1;
        header = 2 + count;
    }
    if (p + header > length || value > length - p - header) return -1;
    item->raw = data + p;
    item->raw_length = header + value;
    item->content = data + p + header;
    item->length = value;
    *offset = p + header + value;
    return 0;
}

static int der_child(const struct der_item *parent, uint32_t *offset, uint8_t tag, struct der_item *item)
{
    if (der_next(parent->content, parent->length, offset, item)) return -1;
    return item->tag == tag ? 0 : -1;
}

static int ecdsa_verify(const uint8_t *point, uint32_t point_length, const struct ec_curve *curve,
                        const uint8_t *digest, uint32_t digest_length,
                        const uint8_t *signature, uint32_t signature_length)
{
    struct ec_context context;
    struct ec_point q, g, a, b;
    struct der_item sequence, integer_r, integer_s;
    uint32_t offset = 0, child = 0;
    uint8_t r_bytes[48], s_bytes[48], e_bytes[48], exponent[48];
    uint32_t r[EC_MAX_LIMBS], s[EC_MAX_LIMBS], e[EC_MAX_LIMBS], s_mont[EC_MAX_LIMBS], w[EC_MAX_LIMBS];
    uint32_t e_mont[EC_MAX_LIMBS], u1[EC_MAX_LIMBS], u2[EC_MAX_LIMBS], x[EC_MAX_LIMBS], px[EC_MAX_LIMBS], py[EC_MAX_LIMBS];
    uint32_t lhs[EC_MAX_LIMBS], rhs[EC_MAX_LIMBS], t[EC_MAX_LIMBS], p_limbs[EC_MAX_LIMBS], n_limbs[EC_MAX_LIMBS];
    uint32_t size = curve->size;
    uint32_t limbs = size / 4;
    uint32_t used;
    if (point_length != 1 + 2 * size || point[0] != 4) return 0;
    if (der_next(signature, signature_length, &offset, &sequence) || sequence.tag != 0x30 || offset != signature_length) return 0;
    if (der_child(&sequence, &child, 0x02, &integer_r) || der_child(&sequence, &child, 0x02, &integer_s) || child != sequence.length) return 0;
    if (der_integer_to_fixed(integer_r.content, integer_r.length, r_bytes, size)) return 0;
    if (der_integer_to_fixed(integer_s.content, integer_s.length, s_bytes, size)) return 0;
    if (ec_init(&context, curve)) return 0;
    bn_load(p_limbs, limbs, curve->p, size);
    bn_load(n_limbs, limbs, curve->n, size);
    bn_load(r, limbs, r_bytes, size);
    bn_load(s, limbs, s_bytes, size);
    if (bn_is_zero(r, limbs) || bn_is_zero(s, limbs) || bn_compare(r, n_limbs, limbs) >= 0 || bn_compare(s, n_limbs, limbs) >= 0) return 0;
    bn_load(px, limbs, point + 1, size);
    bn_load(py, limbs, point + 1 + size, size);
    if (bn_compare(px, p_limbs, limbs) >= 0 || bn_compare(py, p_limbs, limbs) >= 0) return 0;
    mont_to(&context.fp, q.x, px);
    mont_to(&context.fp, q.y, py);
    bn_copy(q.z, context.fp.one, limbs);
    mont_mul(&context.fp, lhs, q.y, q.y);
    mont_mul(&context.fp, rhs, q.x, q.x);
    mont_mul(&context.fp, rhs, rhs, q.x);
    fe_add(&context.fp, t, q.x, q.x);
    fe_add(&context.fp, t, t, q.x);
    fe_sub(&context.fp, rhs, rhs, t);
    fe_add(&context.fp, rhs, rhs, context.b);
    if (bn_compare(lhs, rhs, limbs) != 0) return 0;
    used = digest_length < size ? digest_length : size;
    crypto_zero(e_bytes, size);
    crypto_copy(e_bytes + size - used, digest, used);
    bn_load(e, limbs, e_bytes, size);
    if (bn_compare(e, n_limbs, limbs) >= 0) bn_sub(e, e, n_limbs, limbs);
    mont_to(&context.fn, s_mont, s);
    crypto_copy(exponent, curve->n, size);
    bytes_subtract_small(exponent, size, 2);
    mont_pow(&context.fn, w, s_mont, exponent, size);
    mont_to(&context.fn, e_mont, e);
    mont_mul(&context.fn, u1, e_mont, w);
    mont_from(&context.fn, u1, u1);
    mont_to(&context.fn, e_mont, r);
    mont_mul(&context.fn, u2, e_mont, w);
    mont_from(&context.fn, u2, u2);
    bn_copy(g.x, context.gx, limbs);
    bn_copy(g.y, context.gy, limbs);
    bn_copy(g.z, context.fp.one, limbs);
    ec_multiply(&context, &a, u1, &g);
    ec_multiply(&context, &b, u2, &q);
    ec_add(&context, &a, &a, &b);
    if (ec_to_affine_x(&context, x, &a)) return 0;
    if (bn_compare(x, n_limbs, limbs) >= 0) bn_sub(x, x, n_limbs, limbs);
    return bn_compare(x, r, limbs) == 0;
}

static int ec_load_point(struct ec_context *context, const uint8_t *point, uint32_t point_length, struct ec_point *result)
{
    uint32_t size = context->curve->size;
    uint32_t limbs = context->limbs;
    uint32_t px[EC_MAX_LIMBS], py[EC_MAX_LIMBS], prime[EC_MAX_LIMBS], lhs[EC_MAX_LIMBS], rhs[EC_MAX_LIMBS], t[EC_MAX_LIMBS];
    if (point_length != 1 + 2 * size || point[0] != 4) return -1;
    bn_load(prime, limbs, context->curve->p, size);
    bn_load(px, limbs, point + 1, size);
    bn_load(py, limbs, point + 1 + size, size);
    if (bn_compare(px, prime, limbs) >= 0 || bn_compare(py, prime, limbs) >= 0) return -1;
    mont_to(&context->fp, result->x, px);
    mont_to(&context->fp, result->y, py);
    bn_copy(result->z, context->fp.one, limbs);
    mont_mul(&context->fp, lhs, result->y, result->y);
    mont_mul(&context->fp, rhs, result->x, result->x);
    mont_mul(&context->fp, rhs, rhs, result->x);
    fe_add(&context->fp, t, result->x, result->x);
    fe_add(&context->fp, t, t, result->x);
    fe_sub(&context->fp, rhs, rhs, t);
    fe_add(&context->fp, rhs, rhs, context->b);
    return bn_compare(lhs, rhs, limbs) == 0 ? 0 : -1;
}

static int ec_to_affine(struct ec_context *c, uint32_t *x_out, uint32_t *y_out, const struct ec_point *p)
{
    uint8_t exponent[48];
    uint32_t inverse[EC_MAX_LIMBS], square[EC_MAX_LIMBS], cube[EC_MAX_LIMBS], x[EC_MAX_LIMBS], y[EC_MAX_LIMBS];
    if (bn_is_zero(p->z, c->limbs)) return -1;
    crypto_copy(exponent, c->curve->p, c->curve->size);
    bytes_subtract_small(exponent, c->curve->size, 2);
    mont_pow(&c->fp, inverse, p->z, exponent, c->curve->size);
    mont_mul(&c->fp, square, inverse, inverse);
    mont_mul(&c->fp, cube, square, inverse);
    mont_mul(&c->fp, x, p->x, square);
    mont_mul(&c->fp, y, p->y, cube);
    mont_from(&c->fp, x_out, x);
    mont_from(&c->fp, y_out, y);
    return 0;
}

static int ecdh_generate(const struct ec_curve *curve, uint8_t *private_key, uint8_t *public_point)
{
    struct ec_context context;
    struct ec_point generator, result;
    uint32_t k[EC_MAX_LIMBS], n[EC_MAX_LIMBS], x[EC_MAX_LIMBS], y[EC_MAX_LIMBS];
    uint32_t size = curve->size;
    uint32_t limbs = size / 4;
    uint32_t ready = 0;
    if (ec_init(&context, curve)) return -1;
    bn_load(n, limbs, curve->n, size);
    for (uint32_t attempt = 0; attempt < 16 && !ready; attempt++) {
        if (crypto_random_bytes(private_key, size)) return -2;
        bn_load(k, limbs, private_key, size);
        if (!bn_is_zero(k, limbs) && bn_compare(k, n, limbs) < 0) ready = 1;
    }
    if (!ready) return -2;
    crypto_zero(&generator, sizeof(generator));
    bn_copy(generator.x, context.gx, limbs);
    bn_copy(generator.y, context.gy, limbs);
    bn_copy(generator.z, context.fp.one, limbs);
    ec_multiply(&context, &result, k, &generator);
    if (ec_to_affine(&context, x, y, &result)) return -1;
    public_point[0] = 4;
    bn_store(public_point + 1, size, x, limbs);
    bn_store(public_point + 1 + size, size, y, limbs);
    crypto_zero(k, sizeof(k));
    return 0;
}

static int ecdh_shared(const struct ec_curve *curve, const uint8_t *private_key, const uint8_t *peer,
                       uint32_t peer_length, uint8_t *shared)
{
    struct ec_context context;
    struct ec_point q, result;
    uint32_t k[EC_MAX_LIMBS], n[EC_MAX_LIMBS], x[EC_MAX_LIMBS], y[EC_MAX_LIMBS];
    uint32_t size = curve->size;
    uint32_t limbs = size / 4;
    if (ec_init(&context, curve) || ec_load_point(&context, peer, peer_length, &q)) return -1;
    bn_load(n, limbs, curve->n, size);
    bn_load(k, limbs, private_key, size);
    if (bn_is_zero(k, limbs) || bn_compare(k, n, limbs) >= 0) return -1;
    ec_multiply(&context, &result, k, &q);
    crypto_zero(k, sizeof(k));
    if (ec_to_affine(&context, x, y, &result)) return -1;
    bn_store(shared, size, x, limbs);
    return 0;
}

enum x509_key_type {
    X509_KEY_NONE = 0,
    X509_KEY_RSA = 1,
    X509_KEY_EC_P256 = 2,
    X509_KEY_EC_P384 = 3
};

enum x509_signature_algorithm {
    X509_SIGNATURE_NONE = 0,
    X509_SIGNATURE_RSA_SHA256,
    X509_SIGNATURE_RSA_SHA384,
    X509_SIGNATURE_RSA_SHA512,
    X509_SIGNATURE_ECDSA_SHA256,
    X509_SIGNATURE_ECDSA_SHA384,
    X509_SIGNATURE_ECDSA_SHA512
};

static int oid_equal(const struct der_item *item, const uint8_t *oid, uint32_t length)
{
    return item->tag == 0x06 && item->length == length && crypto_equal(item->content, oid, length);
}

static int x509_parse_public_key(const uint8_t *spki, uint32_t length, struct x509_key *key)
{
    static const uint8_t oid_rsa[9] = {0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01};
    static const uint8_t oid_ec[7] = {0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01};
    static const uint8_t oid_p256[8] = {0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07};
    static const uint8_t oid_p384[5] = {0x2B,0x81,0x04,0x00,0x22};
    struct der_item sequence, algorithm, oid, bits;
    uint32_t offset = 0, child = 0, inner = 0;
    crypto_zero(key, sizeof(*key));
    if (der_next(spki, length, &offset, &sequence) || sequence.tag != 0x30 || offset != length) return -1;
    if (der_child(&sequence, &child, 0x30, &algorithm) || der_child(&sequence, &child, 0x03, &bits) || child != sequence.length) return -1;
    if (bits.length < 2 || bits.content[0] != 0) return -1;
    if (der_child(&algorithm, &inner, 0x06, &oid)) return -1;
    if (oid_equal(&oid, oid_rsa, sizeof(oid_rsa))) {
        struct der_item key_sequence, modulus, exponent;
        uint32_t key_offset = 0, key_child = 0;
        if (der_next(bits.content + 1, bits.length - 1, &key_offset, &key_sequence) || key_sequence.tag != 0x30 ||
            key_offset != bits.length - 1) return -1;
        if (der_child(&key_sequence, &key_child, 0x02, &modulus) || der_child(&key_sequence, &key_child, 0x02, &exponent) ||
            key_child != key_sequence.length) return -1;
        key->type = X509_KEY_RSA;
        key->modulus = modulus.content;
        key->modulus_length = modulus.length;
        key->exponent = exponent.content;
        key->exponent_length = exponent.length;
        return 0;
    } else if (oid_equal(&oid, oid_ec, sizeof(oid_ec))) {
        struct der_item curve;
        if (der_child(&algorithm, &inner, 0x06, &curve) || inner != algorithm.length) return -1;
        if (oid_equal(&curve, oid_p256, sizeof(oid_p256)) && bits.length == 1 + 65) key->type = X509_KEY_EC_P256;
        else if (oid_equal(&curve, oid_p384, sizeof(oid_p384)) && bits.length == 1 + 97) key->type = X509_KEY_EC_P384;
        else return -1;
        key->point = bits.content + 1;
        key->point_length = bits.length - 1;
        return 0;
    }
    return -1;
}

static int parse_two_digits(const uint8_t *text)
{
    if (text[0] < '0' || text[0] > '9' || text[1] < '0' || text[1] > '9') return -1;
    return (text[0] - '0') * 10 + (text[1] - '0');
}

static int days_from_civil(int year, int month, int day)
{
    int era, year_of_era, day_of_year, day_of_era;
    year -= month <= 2;
    era = (year >= 0 ? year : year - 399) / 400;
    year_of_era = year - era * 400;
    day_of_year = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
    return era * 146097 + day_of_era - 719468;
}

static int parse_time(const struct der_item *item, uint64_t *seconds)
{
    const uint8_t *t = item->content;
    int year, month, day, hour, minute, second, century;
    uint32_t p;
    if (item->tag == 0x17 && item->length == 13) {
        year = parse_two_digits(t);
        if (year < 0) return -1;
        year += year >= 50 ? 1900 : 2000;
        p = 2;
    } else if (item->tag == 0x18 && item->length == 15) {
        century = parse_two_digits(t);
        year = parse_two_digits(t + 2);
        if (century < 0 || year < 0) return -1;
        year += century * 100;
        p = 4;
    } else return -1;
    month = parse_two_digits(t + p); day = parse_two_digits(t + p + 2);
    hour = parse_two_digits(t + p + 4); minute = parse_two_digits(t + p + 6); second = parse_two_digits(t + p + 8);
    if (month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
        second < 0 || second > 60 || t[p + 10] != 'Z') return -1;
    *seconds = (uint64_t)(long long)days_from_civil(year, month, day) * 86400u +
               (uint64_t)(hour * 3600 + minute * 60 + second);
    return 0;
}

static int x509_parse_extensions(const struct der_item *wrapper, struct x509_certificate *certificate)
{
    static const uint8_t oid_basic[3] = {0x55,0x1D,0x13};
    static const uint8_t oid_usage[3] = {0x55,0x1D,0x0F};
    static const uint8_t oid_alt[3] = {0x55,0x1D,0x11};
    static const uint8_t oid_ext_usage[3] = {0x55,0x1D,0x25};
    static const uint8_t oid_subject_id[3] = {0x55,0x1D,0x0E};
    static const uint8_t oid_authority_id[3] = {0x55,0x1D,0x23};
    static const uint8_t oid_policies[3] = {0x55,0x1D,0x20};
    static const uint8_t oid_crl[3] = {0x55,0x1D,0x1F};
    static const uint8_t oid_policy_mappings[3] = {0x55,0x1D,0x21};
    static const uint8_t oid_policy_constraints[3] = {0x55,0x1D,0x24};
    static const uint8_t oid_inhibit_any[3] = {0x55,0x1D,0x36};
    static const uint8_t oid_server_auth[8] = {0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x01};
    static const uint8_t oid_any_usage[4] = {0x55,0x1D,0x25,0x00};
    struct der_item list;
    uint32_t offset = 0, position = 0;
    if (der_child(wrapper, &offset, 0x30, &list) || offset != wrapper->length) return -1;
    while (position < list.length) {
        struct der_item extension, oid, value;
        uint32_t child = 0;
        uint32_t critical = 0;
        struct der_item flag;
        if (der_next(list.content, list.length, &position, &extension) || extension.tag != 0x30) return -1;
        if (der_child(&extension, &child, 0x06, &oid)) return -1;
        if (der_next(extension.content, extension.length, &child, &flag)) return -1;
        if (flag.tag == 0x01) {
            if (flag.length != 1) return -1;
            critical = flag.content[0] != 0;
            if (der_next(extension.content, extension.length, &child, &value)) return -1;
        } else value = flag;
        if (value.tag != 0x04 || child != extension.length) return -1;
        if (oid_equal(&oid, oid_basic, 3)) {
            struct der_item sequence, item;
            uint32_t inner = 0, at = 0;
            if (der_next(value.content, value.length, &at, &sequence) || sequence.tag != 0x30 || at != value.length) return -1;
            certificate->basic_constraints_present = 1;
            certificate->is_ca = 0;
            certificate->path_length = -1;
            if (inner < sequence.length) {
                if (der_next(sequence.content, sequence.length, &inner, &item)) return -1;
                if (item.tag == 0x01) {
                    if (item.length != 1) return -1;
                    certificate->is_ca = item.content[0] != 0;
                    if (inner < sequence.length && der_next(sequence.content, sequence.length, &inner, &item)) return -1;
                }
                if (inner <= sequence.length && item.tag == 0x02 && !(inner == 0)) {
                    if (item.length == 0 || item.length > 2 || (item.content[0] & 0x80)) return -1;
                    certificate->path_length = item.length == 1 ? item.content[0] : (int)(item.content[0] * 256 + item.content[1]);
                }
            }
        } else if (oid_equal(&oid, oid_usage, 3)) {
            struct der_item bits;
            uint32_t at = 0;
            if (der_next(value.content, value.length, &at, &bits) || bits.tag != 0x03 || bits.length < 2) return -1;
            certificate->key_usage_present = 1;
            certificate->key_usage = bits.content[1];
        } else if (oid_equal(&oid, oid_alt, 3)) {
            struct der_item names;
            uint32_t at = 0;
            if (der_next(value.content, value.length, &at, &names) || names.tag != 0x30 || at != value.length) return -1;
            certificate->alt_names = names.content;
            certificate->alt_names_length = names.length;
        } else if (oid_equal(&oid, oid_ext_usage, 3)) {
            struct der_item usages, usage;
            uint32_t at = 0, inner = 0;
            if (der_next(value.content, value.length, &at, &usages) || usages.tag != 0x30 || at != value.length) return -1;
            certificate->ext_usage_present = 1;
            while (inner < usages.length) {
                if (der_next(usages.content, usages.length, &inner, &usage)) return -1;
                if (oid_equal(&usage, oid_server_auth, 8) || oid_equal(&usage, oid_any_usage, 4)) certificate->server_auth = 1;
            }
        } else if (oid_equal(&oid, oid_subject_id, 3) || oid_equal(&oid, oid_authority_id, 3) ||
                   oid_equal(&oid, oid_policies, 3) || oid_equal(&oid, oid_crl, 3) ||
                   oid_equal(&oid, oid_policy_mappings, 3) || oid_equal(&oid, oid_policy_constraints, 3) ||
                   oid_equal(&oid, oid_inhibit_any, 3)) {
        } else if (critical) {
            return -1;
        }
    }
    return 0;
}

static int x509_parse_signature_algorithm(const struct der_item *algorithm, uint32_t *result)
{
    static const uint8_t rsa_prefix[8] = {0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01};
    static const uint8_t ecdsa_prefix[7] = {0x2A,0x86,0x48,0xCE,0x3D,0x04,0x03};
    struct der_item oid;
    uint32_t offset = 0;
    *result = X509_SIGNATURE_NONE;
    if (der_child(algorithm, &offset, 0x06, &oid)) return -1;
    if (oid.length == 9 && crypto_equal(oid.content, rsa_prefix, 8)) {
        if (oid.content[8] == 0x0B) *result = X509_SIGNATURE_RSA_SHA256;
        else if (oid.content[8] == 0x0C) *result = X509_SIGNATURE_RSA_SHA384;
        else if (oid.content[8] == 0x0D) *result = X509_SIGNATURE_RSA_SHA512;
    } else if (oid.length == 8 && crypto_equal(oid.content, ecdsa_prefix, 7)) {
        if (oid.content[7] == 0x02) *result = X509_SIGNATURE_ECDSA_SHA256;
        else if (oid.content[7] == 0x03) *result = X509_SIGNATURE_ECDSA_SHA384;
        else if (oid.content[7] == 0x04) *result = X509_SIGNATURE_ECDSA_SHA512;
    }
    return 0;
}

int x509_parse_certificate(const uint8_t *der, uint32_t length, struct x509_certificate *certificate)
{
    struct der_item outer, tbs, outer_algorithm, signature, item, inner_algorithm, validity, time_item;
    uint32_t offset = 0, child = 0, tbs_child = 0, validity_child = 0, algorithm_check;
    crypto_zero(certificate, sizeof(*certificate));
    certificate->path_length = -1;
    if (der_next(der, length, &offset, &outer) || outer.tag != 0x30 || offset != length) return -1;
    certificate->der = der;
    certificate->der_length = length;
    if (der_child(&outer, &child, 0x30, &tbs) || der_child(&outer, &child, 0x30, &outer_algorithm) ||
        der_child(&outer, &child, 0x03, &signature) || child != outer.length) return -1;
    if (signature.length < 2 || signature.content[0] != 0) return -1;
    certificate->tbs = tbs.raw;
    certificate->tbs_length = tbs.raw_length;
    certificate->signature = signature.content + 1;
    certificate->signature_length = signature.length - 1;
    if (x509_parse_signature_algorithm(&outer_algorithm, &algorithm_check)) return -1;
    certificate->signature_algorithm = algorithm_check;
    if (der_next(tbs.content, tbs.length, &tbs_child, &item)) return -1;
    if (item.tag == 0xA0) {
        if (der_next(tbs.content, tbs.length, &tbs_child, &item)) return -1;
    }
    if (item.tag != 0x02) return -1;
    if (der_child(&tbs, &tbs_child, 0x30, &inner_algorithm)) return -1;
    if (inner_algorithm.raw_length != outer_algorithm.raw_length ||
        !crypto_equal(inner_algorithm.raw, outer_algorithm.raw, outer_algorithm.raw_length)) return -1;
    if (der_child(&tbs, &tbs_child, 0x30, &item)) return -1;
    certificate->issuer = item.raw;
    certificate->issuer_length = item.raw_length;
    if (der_child(&tbs, &tbs_child, 0x30, &validity)) return -1;
    if (der_next(validity.content, validity.length, &validity_child, &time_item) || parse_time(&time_item, &certificate->not_before)) return -1;
    if (der_next(validity.content, validity.length, &validity_child, &time_item) || parse_time(&time_item, &certificate->not_after)) return -1;
    if (validity_child != validity.length) return -1;
    if (der_child(&tbs, &tbs_child, 0x30, &item)) return -1;
    certificate->subject = item.raw;
    certificate->subject_length = item.raw_length;
    if (der_child(&tbs, &tbs_child, 0x30, &item)) return -1;
    certificate->spki = item.raw;
    certificate->spki_length = item.raw_length;
    if (x509_parse_public_key(item.raw, item.raw_length, &certificate->key)) {
        crypto_zero(&certificate->key, sizeof(certificate->key));
    }
    while (tbs_child < tbs.length) {
        if (der_next(tbs.content, tbs.length, &tbs_child, &item)) return -1;
        if (item.tag == 0xA3) {
            if (x509_parse_extensions(&item, certificate)) return -1;
        } else if (item.tag != 0xA1 && item.tag != 0xA2) return -1;
    }
    return 0;
}

int x509_verify_signature(const struct x509_certificate *certificate, const struct x509_key *issuer_key)
{
    uint8_t digest[HASH_MAX_DIGEST_SIZE];
    enum hash_algorithm algorithm = HASH_NONE;
    uint32_t digest_size;
    int rsa = 0;
    switch (certificate->signature_algorithm) {
    case X509_SIGNATURE_RSA_SHA256: algorithm = HASH_SHA256; rsa = 1; break;
    case X509_SIGNATURE_RSA_SHA384: algorithm = HASH_SHA384; rsa = 1; break;
    case X509_SIGNATURE_RSA_SHA512: algorithm = HASH_SHA512; rsa = 1; break;
    case X509_SIGNATURE_ECDSA_SHA256: algorithm = HASH_SHA256; break;
    case X509_SIGNATURE_ECDSA_SHA384: algorithm = HASH_SHA384; break;
    case X509_SIGNATURE_ECDSA_SHA512: algorithm = HASH_SHA512; break;
    default: return 0;
    }
    digest_size = hash_digest_size(algorithm);
    hash_digest(algorithm, certificate->tbs, certificate->tbs_length, digest);
    if (rsa) {
        if (issuer_key->type != X509_KEY_RSA) return 0;
        return rsa_verify_pkcs1(issuer_key->modulus, issuer_key->modulus_length, issuer_key->exponent,
                                issuer_key->exponent_length, certificate->signature, certificate->signature_length,
                                algorithm, digest);
    }
    if (issuer_key->type == X509_KEY_EC_P256)
        return ecdsa_verify(issuer_key->point, issuer_key->point_length, &ec_p256, digest, digest_size,
                            certificate->signature, certificate->signature_length);
    if (issuer_key->type == X509_KEY_EC_P384)
        return ecdsa_verify(issuer_key->point, issuer_key->point_length, &ec_p384, digest, digest_size,
                            certificate->signature, certificate->signature_length);
    return 0;
}

int x509_verify_handshake_signature(const struct x509_key *key, uint16_t scheme, const uint8_t *message,
                                    uint32_t message_length, const uint8_t *signature, uint32_t signature_length)
{
    uint8_t digest[HASH_MAX_DIGEST_SIZE];
    enum hash_algorithm algorithm;
    uint32_t digest_size;
    switch (scheme) {
    case 0x0804: case 0x0805: case 0x0806:
        algorithm = scheme == 0x0804 ? HASH_SHA256 : scheme == 0x0805 ? HASH_SHA384 : HASH_SHA512;
        if (key->type != X509_KEY_RSA) return 0;
        hash_digest(algorithm, message, message_length, digest);
        return rsa_verify_pss(key->modulus, key->modulus_length, key->exponent, key->exponent_length,
                              signature, signature_length, algorithm, digest);
    case 0x0401: case 0x0501: case 0x0601:
        algorithm = scheme == 0x0401 ? HASH_SHA256 : scheme == 0x0501 ? HASH_SHA384 : HASH_SHA512;
        if (key->type != X509_KEY_RSA) return 0;
        hash_digest(algorithm, message, message_length, digest);
        return rsa_verify_pkcs1(key->modulus, key->modulus_length, key->exponent, key->exponent_length,
                                signature, signature_length, algorithm, digest);
    case 0x0403:
        if (key->type != X509_KEY_EC_P256) return 0;
        algorithm = HASH_SHA256;
        digest_size = hash_digest_size(algorithm);
        hash_digest(algorithm, message, message_length, digest);
        return ecdsa_verify(key->point, key->point_length, &ec_p256, digest, digest_size, signature, signature_length);
    case 0x0503:
        if (key->type != X509_KEY_EC_P384) return 0;
        algorithm = HASH_SHA384;
        digest_size = hash_digest_size(algorithm);
        hash_digest(algorithm, message, message_length, digest);
        return ecdsa_verify(key->point, key->point_length, &ec_p384, digest, digest_size, signature, signature_length);
    default:
        return 0;
    }
}

static char fold_case(char value)
{
    return (value >= 'A' && value <= 'Z') ? (char)(value + 32) : value;
}

static int names_equal_ignore_case(const char *left, const char *right, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) if (fold_case(left[i]) != fold_case(right[i])) return 0;
    return 1;
}

static int dns_name_matches(const char *pattern, uint32_t pattern_length, const char *host, uint32_t host_length)
{
    if (host_length && host[host_length - 1] == '.') host_length--;
    if (pattern_length >= 3 && pattern[0] == '*' && pattern[1] == '.') {
        uint32_t dot = 0;
        uint32_t rest_dots = 0;
        while (dot < host_length && host[dot] != '.') dot++;
        if (dot == 0 || dot >= host_length) return 0;
        for (uint32_t i = 2; i < pattern_length; i++) if (pattern[i] == '.') rest_dots++;
        if (rest_dots == 0) return 0;
        for (uint32_t i = 0; i < pattern_length; i++) if (pattern[i] == '*' && i != 0) return 0;
        return host_length - dot == pattern_length - 1 && names_equal_ignore_case(host + dot, pattern + 1, pattern_length - 1);
    }
    for (uint32_t i = 0; i < pattern_length; i++) if (pattern[i] == '*') return 0;
    return pattern_length == host_length && names_equal_ignore_case(pattern, host, host_length);
}

static int parse_ipv4(const char *text, uint8_t address[4])
{
    uint32_t part = 0, value = 0, digits = 0;
    for (uint32_t i = 0;; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            value = value * 10 + (uint32_t)(c - '0');
            digits++;
            if (value > 255 || digits > 3) return 0;
        } else if (c == '.' || c == 0) {
            if (digits == 0 || part > 3) return 0;
            address[part++] = (uint8_t)value;
            value = 0;
            digits = 0;
            if (c == 0) break;
        } else return 0;
    }
    return part == 4;
}

int x509_matches_host(const struct x509_certificate *certificate, const char *host)
{
    uint8_t address[4];
    uint32_t host_length = crypto_string_length(host);
    uint32_t offset = 0;
    int is_address = parse_ipv4(host, address);
    if (!host_length) return 0;
    while (offset < certificate->alt_names_length) {
        struct der_item name;
        if (der_next(certificate->alt_names, certificate->alt_names_length, &offset, &name)) return 0;
        if (is_address) {
            if (name.tag == 0x87 && name.length == 4 && crypto_equal(name.content, address, 4)) return 1;
        } else if (name.tag == 0x82) {
            if (dns_name_matches((const char *)name.content, name.length, host, host_length)) return 1;
        }
    }
    return 0;
}

static int bytes_equal_lengths(const uint8_t *left, uint32_t left_length, const uint8_t *right, uint32_t right_length)
{
    return left_length == right_length && crypto_equal(left, right, left_length);
}

void tls_trust_store_init(struct tls_trust_store *store)
{
    crypto_zero(store, sizeof(*store));
}

int tls_trust_store_add_anchor(struct tls_trust_store *store, const uint8_t *subject, uint32_t subject_length,
                               const uint8_t *spki, uint32_t spki_length)
{
    struct x509_key key;
    if (store->count >= TLS_TRUST_STORE_MAX) return -1;
    if (x509_parse_public_key(spki, spki_length, &key)) return -1;
    store->anchors[store->count].subject = subject;
    store->anchors[store->count].subject_length = subject_length;
    store->anchors[store->count].spki = spki;
    store->anchors[store->count].spki_length = spki_length;
    store->count++;
    return 0;
}

int tls_trust_store_add_certificate(struct tls_trust_store *store, const uint8_t *der, uint32_t length)
{
    struct x509_certificate certificate;
    if (x509_parse_certificate(der, length, &certificate)) return -1;
    return tls_trust_store_add_anchor(store, certificate.subject, certificate.subject_length,
                                      certificate.spki, certificate.spki_length);
}

#define TLS_BUILTIN_ANCHOR_COUNT 118
#define TLS_BUILTIN_POOL_SIZE 54355
struct tls_builtin_anchor { const char *data; uint16_t subject_length; uint16_t spki_length; };
static const struct tls_builtin_anchor tls_builtin_anchors[TLS_BUILTIN_ANCHOR_COUNT] = {
    {"MEIxEjAQBgNVBAMMCUFDQ1ZSQUlaMTEQMA4GA1UECwwHUEtJQUNDVjENMAsGA1UECgwEQUNDVjELMAkGA1UEBhMCRVMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCbqau/YUqXry+XZpp0X9DZlv3P4uRm7x8fRzPCRKPfmt4ftVTdFXxpNRFvu8gMjmoYHtiP2Ra8EEg2XPBjs5BaXCQ316PWywlxufEBcoSwfdtNgM3802/J+Nq2DoLSRYWoG2ioPej0RGy9ocLLA76MPhMAhN9KSMDjIgro6TenGEyxCQ0jVn8ETdkXhBilyNpAlHPrzg5XPAOBOp0KoVdDaaxXbXmQeOW1tDvYvEyNKKGno6e6Ak4l0Squ7a4DIrhrIA8wKFSVf+DuzgpmndFALW4ir50awQUZ0m/A8p/4e7MCQvtQqR0tkw8jq8bBD5L/0KIV9VMJcRz/RROE5iZe+OCIHAr8Fraocwa48GOEAqDGWuzndN9wrqODJerWx5eHk6fGioozl2A3ED6XPm4pFdahD9GILBKfb6qkxkLrQaLjlUPTAYVtjrs78yM2x/474KElB0iryYl0/wiPgL/AlmXz7uxLaL2diMMxs0Dx6M/2OLuc5NF/1OVYm3z61PMOm3WR5LpSLhl+0fXNWhn8ugb2+1KoS5kE3fj5tItQo05iifCHJPqDQsGH+tUtKSpacXpkatcnYGMN285J9Y0fkIkyF/hzQ7jSWpOGYdbhdQrqeWZ2iE9x6wQl1gpaepPluUsXQA+xtrn13k/c4LOsOxFwYIRKQ26ZIMApcQrAZQIDAQAB", 68, 550},
    {"MDsxCzAJBgNVBAYTAkVTMREwDwYDVQQKDAhGTk1ULVJDTTEZMBcGA1UECwwQQUMgUkFJWiBGTk1ULVJDTTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALpxgHpMhm5/yBNtwMZ9HACXjywMI7sQmkCpGreHiPibVmr75nuOi5KOpyVdWRHbNi63URcfqQgfBBckWKo3Shjf5TnUV/3XwSyRAZHiItQDwFj8d0fsjz50Q7qsNI1NOHZnjrDIbzAzWHFctPVrbtQBULgTfmxKo0nRIBnuvMApGGWn3v7v3QqQIecaZ5JCEJhfTzC8PhxFtBDXaEAUwED653cXeuYLj2VbPNmaUtu1vZ5Gzz3rkQUCwJaydkxNEJY7kvqcfw+Z374jNUUeAlz+taibmSXaXvMiwzn15Cou08YfxGyqxRxqAQVKL9LFwag0Jl1mpdICIfkYtwb1TplvqKtMUejPUBjFd8g5CSxJkjKZqLsXF3mwWsXmo8RZZUc1g16p6DULmbvkzSDGm0oGObVo/CK67lWMK07q87Hj/LaZmtVC+nFNCM+HHmpxffnTtOmlcYF7wk5HlqX2doWjKI/pgG6BU6VtX7hI+cL5NqYuSf+4lsKMB7ObiFj86xsc3i1w4peSMKGJ47xVqCfWS+2QrYv6YyVZLag13cqXM7zlzced0ezvXg5KkAYmY6252TUtB7p2ZSysV4999AeU14ECll2jB0nVetBX+RvnU0Z1qrB5QstocQjpYL05ac70r8NWQMetUqIJ5G+GR4of6ygnXYMgrwTJbFaai0b1AgMBAAE=", 61, 550},
    {"MHgxCzAJBgNVBAYTAkVTMREwDwYDVQQKDAhGTk1ULVJDTTEOMAwGA1UECwwFQ2VyZXMxGDAWBgNVBGEMD1ZBVEVTLVEyODI2MDA0SjEsMCoGA1UEAwwjQUMgUkFJWiBGTk1ULVJDTSBTRVJWSURPUkVTIFNFR1VST1MwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAT2uldTyMqr3zZKUiHkl9KDZ57wZVHQXofHR7FZ8ldHmwACk0QXadtCx7GyOhgOtF2Ms2ZdoTT5NixJ2/NG/LNEaUQTZv3Xxf2vNk3OA00Hcc+vagXSokNaClJvAQNOjos=", 122, 120},
    {"MIGEMRIwEAYDVQQFEwlHNjMyODc1MTAxCzAJBgNVBAYTAkVTMScwJQYDVQQKEx5BTkYgQXV0b3JpZGFkIGRlIENlcnRpZmljYWNpb24xFDASBgNVBAsTC0FORiBDQSBSYWl6MSIwIAYDVQQDExlBTkYgU2VjdXJlIFNlcnZlciBSb290IENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2+trK+ZkVJWCkKNypBkBnZwLgV9zSbqnrPMETnuWC+wR4FumHM4b0g2DHCu4nh1+RTJgDwfpd1h+n2rIYU62JsFMjf9M7zSyH2XYuXj1ralxue9PWB2l3nQgl6HtaEzekhdLvKv/ZZqe+0fZV3LzCaGudkQTbpwtRDm8+cc7pFg9Qb20wkmjyA3Sly8HZVIAp27Ir2js9BSWtlcfVsM5nytt5PM+9jVk2gwcoYRLL0tL4iwknW2TQOu1I44yym9F06iJex7PHvpbQ4vNzagPasoMXrmeR4/w2bYKC1hlFzO5I+R3GX3LSi6Se08vEHexjS9onGLM4FD47JGnVExXCdV2Y8XoZR7ubWrPCZ36fE+tYAj9VpkPFSx7qYCrjGGPSgd2Qt499N2yJDNbuLWjRMmsf3c8HSPsgqmm4sgGTAL+rFyZmQsvEIqm9H/Vh3QNWUlF9vBxXDkp1r9KI4v1XwFj0odzKLVLCvX4q4IsfnMlMh0LYwoXgQD/tnZe57SxQMohu9WAUeVIUmcs0mGJBw0PzkJ3wERznERQoNsQCi2VHIGv5BzlFB7xNkEBAi99c6feQsxM6YkNVvefkdQDxmzJj9vYHOBAmF1mmZiAbi3/AcXOy0YfrALGQ+auooQ8xU4ePW3JFEzjLkG7yjm/NjwqGapBh06lzksyed2QSX8CAwEAAQ==", 135, 550},
    {"MGsxCzAJBgNVBAYTAklUMQ4wDAYDVQQHDAVNaWxhbjEjMCEGA1UECgwaQWN0YWxpcyBTLnAuQS4vMDMzNTg1MjA5NjcxJzAlBgNVBAMMHkFjdGFsaXMgQXV0aGVudGljYXRpb24gUm9vdCBDQTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAKfGxKUppCzv5RjFsFCjb1E7nwpaycJIOArCHKAYf5G1h7lAP90daB8Ig9UtHoig+I9Wj22ZApKQFtVfCGyJ1+GsvCDCseCDUYppTQCWWm8vwER+ow7kkc1Y7tz7xx5FR90nuQgBn6YhHfVBLS9M/Sit4IqtIrRWZY6GVI+TQyneOUZ4ozAjus3wfRNXwF3Sg2tITMSrn4BaWzq9yaciP4AnM1sOt4oMXQc3CMts0npHIkQ1xczMLo7dKu23fWYNX2FRIlUb40bj4z3QNWKa268UyFuhzIkb4TAm/KCbH4GnRx8E66M5kgafmdO/0+pPUJwZ/paHHjxl9qMYJIOGEOdUPqg6diRPgSHF4w8C+JOURyC7/tQO02i53cR6hILjU1R53duc0vIHmy62vD7thW3vJRHylxpCYfdKl+iLsRAH+mWBsqI5z/c8/xj7xvFai1niAqx7ktBOFE9ZRfYMXihfsOg/Rc/Pr5tv+4TTd1qVb6yUhJ7uvMBKj0qT+EQh4jFFYVBOENjjNXxMGbTeBb+jBp/Itc3kH9cXBg16lXRVDWga/BAbYmSdbeCVoMOUB1cNFOa9Bfu4n+bfi+LG536W9lPFgDRQKFjwElBxFzC65nhjvPSyrZsrsv7hOYxeugsglN57g7j/41aNtxHpO4zyscFdnaQLTCvZshj1tZ9LAgMBAAE=", 109, 550},
    {"MDkxCzAJBgNVBAYTAlVTMQ8wDQYDVQQKEwZBbWF6b24xGTAXBgNVBAMTEEFtYXpvbiBSb290IENBIDEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCyeIBxynjV43GvR4BQdH1u2NeIdvSZaPdYIWD5dIQBL6wCLYbToEN6TrKk0Da6Ab6N20jIBxc2TPTuiCPHPus39bUZ+ElosN7XuXY4HWGepP6CNqXlSlbkReH5/bQW+nTanJs1OS/6sCBQBmx60ICypvmv7EcZj1A4B9yihzlY+LrVqflIZzCW7pR4Xm+Jo1HAMIZmoUVmulTro8OR+Ujc/9HoMC19LXRwNdeIJPeexFluu3OHF/IyRii4Q/q3HarKtPKfJA4tS/dxXF5p/+qVAss4iq5QOG/b+y1iG8XHHlThd+BnyA+chyPWP0AgfyCAxIBMPjskJo4ErmyayKoNAgMBAAE=", 59, 294},
    {"MDkxCzAJBgNVBAYTAlVTMQ8wDQYDVQQKEwZBbWF6b24xGTAXBgNVBAMTEEFtYXpvbiBSb290IENBIDIwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCtlp8tnEpMSoF5UZnsistrYFETvE1tBvywCI3dGRBqxyYMNdjAbyCE6ZSxm4UDw1vbSujI+JB22VtP40zoBjZNzJqsPQyQK5LUBhlgrDdEeYWBgq1aN+ANzJ2mTFJ26kOdtwTRUPZV4NXSpkmF6Tfpyn6uXJVNSJo/riBabYiV2TS4UhpDkLC/bAW5tni36tDkOjwSU2L/SvJ7vjUFqRI04/NkdGIsPQBJWij+MkS7h91lJwJxO9pK9x/azfchVZBPD+yuguGfa9lF07vwX4ftPCw5hto/3uxyVet5o63b3XywuhzO/N5PNXbPD/h4H2o2UUYnYVvpns/wolV9fCWKby+0xc+ELiv9DVEQbPtfG7wbfsWuO5gBMZL/C1f0mrK5V+mr7w120fDu9M6Gp+Bu6bRpod9p9jPGaS6XE56lh7BXEIE3yVOzu3/2ktGc0Bj0km7ag0+mY5lMpfte7yFkeiBfbGSFFcs36WIMCyoW3AEuMto+S/WeOvYXQJTvnpEIhvq+Y6haM+zLdEOV+WxpUjbHKW/8VQNcH/ufvUfr50lHlQtOiSIJSeD1YR7xvy6Kcm6AWf9XOvl1MqNOX+ztKGLZTXPyzIEXYO3N69zbp8rFfgK98lQIVP20LQksF1RKmNFU4VFnCNLtbn5vP9ItgVkpZsuQOZURHnQn/t3rrwIDAQAB", 59, 550},
    {"MDkxCzAJBgNVBAYTAlVTMQ8wDQYDVQQKEwZBbWF6b24xGTAXBgNVBAMTEEFtYXpvbiBSb290IENBIDMwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQpl6fGQX/ADZvoARtWxvJSpbotshLo0i7X+snF2KptH3OBOzuYazl8M6XFToaOgBdoYkVXfURYHbM35WcI62be", 59, 91},
    {"MDkxCzAJBgNVBAYTAlVTMQ8wDQYDVQQKEwZBbWF6b24xGTAXBgNVBAMTEEFtYXpvbiBSb290IENBIDQwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATSq4o3T6NTDf7BintLqHtGS2OwYvYtG9sIcSHSAOhjvZon+/A5bl3qPaXJgaqjWyCYRV0W2/3oEG3jnODjvV+EYvNwZDOgyyQvcLqIoSqgdfiBrmIGxIHbOW4psB76Llw=", 59, 120},
    {"MEwxLjAsBgNVBAMMJUF0b3MgVHJ1c3RlZFJvb3QgUm9vdCBDQSBFQ0MgVExTIDIwMjExDTALBgNVBAoMBEF0b3MxCzAJBgNVBAYTAkRFMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEloZYKDcKZ9Cg3iQZGeHkBQcfl+3oZIK59sRxUM6KDP/XtXa7oWyTbIOiaG6l2b4siJVBzV3dscqDY4PMwL502eCdpO5KTlbgmClBk1IQ1SQ4AjJn8ZQSb+/Xxd4u/RmA", 78, 120},
    {"MEwxLjAsBgNVBAMMJUF0b3MgVHJ1c3RlZFJvb3QgUm9vdCBDQSBSU0EgVExTIDIwMjExDTALBgNVBAoMBEF0b3MxCzAJBgNVBAYTAkRFMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAtoAOxHm9BYx9sKOdTSJNy/BBl01Z4NH+VoyX8te9j2y3I49f1cTYQcvyAh5x5en2XssIKl4w8i1mx4QbZFc4nXUtVsYvYe+W/CBGvevUez8/fEc4BKkbqlLfEzfTFRVOvV98r61jx3ncCHvVoOX3W3WsgFWZkmGbzSoXfduP9LVq6hdKZChmFSlsAvFr1bqjM9xaZ6cF4r9lthawEO3NUDPJcFDsGY6wx/J0W2tExn2WuZgIWWbeKQGb9Cpt0xU6kGpn8bRrZtkh68rZYnxGEFzedUlnnkL5/nWpo63/dgpnQOPF943HhZpZnmKaau1Fh5hnstVKPNe0OwANwI8f4UDErmwh3El+fsqyjW22v5MvoVw+j8rtgI5Y4dtXz4U2OLJxpAmMkokIiEjxQGMYsluMWuPD0xeqqxmjLBvk1cbiZnrXghmmOxYsL3GHX0WelXOTwkKBIROW1527k2gV+p2kHYzygeBYBr3JtuP2iV2J+axEoctr+hbxx1A9JNr3w+SH1VbxT5Aw+kUJWdo0zuATHAR8ANSbhqRAvNncTFd+rrcztl524WWLZt+NyteYr842mIycg5kDcPOvdO3GDjbnvezBc6eUWsuSZIKmAMFwoW4sKeFYV+xafJlrJaSQOoD0IJ2azsct+bJLKZWD6TWNp0lIpw9MGZHQ9b8Q4HECAwEAAQ==", 78, 550},
    {"MFExCzAJBgNVBAYTAkVTMUIwQAYDVQQDDDlBdXRvcmlkYWQgZGUgQ2VydGlmaWNhY2lvbiBGaXJtYXByb2Zlc2lvbmFsIENJRiBBNjI2MzQwNjgwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDKlmuO6vj78aI14H9M2uDDUtd9thDIAl6zQyrET2qyyhxdKJp4ERppWVevtSBC5IsP5t9bpgOSL/UR5GLXMnE42QQMcas9UX4PB99jBVzpv5RvwSmCwLTaUbDBPLutN0pcyvFLNg4kq7/DhHf9qFD0sefGL9ItWY16Ck6WaVICqjaY7Pz6FIMMNx/Jkjd/14Et5cS54D40/mf0PmbR0/RAz15iNA9wBj4gGFrO93IbJWyTdBSTo3OxDqqHECNZXyAFGUftaI6SEspd/NYrspI8IM/hX68gvqB2f3bl7BqGYTM+53u0P6APjqK5am+5hyZvQWyIplD9amML9ZMWGxmPsu2bm8mQ9QEM3xk9Dz44I8kvjwzRAv4bVdZO0I08r0+k8/6vKtMFnXkIoctXMbScyJCyZ/QYFpM6/EfY0XiWMR+6KwxfXZmtY4laJCB22N/9q06mIqqdXuYnin1oKaPnirjaEbsXLZmdEyRG98Xi2J+Of8ePdG1asuhy9azuJBCtLxTa/y2aRnFHvkLfuwHb9H/TKI8xWVvTyQKmtFLKbpf7Q8UIJm+K9Lv9nyiqDdVF8xM6HdjAeI9BZzwelGSuewvF6NkBiDkal4ZkQdU7hwxu+g/GvUgUvzlN1J5Bto+WHWOWk9mVBngxaJ43BjuAiUVhOSPHG0SjFeUc+JIwuwIDAQAB", 83, 550},
    {"MFQxCzAJBgNVBAYTAkNOMSYwJAYDVQQKDB1CRUlKSU5HIENFUlRJRklDQVRFIEFVVEhPUklUWTEdMBsGA1UEAwwUQkpDQSBHbG9iYWwgUm9vdCBDQTEwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDxZgi92cUVYcuEBEGlaTd3HcGwe/rDd0iQE3Jk0bh8kDWdGHmI45cBPEeB8g6imA2ePzfgGbKQ8kYckrE6Yc76t0aeA4bXM27t90WMdjfebpaR99d+K4cX1Ys17oSRclfcYMPDuefHZyQjT2MKY/ZmfUtVpz94ZElpEpfgTA3TCaAyMDr6n8DynMUSKi4ctQQz2qQ4EWrexhj2RzoiQYci/MSJKFTYjKUwCvgXFsqsN/15p5EXeDiZrVjtst7MiX0DnLOJZefjO7Eiho8GbXgH/ZESf7BrHIkN+bjLdFsHwsj0NdFkY3rpbpoo1jC95hvdFa+E6pzHyvUO6vJdKYePaXM5vi4kb0UhrMXUaSUGg616SIUTLA0GuGx5VvyjZzKB9VelyldCaelcJGHv4jAYTkSYVW96wpPYGbbefEeKEU5JR9solAILlEos+RLQT+gxfmx6v6Y/mzk9AhajGLNnrFs/LIMrZzmBXLl+lNVk3Z6Pbq7ofFu012pHSNd+s9QtjlZ2Ts9p8W5EbNQk6o0koRi/vVf+qZk1tdsQd7g9SLrWwefxIz7X34WdJzzUQL0KDL31540l1oF0h0bUKXWiQmz3c4nnfb96Sh/TIskVVc/fb3xV0KSLBxE3X4OmJlemAVt+/lhoB6npetm56P9QH6vCtMDO6Or9D72NTbi8cQIDAQAB", 86, 550},
    {"MFQxCzAJBgNVBAYTAkNOMSYwJAYDVQQKDB1CRUlKSU5HIENFUlRJRklDQVRFIEFVVEhPUklUWTEdMBsGA1UEAwwUQkpDQSBHbG9iYWwgUm9vdCBDQTIwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAASdy4CRjVNntblQsQP45UkfQSIJsFFSWNYrNI/FEkYUxYsvLIT/LG6o1fEJ4wMhFMRDPXzBLMRLakrN6YfgffYivvpKUbgwiv3h3hgSCvZHt+cXvyeK1EFMljxglsH9FRw=", 86, 120},
    {"ME4xCzAJBgNVBAYTAk5PMR0wGwYDVQQKDBRCdXlwYXNzIEFTLTk4MzE2MzMyNzEgMB4GA1UEAwwXQnV5cGFzcyBDbGFzcyAyIFJvb3QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDXx173wQfUd/tDIfT09Wnk7jIB26OGH+RZDbrndYNS6+ocYRVIux0HyoyusNyWnerDYJKGgihznFYG/0tk8AwqN0m15c8MfO7xSrtzMGXz1S+Dtn7j5/Weq2D50/GdknSK5ByWrFuA6bX0MYejUfzHfqFvjlN31JfBVTOSPhgvddSthknLla9UBmzYBhONW//hJhlZwCS6gXF5kERQaCSUX7izEfEpQWGjQcsjNtXB8TJQEE5/9IaT7ITTjrxLv1wBTgc93BSKlAqk6nP7C1HoEwcY+g7xK9FUFX084fe0GUJnYl534KJV7LbZaRfVOq9E7UrFnuR6J3zlddeqyyXn32sK2w9Nk06ooM17LvJZAWq3DbgHgX6LOBs45gpXmT3uIeij9QwW3YvsNI6cKhwAFReNaIPScJ8YCM0RaNXJa1LNxEaP3LXz2FdzHumUOQS/09443rRT7Gkcon7Ej+QbcK3yovn79xZkZmmfSVGi4hUYZwZKf9VstU2zM+Bh612+6ZgPMtcdSzwuWgFSkQny3+qN2AZAY6oR5P7DN54UUj/04szyYZPR/Wdr11Kuv2irQEOgVzVTePBT+GFCB2TG12+bTDgNY6xirzaLonMKDfUhvXSqTepyA0nbx18dYmPH/d2R7DPu9W20bjBo3sjWJrB1Xnu0ByCYoXYyuE1sTwIDAQAB", 80, 550},
    {"ME4xCzAJBgNVBAYTAk5PMR0wGwYDVQQKDBRCdXlwYXNzIEFTLTk4MzE2MzMyNzEgMB4GA1UEAwwXQnV5cGFzcyBDbGFzcyAzIFJvb3QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCl2gqVFlDjlfJenXYxBjJ6m/EQdrgAmrVSNs0kR7CfGGS8mvb61XnYkGJMIi/eOD3W4KjpHCzbeBHpjmhRFXLH8zOH5KBdC1zgVwcqMPXNxDd3KE0Ykea/1VL9cS1wPufGxIrj8CgL9HaYoYuHVbI6E/y3Pic3jiLjqE8q72C7Pbc5ww4BR5ldEk/bQ/pXoe35nb4RRyZbE5irXRaKsDccV51F/4iWNr+7ygd7b4dj19AyatZdbAzxs2454msxLjkAJxTeOMDsGWaGEuidchYTZFLHqTcc/YIw7YQYHfSuXP9wEwDrsfUzekvWVfgFjUtpsPWzKDZcFMRRc01rC/E0B9sXOdfcKHtr9Z/zLsFPFyoQ88zK6Ov9a6sump8tgm4E1FIBky09hvx+/N/vQh2ma++5IMb3vaCnlf2n5okk2MyMNGziIy/ZEhohuVWRbwuReRkMrUCIC3DietIO2GhIu4ITORBY6dgqB8YS21jb0jtVEEcFFWdifhhjpkY/CQ5UMl6/DWJ6J++A6NvZSwZaN1ol0AgSd9RvCVCXPcgdw9+MRTBWxtNkq2bzwF6WnMPE78N8a4s6eX+zSc894omfoDBLhbmclCR5j31rqUVoDyvQ8docy2m4yklibcjQY2LdYA9Yqo+hvAWlZqLPG3ayhGSxTDlSwDC68IxLArC2twIDAQAB", 80, 550},
    {"MFIxCzAJBgNVBAYTAlNLMRMwEQYDVQQHEwpCcmF0aXNsYXZhMRMwEQYDVQQKEwpEaXNpZyBhLnMuMRkwFwYDVQQDExBDQSBEaXNpZyBSb290IFIyMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAoqPEAAnWhV0tbRT2wsNznjXCcVV+gfurRlDgwXxJeOareVg82v98HJ/YlwJ4PmtBBOlBvb4DLEX2L2TUq12jRz1km+lomsbMGz+6vrKLNAIumFUZ/Ixvql/aTM5NAyGj2NI0k1aWy0wMABY8XxrNyMdspq3TMae86OXhZtbS+wO0QWXJEK4OBWPGgGppMP3S7pDvDSffn5Vz9OEl2mwW3kE4NOqL/NHoBBRhLUF+rMd3TstRVPtekhgbBFpoxsnE+rcToJi3ESu31lfMfJ4X0csl/oZOJC5WDHhNngESpiunAWVufGIdhITf6sBrtaUqlYPDUxEMcx0LskaQ0UI6zkBula3/xpStbpeEjn1vnoqADUltc+J7kh7D88Hz6y4Fb9kbzzd2BMi0WuQXp8vddh/QGXboLAWz1pw02JbcYYeRBeRECDPB2rkIZdSusjYN67o4ugzlm57rjWbdmc/WiUH2BJKKKSltazoc53V9AnEO88DnvcsZ3Z1gssJmYLaxBO7J5oa5mmZAqOcR7YFFA4v2Z1nowQYRvd3PgAJPZUB4XEdQyJvmH4F75ESoW4Wa4t5a1cf5OkRmS+QyVHzkbJyzDj0XorI0EtZ+sqhJu9F6KEC+ohYf3+Q3HxFz+5AKZUOiDXz4BgFVM32wDbj09a6lQld8NhGMe17EA52MeZ0CAwEAAQ==", 84, 550},
    {"MFYxCzAJBgNVBAYTAkNOMTAwLgYDVQQKDCdDaGluYSBGaW5hbmNpYWwgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxFTATBgNVBAMMDENGQ0EgRVYgUk9PVDCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBANdda80QPx8FWdUFTTexDuyYK44VHfqTSxeCIXEQUtdRZHAWwlVpTY4VbZ+/DBvC4KNn1gyszyKur3dUKktMilNSesPuLt6zcSXB6V097qEvo/cqPMkjHWqrHaGn8fPsoNVEzxXPci8dY5fomfn9k6RUgExS1FKrLknfkM24X74/3qHKTSDUJeiEKVO3sYgf//rakJ8KqS1BP7HxGCnuFlksNEkaqAbXqIjSA3J6MuLqaE1uLJZle8pZ+vLi3e4wLPvMRqzEY+tvfzYrNHMSlH/fzCae8XJdUGVZj2mzh14yb8MYirWVj7B6N95aRTvHNuHvZ9E505dbc2IZSC2HHAb7dJggSXPwBdIbsaCjtxtw04hpuVrWOPRi3CWLeL/46H64XMmVT1+nLbkga89r3fUN9IK39LJmLhAo9pdae5YWjwEZLWxufzlYBmSDAYODw02S3TLGh6Q36RbOqi1orwqBZTpwwZutTW1UyiotS4Ubs4DmcEUNa1418H87uJzkBHCJEiWT2gqZImBqY2BOdgaYTr2DrR1YiiWF0sdlHi2Oxt+2xuF/igQhFSl08D6ckJ0MLvGKPlqqDAkex9U8o+2Xwx40+jj5CA7jwF0rg9FWasm2qFRTLngyZz2Cf3TQ++G2BWC5cNuOC/kTWG9xYBBSELnBQQnvch9nMXj/lgWNAgMBAAE=", 88, 550},
    {"MIGFMQswCQYDVQQGEwJHQjEbMBkGA1UECBMSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHEwdTYWxmb3JkMRowGAYDVQQKExFDT01PRE8gQ0EgTGltaXRlZDErMCkGA1UEAxMiQ09NT0RPIEVDQyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTB2MBAGByqGSM49AgEGBSuBBAAiA2IABANHey91yYIVhft15JEW1KtimfU+UgsGzkEAf5fhCiQ8HQEE7j3SjQmXDOB15Pr7d4oq9QNgSzaLFiMWrQlx9Er0KFC0/ogcbj9sLy8JWVulWwszmeLDPYn5aizvstMG6Q==", 136, 120},
    {"MIGFMQswCQYDVQQGEwJHQjEbMBkGA1UECBMSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHEwdTYWxmb3JkMRowGAYDVQQKExFDT01PRE8gQ0EgTGltaXRlZDErMCkGA1UEAxMiQ09NT0RPIFJTQSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAJHoVJLSClaxrA0k3cXPRGd0mSs3o30jcABxvFPfxPoqEo9LfxBWvZ9wcrdhf8lLDxenPeOwBGHu/xGXx/SGPgr6Plz5k+Y0etkUa+ecs4Wggnp2r3GQ1+z9DfqcbPrfsIL0FH75vsSmL09/mX+1/GdDcr0MANaJ62ss0+2PmBwUq37l42782KjkkiTaQ2tiuFX96sG8bLaL8w6NmuSbbGmZ+HhIMEXVreENPEVg/DKWUSe8Z8PKLrZr6kbHxyCgsR9l3kgIuqROqfKDRjeE6+jMgUhDZ05yKptcvUwbKIpcInu0q5jZ7uBRg8MJRk5tPpn6lRfafDNXQTyNUe0LtlyvLGMa31fIP7zpXcSbr0WZ4qNaJLS6qVY9z2+q/0lYvvCo//S4rek3+7q49As6+ehDQh6J2ITLE/HZu+GJYLiMKFasFB2cCudx688O3T2plqFIvTz3r7UNIkzAEYHsVjv206LiW7eyBCJSlYCTaeiOTGXxkQMtcHQC6otnFSlpUgK7199QalVGv6CjKGF/cNDDoqosIapHziicBkV2v4IYJ7TVrrTLUOZr9EyGcTDppt8WhuDY/0Dd+9BCiH+jMzouXB5BEYFjzhhxayvspoq3MVw6akfgw3lZ1iAar/JqmKpyvFdK0kuduxD8sExB5e0dPV4onZzMv7NR2qdH5YRTAgMBAAE=", 136, 550},
    {"MD0xCzAJBgNVBAYTAlVTMRIwEAYDVQQKEwlDZXJ0YWlubHkxGjAYBgNVBAMTEUNlcnRhaW5seSBSb290IEUxMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE3m/4fxzf7flHh4axpMCK+IKXgOqPyEpeKn2IaKcBYhSRJHpcnqMXfYqGITQYUBsQ3tA3SybHGWCA6TS9YBk2QNYphwk8kXr2vBMj3VlOBF7PyAIcGFPBMdjaIOlEjeR2", 63, 120},
    {"MD0xCzAJBgNVBAYTAlVTMRIwEAYDVQQKEwlDZXJ0YWlubHkxGjAYBgNVBAMTEUNlcnRhaW5seSBSb290IFIxMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA0DbUH+rdq+TRtub7IsDdEw1qeyITHJc8aGNmMpwDtY2kgYPaeDARz9yyK76Sv47kxBO+pGhM2gJoFnS+st0E5Gsq3TcfYCzb9fehfJW3DHCGLvE671L3zNOb+Yu+Dt8xt51oXJKm9eXzCjS1/3ui5Iehxq8XAO8Dke2pHE5xPdKLbIn0eIbmakmgzrXSsKub9vTULuNy+TbG6xW3JYw6/CUNsyJzIXTISpZhkvUvCxil9K3i7kG9AXn6loyNFwIwtPmveBqMtDYQEAcFcND0MZCKUcWGJnmyEYhexfAKVM1Jpr8CnNJEp+3jeO9GXm1x0XlwHEZfUenJN9xffml7Qd80ReA7hPShigo2njfMYlLhiQ0o+XojsQ09PZr9nYHvLJDAe0ROu0ngDkpWkrzLtd15F4mR3mGJdJKo4zKFvk6FpEtZyyvFeI5xVNACN5mM5Unq4FRypBEGLwuMwVu+taGwU26cuGCRH1lr+S30lAqXtezFdgNUG2VSukySVlE1oEDYKduuUnY7LTBAm4rQQla0t4gBpIc7U5bNoxaP82aqF7HHYODBQwUM7ptbYG8GXIdbJ/lAEZ6cM8G35TVXBX8nzhcgjBz88fvaMSlJ7fULhKdPwfZOwiic+u7grwf7MxF6IU8LIRC2QDqrIjoEnIubhIZymtKnpcS0dZGpKyMCAwEAAQ==", 63, 550},
    {"MFoxCzAJBgNVBAYTAkZSMRIwEAYDVQQKDAlEaGlteW90aXMxHDAaBgNVBAsMEzAwMDIgNDgxNDYzMDgxMDAwMzYxGTAXBgNVBAMMEENlcnRpZ25hIFJvb3QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDNGDllGlmx6mQWDoyUJJV8g9PFOSbcDO8WV43X2KyjQn+Cyu3NW9sOty3tRQgXstmzy9YXUnIo245Onoq2C/mehJpNdt4iKVzSs9IGPjA5qXSjklYcoW9MCiBtnyN6tMbaLOQdLNyzKNAT8kxOAkmhVECe5uUFoC2EyP+YbNDrihqECB63aCPuI9Vwzm1RaRDuoXrC0SIxwoKF0vJVdlB8JXrJhFwLrN1CTivngqIkicuQstDuI7pmTLtipPlTWmR7fJj6o0ieD5Wupxj0auwuA0Wv8HT4Ks16XdG+RCYyKfHx9WzMfgIhC59vpD++nVPiz32pLHxYGpfhPTc3GGYo0kDFUYqMwy3OU4gkWGQwFsWq4NYKpkDfePb1BHxpE4S80dGnBs8B92jAqFe7OmGtBIyT46388NtEbVncSVmurJqZNjBBe3YzIoejwpKGbvlw7q6Hh5UbxHq9MfPU0uWZ/75I7HX1eBYdpnDBfzwboZL7z8g81sWTCo/1VTp2lc5ZmIoJlXcymoO6LAQ6l73UL77XbJuiyn1tJslV1c/DeVIICZkHJC1kJWumIWmbat10TWuXekG9qxf5kBdIjzb5LdXF2+6qhUVB+s06RbFo5jZMm5BX7CO5hwjCxAnxl4YqKE3idMDaxIzb3+KhF1nOJFl0Mdp//TBt2dzhauH8XwIDAQAB", 92, 550},
    {"MHQxCzAJBgNVBAYTAlBMMSEwHwYDVQQKExhBc3NlY28gRGF0YSBTeXN0ZW1zIFMuQS4xJzAlBgNVBAsTHkNlcnR1bSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEZMBcGA1UEAxMQQ2VydHVtIEVDLTM4NCBDQTB2MBAGByqGSM49AgEGBSuBBAAiA2IABMQojqsYW2q+bmQ3Y+TN7Ks698yhuA6CSdeGKZ+hlPLjYHiYgXgGTfLsmg5XYIOftOYXLxqzXQJbiSM8whEFKqeIExjzUITXvTQsJ4lV/85M59+mHyjE8FTDuXy3U63rwg==", 118, 120},
    {"MH4xCzAJBgNVBAYTAlBMMSIwIAYDVQQKExlVbml6ZXRvIFRlY2hub2xvZ2llcyBTLkEuMScwJQYDVQQLEx5DZXJ0dW0gQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxIjAgBgNVBAMTGUNlcnR1bSBUcnVzdGVkIE5ldHdvcmsgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDj+32jcrrC8MkUh/VrAU7hbkAHum0nXX/3Wy2zWsdRX6ukMqZhh7ZuD4bSMAKX+NdpV6EYOV1qZHnGAVmsPDFKOHzSBNJLKOggXzsHosxNc9vzrk/HVtVap5aJ+vOraNQjhlknzwknvKxucoMcMHLf4KLp0uF0dRm9Kp57FVQEG9dDOa1VKMXiGrv0wOSuOEkzzHaFnzlF0qSe8hKMUfh85C1/9axf6xafsS3RusyRQndMJcmQOG/b8Mz7jh6XWT7VYE7mBSjtSXkTS7pI2y/5ctM5yv4f2DRy9bRAzzEBw+zeES0XXR+4UNFeGadp3gczKMpQlfmnVMtUhlBFqflJAgMBAAE=", 128, 294},
    {"MIGAMQswCQYDVQQGEwJQTDEiMCAGA1UEChMZVW5pemV0byBUZWNobm9sb2dpZXMgUy5BLjEnMCUGA1UECxMeQ2VydHVtIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MSQwIgYDVQQDExtDZXJ0dW0gVHJ1c3RlZCBOZXR3b3JrIENBIDIwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC9+Xj45tWADGSdhhuWZGc/IjoedQF97/tcZ4zJzFxrqZHmuULlIEub2pt7uZld2ZuAS9eEQCsn0+i6MLs+CRqnSZXvK0AkwpfHp+6bJe+oCgCXhVqqndwpyeI1B+twTUrWwbNWuKFBOJvR+zF/j+Bf4bE/D44WSWDXBo0Y+aomEKsq09DRZ40bRr5HMNUuctHFY9rnY3lEfktjJImGLjQ/KUxSiyqnwOKRKIm5wFv5HdnnJ63/mgKXwcZQkpsCLL2puTRZCr+ESv/f/rOf69me4Jgj7KZrdxYq28ytOxykh9xGc14ZYmhFV+SQgkK7QtbwYeDBoz1mo130GO6IyY0XRSmZMnUCMe4pJshrAua1YkV/NxVaI2iJ1D7eTiew8EAMvE0Xy02isx7QBlrd9pPPV3WZ9fqGGmd4s7+W/jTcvedSVuWz5XV710GRBdxdaeOVDUO5/IOWOZV7bIBaTxNyxtd9KXpEulKkKtVBRgkg/iKgtlswjbyJDNXXcPiHUv3a76xRLgezTv7QCdpw75j6VuZt27VXS9zlLCUVyJ4ueE742pyehizKV/Ma5ciSixqClnrDvFASadgOWkaLOusm+iPJtrCBvkIApPjW/jAux9JG9uWOdf3yzLnQh1vMBhBgu4M1t15n3kfsmUjxpKEV/q2MYo45VU85FrmxY53/twIDAQAB", 131, 550},
    {"MHoxCzAJBgNVBAYTAlBMMSEwHwYDVQQKExhBc3NlY28gRGF0YSBTeXN0ZW1zIFMuQS4xJzAlBgNVBAsTHkNlcnR1bSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEfMB0GA1UEAxMWQ2VydHVtIFRydXN0ZWQgUm9vdCBDQTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBANEtjru3NuptN5GfTpOnBeQpAyXOHIL3fJmfQQbN7aO6wNsJLMF83yl+S2Uvk6fUAWsDKBij2J0FwSrYRfGR3t870IACjM84D+qnXHgRpMHIhVwl09Oy5yXPEVSXqzXAHnYc7wBTnzncFKUsIiWzcnL8jbPlPggeFCo3C4g8yrD0yMKhrrzBvilnVeL8rVlc/r1XLLCQjcLtN7Z8mYi11QOaPRUNPTqoqEXwlU4lWR3NmGm708wyyY3vgf6tfYm7umATymWVZ6DzGfYDVtRq0yfioa2D8EoSInccBXPiGXFCwOx1RpqQWOBqjiulRjAEjhmyF+O+qbp/VvEkA9eyISh2DjYwTHnVQZqaqLg1ugw68kQbIIj3xSXXPcbjPkPdh/7E6vVTPkxl/ztKy3haaxdfDcfDT06aKqLtV00i4kaaPw+RNCR9VeOMlTfTGvAJKyzSyY20DQCrZyko2AH1GQS2Hb52/nJcxIXK0oBB3wWoo9WEkE8L8+A/mxnSN4k/8ntSHIz24fc8B5eMDqJZgQyykD3T41lG7Q+pp96Aa1qqB7YZy7xX85chegyxK3Q+69qnZy1MxJieNgl2Zmb8Gj/qSFQcvjC9gFC/fLXOAPYMYdnnJAPg4wGBDr3YhTSIvbI2qHtcCOVEgIxv+C/VIcodHND7xLWH0TpOx3a1NUi1AgMBAAE=", 124, 550},
    {"MEgxCzAJBgNVBAYTAkRFMRUwEwYDVQQKEwxELVRydXN0IEdtYkgxIjAgBgNVBAMTGUQtVFJVU1QgQlIgUm9vdCBDQSAxIDIwMjAwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATGy8co0fuE9ZrvQhQg4UNrbnWt/CsDhNR2kyXXWTtBZWse5jQqu3T2Es7obeer5DxOP0QIi80Wccu/kpn0pNc8UFRSkIWDeJRnZ6McCRk9dTSF3u1gfccMtEFSuW7l7kI=", 74, 120},
    {"MEgxCzAJBgNVBAYTAkRFMRUwEwYDVQQKEwxELVRydXN0IEdtYkgxIjAgBgNVBAMTGUQtVFJVU1QgQlIgUm9vdCBDQSAyIDIwMjMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCu/wlZkYAKSmjmJD+4p+TIOgo6Fs3JI2Ggk3Hyq4tzj6BnZWDSVGtjUW9JM+ByBxN9OM0GkgcpUmtOd2wE05X63UyM2V3BYX1L5yizRIF7Ua/dM7FofNZOTP4raLnKZmnE7F5Xf/cNx5w2NuUHYKzATOoIbO8GfE9bKHoI/JNdm/actIuGuiG59PDoWVoooTSEGiWRtrWP77L5gPr5PTwRctjjL4Z2xXkswamQk0aYZ8uDaqBQI6c79oE54O3wub9l8djLevvvcwPOAPR91+BdO2a43I66g8uHdgP8JdnnI28G/Wfz4P+EvEe/tRYYRmkUzAX329NJrGvMq+S1C0MkXktrTWff1rU+T3gflHEk6t5w/PGT/p6TWuSUWpdUDDV7X2zuAB8k7AO6AvV29J/Umu2FLDgiL8fYL3YRT/1sXOj1jieHfxlKIUeQHXmNHFv4z0qF5O2zW42+xGQoXUHEbqw4Wk8jdHSpEsP20rkRFTMHkdg7NzpjMAbRxSI2KGIjEOBGzJes1itdZCTV7hwO3vsIWnUq9mNtzgtCvtG6cBycIeUPMWkX1/wKtN7tgJzLkrSL9d5ZolgJpWNHC+FBMjRB2Zqx2aiwG1reDQ304rJdNYC5gdSEaZECy3XQjcW1PQmRCY8UoRR0eT7WyRUdpFlZItz2ikU9PBLWPl0yLwIDAQAB", 74, 550},
    {"MEgxCzAJBgNVBAYTAkRFMRUwEwYDVQQKEwxELVRydXN0IEdtYkgxIjAgBgNVBAMTGUQtVFJVU1QgRVYgUm9vdCBDQSAxIDIwMjAwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATxC92GQyAZ35eF6CJKm8+dmL+0BSbJy+Om0o/Fnnh7MYmpia0nPGUQgvzfw51O8DMjxNIy9Ryw3zMXXcXwsYr577m3FMopSsIPqX91ZUkqMGf0ZPfWGnfaw8KXYUJ7Sa0=", 74, 120},
    {"MEgxCzAJBgNVBAYTAkRFMRUwEwYDVQQKEwxELVRydXN0IEdtYkgxIjAgBgNVBAMTGUQtVFJVU1QgRVYgUm9vdCBDQSAyIDIwMjMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDYjqOJgAuyV1LcqVNMN7l/YxcT76dbI1tpdbCZChfBi8TbqODMMbrC8s1d6bf4Ha9qxJWH10fJldiCBFA9gQj/5D2zsdbFsv2ICduchOwlFxSHfzB4m2pYybZzKDw095n3f9Om+BxFfK0sjJQ/2GcQU34izU4lUfAlJDURXhDG7IdmiYFouswrnUdzH73NkaRyapyiGxigb+xQ9H1Awqgwz71zyBMrEBMei5qoOpRz0xhpCkr/wQED/3l/tUh/e+7oKW82TJVhhtj5onOK7q4vlu5ozT1NKEL5RSsyG0ZVFmqmSyn5u5VWv0Yd7B2THcBlsh+hQ65WnqCxj2sSt2BteAvKilztHpYOg6ZIlY07oyHErljGALKEtCOkloY1uNie2Kw0SZhjlcXLbUhH4vIuGB7QMavddOz53Iy4HI5oI7rQ81Dcz2WPczoyx3z+yoIiT76OYkdm5c2H4ujVDxif5QRyS0Y8EPJEwmRWcU516JzJJnTFfVnRClsPbf6edRwYxho6fNgNBMzNt0VlerGPuK6ESD6zek2oA+LifgEWWWgYQzOw0tywGkM17qXaqUZcroaBQQFKdCbsnwa/wgU3ZHV4KWj9xfXr/kf55IWw4XsxnaZ/cqO5xCwuzJlXDiEMRQGUZetlCcZjIgszSZJIPPzNzrA+jp6L+P5JxTVyRwIDAQAB", 74, 550},
    {"ME0xCzAJBgNVBAYTAkRFMRUwEwYDVQQKDAxELVRydXN0IEdtYkgxJzAlBgNVBAMMHkQtVFJVU1QgUm9vdCBDbGFzcyAzIENBIDIgMjAwOTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANOySs96R+91myP6Oi/WUEWJNTrGa9v+2wBoqOADER03UAifTUpolDWzU9GUY6cgVq/eUXjsKj3zSEhQPgrfRlWLJ23DEE0NkVJD2IfgXU42tSHKXzlABF9bfsyjxiupQB7ZNoTWSPOSHjRGICTBpFGOShrvUD9pXRl/RcPHAY9RySPocq60vFYJfxLLHLGvKZAKyVXMD9O0Gu1HNVpK7ZxzBCHQqr0ME7UAyiZsxGsMlFqVlNpQmvH/pStmMaTJOKDfHR+4CS7zp+hnUquVH+BGPtikw8paxTGA6Eian5Rp/hnd2HN8gcqW3o7tszIFZYQ05ub9VxC1X3a/L7AQDcUCAwEAAQ==", 79, 294},
    {"MFAxCzAJBgNVBAYTAkRFMRUwEwYDVQQKDAxELVRydXN0IEdtYkgxKjAoBgNVBAMMIUQtVFJVU1QgUm9vdCBDbGFzcyAzIENBIDIgRVYgMjAwOTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJnxhDRwui+3MKCOvXwEz75ivJn9gpfSegpnljgJ9hBOlSJzmY3aFS3nBfwZcyK3jpgAvDw9rKFs+9Z5JUut8Mxk2og+KbgPCdM03TP1YtHhzRnp7hhPTFiu4h7WDFsVWtg6uMQYZB7jM7K1iXdODL/ZlGsTl28So/6ZqQTMFexgaDbtCHu39b+T7WYxg4zGcTSHThfqr4uRjRxWQa4iN1438h3Z0S0NL2lRp75mpoo6Kr3HGrHhFPC+Oh25z1uxav60sUYgovseO3Dvk5h9jHOW8sXvhXCtKSb8HgQ+HKDYD8tSg2J87otTlZCpV6LqYQXY+U3EJ/pure3511H3a6UCAwEAAQ==", 82, 294},
    {"MGUxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xJDAiBgNVBAMTG0RpZ2lDZXJ0IEFzc3VyZWQgSUQgUm9vdCBHMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANnnKC9SPzZySYiTNPP4ah4xVICfrVRBtUfflqjUr4AtuQrPdf2JpX0k+uMiDCu8lRcLM78ZTUEGkAC9DE0Q/ge15xxuIlUxZZe90xfSHmLz2+psUIw/hAyWz7fLA+DKbaEUTBuJ3e0AsFJ8r5FssTgT0ekSCMAAsBwrEdp3cDabrs55h9yCcOYJdHBVaa+jaJ+/3bZ5s/KdcClV9Kv/lWHzyUBvHdG+k7vTiCq7nb9yWlZxOz/U89EK/ijvo+7Zma8D049gt/KSobG9iYkfMM3Dpi5iM64WAndEWueBCjynRC55uD8EvFygh+Ebr1GOzews+vj+bfA6fKqL5GeVMY0CAwEAAQ==", 103, 294},
    {"MGUxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xJDAiBgNVBAMTG0RpZ2lDZXJ0IEFzc3VyZWQgSUQgUm9vdCBHMzB2MBAGByqGSM49AgEGBSuBBAAiA2IABBnnvKxEZe3NuD9Y+42xV6lELQUV8u8L/xB0n7ViUl9mfh/l3BtFeQvMxlMKnY1dAtmpWd4CWvaVKg6NOEqKSca8xgM4B19V2n4JbuJ/XtBFIA9ZdhDWoCTwLd428mwpOQ==", 103, 120},
    {"MGExCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQq2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5WztCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQvIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQAB", 99, 294},
    {"MGExCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEczMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE3afZu4q4C/sLfyHS8L6+c/MzXRq8NOrexpu80JX28MzQC7phW1FGfp4tn+6OYwwX7Adw9c+ELkCDnOg/QW07rdOkFFk2eJ0DQ+4QE2xy3q6Ip6FrtUPOZ9wj/wMco+I+", 99, 120},
    {"ME4xCzAJBgNVBAYTAlVTMRcwFQYDVQQKEw5EaWdpQ2VydCwgSW5jLjEmMCQGA1UEAxMdRGlnaUNlcnQgVExTIEVDQyBQMzg0IFJvb3QgRzUwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATBRKHPEZdQmt4jgjUHzdDLGJ3S8X93NU873ZRyUu3CO/js+ntrWCDsma7J/GizdbnbCezIE/VOxgodZjBMux9HCjxhEEIpfKUIDuAi6dM1aM6bY5+EtZlNWKCO9VTnlck=", 80, 120},
    {"ME0xCzAJBgNVBAYTAlVTMRcwFQYDVQQKEw5EaWdpQ2VydCwgSW5jLjElMCMGA1UEAxMcRGlnaUNlcnQgVExTIFJTQTQwOTYgUm9vdCBHNTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALPQ9Ml5EZ39/GaB58zV5LzsgT5qNY4ut+fer/kHTc8wneoJC5m9bFfaGEq4eKw6OaimSKwucuW96/EazeekA6k/EbTYL4kW+5QBPbsv+BMFoXgcjijgReCD9FkblbOufgNF5b7CQv7u8jy2hROYMp0WqCnCCxw43J8xd1y/J6P8J6y3K710mxct8oHaXbDhIxc+iEoSI9Dqz53eAxexQkqgFkykbZPpPzruOnydWJ30To/8OyPIbbjiBdrM6+zDMfTXpylUgM9EW0xvMJ7zzN0flEOdTX9wcA3UOtE38Gydm8AUk1jvzUE4dbwTA5V8f+Nc6dUN1eJ8EGKqa/A9dvM/o+iwwf3vqldNrIanGLQpwSwOv2S+KYzYAi3NXC/yf+8V9AwVrAqw8dMNT2pNd5cBoPFmt7fO787s7KV1yqzj4WP3uKEEyLx7P10tFiJW7UhJ/qcveTAlm7prLT+dO8QX5x0u+/LPpvzjFCyWmCGMtJHpGWCD8jArBnNQ1Zg7BunHigxgjCj4Uptu4fZNuwYkm9crJj/9Ki9x9dYkvn8xng9t6I9PTaM//zXq30leQY+G+fF3eUsbtKNeL/tGAtBmE15ehU/O2HCIe84BtZaX1819/YL4wiTBygE5T42iwRRAH5xm1QwJRtby0NFIdlY6Q8u2ChE5uowTbAa1ns/rAgMBAAE=", 79, 550},
    {"MGIxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xITAfBgNVBAMTGERpZ2lDZXJ0IFRydXN0ZWQgUm9vdCBHNDCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAL/mkHNo3rvkXUo8MCIwaTPswqclLskhPfKK2FnC4SmnPVirdprNrnsbhA3EMB/zG6Q4FutWxpdtHauyefLKEdLkX9YFPFIPUh/GnhWlfr6fqVcWWVVyr2iTcMKyunWZanMylNEQRBAu34LzB4TmdDttceItDBvuINXJIB1jKS3O7F5OyJP4IWGbNOsFxl7sWxq868nPzaw0QF+xembud8hIqGZXV59UWI4MK7dPpzDZVu7Ke13jrclPXuU15zHL2pNe3I6PgNq2kZhAkHnDeMe2scS1ahg4AxCN2NQ3pC4FfYj1gj4QkXCrVYJBMtfbBHMqbpEBfCFM1LyuGwN1XXhm2ToxRJozQL8I11pJpMLmqaBn3aQnvKFPObURWBf3JFxGj2T3wWmIdph2PVldQnaHiZdpekjw4KISG2aadMreSx7nDmOu5tTvkpI6nj3cAORFJYm2mkQZK37AlLTSYW3rM9nF30sEAMx9HJXDj/chsrIRt7t/8tWMcCxBYKqxYxhElRp2Yn72gLD76GSmM9GJB+G9t+ZDpBi4pncB4Q+UDCEdslQpJYls5Q5SUUd0viastkF13nqsX40/ybzTQRESW+UQUOsxxcpyFiIJ33xMdT9j7CFfxCBRa2+xq4aLT8LWRV+dIPyhHsXAj6KxfgommfXkaS+YHS312amyHeUbAgMBAAE=", 100, 550},
    {"MGIxCzAJBgNVBAYTAkNOMTIwMAYDVQQKDClHVUFORyBET05HIENFUlRJRklDQVRFIEFVVEhPUklUWSBDTy4sTFRELjEfMB0GA1UEAwwWR0RDQSBUcnVzdEFVVEggUjUgUk9PVDCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBANmjFvDIdHR3m+8zDTsGflX8tWCPdoYSQn1WZj6Igu1yYw6ei900LAJRUcMZ/VlUhMnxa7NMsOnoRl04xqKnLhFXuoIVopyPbbCZSgry64lwY055xLdbvaJdsfJBAiutqTqj7HkK7F864/3vgDytNJsaq4gme1aigoYf6zWJg39frilOPbZu7K7B8CebruP07O+uf/eGPXJ666X7WU6n65WMIjl54S0Ij8y8kbhB9xTBI6nDrZpFRLOy1yzNxiniUBCuXMuCjhcYNn2X5oiasE00CfQsuVpmKrAXm54edp1KZjFB3z/7xQbvG7Z+GkY292RjO+M5GCPnZ3UU1XVXkje9vmobJlDyNiYGkMVwAWRtdmbhkdtuB8BhgC6yLi+McKfROzyzkeRutsQ7cPJskpcJzUd9GMDzu54P1ouuB7ZaD84LDEen5T64vX3HmzWgYZc6QXUXzCuWdyqSIR7ZlXYgZ2jPDb3f1h8JaprizHNxpC99EoC3UzBGXktUmQ9nyaXI8iDBguydEd/CAvsaO9HtIJrvZWSSEA0q4t5w8Rhngoxh3ri80S+c+w/QK+0bdrnkOVX4+KEduKqAAEyC57J/Cbi8MKAvDfVSno73krMKAB0AVJcG4LEH2ccPXGV9PG1ZV+TtpY3pQFOfFUugcfYaIePacAYhWBSHhXd5qoJ5AgMBAAE=", 100, 550},
    {"MEcxCzAJBgNVBAYTAlVTMSIwIAYDVQQKExlHb29nbGUgVHJ1c3QgU2VydmljZXMgTExDMRQwEgYDVQQDEwtHVFMgUm9vdCBSMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALYRAose46F3mzvcv5Q+t5WnQDyh/YL5fTIGgnH29ox/++jbvGoul5ejjEv5K/ax+c6EHbH5xZfe77nyo+m8Eolep6pSq/gjJ8uksZxj29eZfvAKXutopvTGWkcNTRAz406xE6PIGGxL7PwJkN+dZCklIwehtNI9LmDgz9IJh7vNSPBNwsJ6iIq7us9ZGdavj7AHsJ4x8YLBwN8upm1sGQ612H4mGkUDPbB5pJQorQ9/JuWoCP6W6DxolFPugzqIKxWWCbLgeowuddac66dWZI+WT2iuPZfChI/AvEDAC1y99oezNWysGFB/hOBMzZLTIOkzvFKZrzK1KbMlKrRI+XLhymT35oIQjeidwoqI+jhmivxj+QH5eP17XHf6dof67N+xDnmVV7S9Ju/WAdHrFgq7jgu1xcWKVavTrOqRSynMGaQyJU4q8WVE0ALOqs5JtOqffIOwQHvnQ6unbKOPfYmB+kyl/9WOw85L4LXYs45Fz3bA7UAr/VMPsKfVOw2xiqID3jGtzHfqb3s+1t+RIhLmvvrYMvwQYxRRct5d1haTvSloM+86ZuwHiibfE9dXZXgn3l5JFACiAH+aqCG2qbGVsKW5DRYR2sdsSDxA4H4NWs1WPNGXBbnLS+05S5zEP9JVE24ksNZx+vTBusztG/X+gUHYAJg9Osiuepg3GAWVAgMBAAE=", 73, 550},
    {"MEcxCzAJBgNVBAYTAlVTMSIwIAYDVQQKExlHb29nbGUgVHJ1c3QgU2VydmljZXMgTExDMRQwEgYDVQQDEwtHVFMgUm9vdCBSMzB2MBAGByqGSM49AgEGBSuBBAAiA2IABB9PM4czKYqhhN7LxyFYQYnqVp0rS4XGHUwnvH8mUXJv4p/Wo8rMRRRGi63vfoaM7LF+L/+pcZ0YhEUEQVVuK+omf7uQAeNLGbrkVJZFCbHVbJFErYQTjpqMDYAMMvbgJw==", 73, 120},
    {"MEcxCzAJBgNVBAYTAlVTMSIwIAYDVQQKExlHb29nbGUgVHJ1c3QgU2VydmljZXMgTExDMRQwEgYDVQQDEwtHVFMgUm9vdCBSNDB2MBAGByqGSM49AgEGBSuBBAAiA2IABPN0c6doi2CuQ7g1xYEwe0tJnfvBYc7m3ka9a9VhGDWuQN1z94mRMFrrPO6FfKJAdjupxrhH2CrnkpFqc+mxcjmfKZ+imNNfXliGZQ+hhGUG0dyLycdzyIxqL+XEq9Edig==", 73, 120},
    {"MFAxJDAiBgNVBAsTG0dsb2JhbFNpZ24gRUNDIFJvb3QgQ0EgLSBSNDETMBEGA1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABLjGedOPbCUOny45GRwDpK6a5TkHCRbKY7G5hviKV8FXzkL6c6H3ZUL/HsEAsm5zDv/HIeUYpKrZcT+o1LnOjB0=", 82, 91},
    {"MFAxJDAiBgNVBAsTG0dsb2JhbFNpZ24gRUNDIFJvb3QgQ0EgLSBSNTETMBEGA1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjB2MBAGByqGSM49AgEGBSuBBAAiA2IABEdFDpb7fV2/6TnRIfifC7bVex6SOkhZHPBiMS3Aeij+Gqdcs7bMl+dF1Fj60XdtQ6LAh2U0Ch963es8M6HFnU2kb0GVOH/JHoTr0Z5JkoeUhww6hUpmn51Zk02XYQaGSg==", 82, 120},
    {"MEwxIDAeBgNVBAsTF0dsb2JhbFNpZ24gUm9vdCBDQSAtIFIzMRMwEQYDVQQKEwpHbG9iYWxTaWduMRMwEQYDVQQDEwpHbG9iYWxTaWduMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzCV2kHkGeCIW9cCDtoTKKJ79BXYRxa2IcvxGAkPHsoqdBF8kyy5L4WCCRuFSqwyBR3Bs3WTR6/Usow+CPQwrrpfXthSGEHm7OxOAd4wI4UnSamIvH176lmjfiSeVOJ8G1z7JyyZZDXPesMjpJg6DFcbvW4vSBGDKSaYo9mk79svIKJHlnYphVzesdBTcdOA67nIvLpz70Lu/9T0A4QYz6IIrrlOmOhZzjN1BDiA6wLSnoemyT5AuMmDpV8u5BJJoaOU4JmB1sp93/5EU764gSfytQBVI0QIxYRleuJfvrXe3ZJp6v1/BE++bYvsNbOBUaRapA9pu6YOTcXbGaYWCFwIDAQAB", 78, 294},
    {"MEwxIDAeBgNVBAsTF0dsb2JhbFNpZ24gUm9vdCBDQSAtIFI2MRMwEQYDVQQKEwpHbG9iYWxTaWduMRMwEQYDVQQDEwpHbG9iYWxTaWduMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAlQfoc8pm+ewUyns89w0I8bRFCyyCtEjG61s8roO4QZIzFKRvf+kqzMawiGvFtonRxrL/FM5RFCHsSt0bWsbWh+5NOhUG7WRmC5KAykTec5RO86eJf094YwjIElBtQmYvTbl5KE1SGooagLcZgQ5+xIq8ZEwhHENo1z08isWyZtWQmrcxBsW+4m0yBqYe+bnrqqO4v76CY1DQ8BiJ3+QPefXqoh8q0nAue+e8k7ttU+JIfIwQBzj/ZrJ3YX7g6ow8qrSk9vOVShIHbf2MsonP0KBhd8hYdLDUIzr3XTrKotudCd5dRC2Q8YHNV5L6frxQBGM032uTGL5rNrI55KwkNrfw77YcE1eTtt6y+OKFt3OiuDWqRfLgnTahb1SK8XJWbi6IxVFCRBWU7qPFOJabTk5aC0fzBjZJdzC8cTflpuwhCHX85mEWP3fV2ZGXhAps1AJNdMAU7f05+4PyXhShBLAL6f7uj+FuC7IIs2FmCWqxBjplllnA8DX9ydoojRoRh3CBCqiadR2eOoYFAJ7bgNYl+dwFnidZTHY5W+r5paHYgw/R/98wEfmFzzNI9cptZBQselhP00sIScWVZBpjDnk99bOMylitnEJFeW4OhxlcVLFltr+Mm9wT6Q1vuC7cZ27JixG1hBSKABlwg3mRl5HUGie/Nx4yB9gUYzwoTK8CAwEAAQ==", 78, 550},
    {"MEYxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52LXNhMRwwGgYDVQQDExNHbG9iYWxTaWduIFJvb3QgRTQ2MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEnA6xz7fonlJ3dTT6pUanrTIZMrQHqSfKlLsM0goQx9qJsJcMcBMJAY7Y6kfqvrKAK838KA3brLykhjftcAgAdeqTC3suUpwjaCMGQ+ySL1OE2/tHFAfoX5RnXcl6gTwg", 72, 120},
    {"MEYxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52LXNhMRwwGgYDVQQDExNHbG9iYWxTaWduIFJvb3QgUjQ2MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEArKx0MuizZeW67UMmHaaJDUW6KYiypB1j3dPBLAlXiTmhVelnNHcMbuRVHVIl0hNrXuEdqbd9iTJfDZ6fLHpjYEAfprC2eI+ZVJYIWK7kBrxiBQIWv6+oIwO2lA+8bmzCy9Wmuwzp9sEC+yHeZt0Xq3RC7/B0LyX06mtVW5Dbnd9ehwpA+60Za/v3ymCI3trBj9au1X/UPIPu1xZMg0UzayfQhtAcLWvzq33xhan1KNKt7/OESxyH/BOjOnKiWhEr1idxJ+2BLW1mgZKHtBtYesw/CvpGT014XPgrSOMEhMtd9rRqs2X8Qp5RJiMgyz0U+YHtZRYATxpkl2YIz4x74yvAnfkU8hvxVmoWvyyFhc14OJrrQmoCNBiDF06UVvi2grXzlt09875/IHc+exkjayzUcnNDV33g+NdpTxc2BPnAkGA3Rd7mDNh0ja6com10XUK+BvXZZG4CEKyJsEw7B01AfiTFipiCeY6kp4IgjSP6J3HJ38ZBdKBN9pEW3EaMXyljMVlxDNhvwrYyffvmXVOmfhX8u3V8Xez49hcc7MdrGcvze/ArB6XZbHlUdmydHKZuDul5DKgjaqPfGzAxn7FUe/5qy2aq3GXQop5Kmgcha4GP28RZ+t4iwASc46pbNpPoPb16oZ0LdrELx539z5ioBsL4KqOhg6C3JXKlAuMCAwEAAQ==", 72, 550},
    {"MIGDMQswCQYDVQQGEwJVUzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTEaMBgGA1UEChMRR29EYWRkeS5jb20sIEluYy4xMTAvBgNVBAMTKEdvIERhZGR5IFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC/cWII8fpZNPcbyRij94BJWOkigxOmxSBDATuE8eaFSZ8n6vaEG06gtNtwmMcyAbEFPgdO7vT6Ty9ZMCLnqxlWa+KAB/zzFnWAOVF75fk1tnROqY2CE+S2P6kDg/qivooVan/eC8O2GRQFyurDqASUO0Z8Mg3zAGYiyI1pbTaMERi307IcYLQ4+gKMztPdRgfeCj7rXXzIfPuwK1OkkmJpUSUFYRpEgYwsqUOWI9+sOoGaDinFHKnpXR62np4wCjnO8YiA+0tdzDLshWJDJTQCVicBkbQ7cCo/brHonIgBfZ/U+dtTbWCdvyznWKu4X0b8zsQbAzwJ60kxXGlGs+BHAgMBAAE=", 134, 294},
    {"MGwxCzAJBgNVBAYTAkdSMTcwNQYDVQQKDC5IZWxsZW5pYyBBY2FkZW1pYyBhbmQgUmVzZWFyY2ggSW5zdGl0dXRpb25zIENBMSQwIgYDVQQDDBtIQVJJQ0EgVExTIEVDQyBSb290IENBIDIwMjEwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQ4CP6xoJbSeqyvSTrQwODDOyiq8XJtZQBHiIT8miZrqku6bAQKiF4X8lWH/DCwNOI0WFcahFPpMNmp8pZ0w1EfWEkxzJhOYBGHddNylJBPmxAlKqh4Lb6QQViQFXKnobc=", 110, 120},
    {"MGwxCzAJBgNVBAYTAkdSMTcwNQYDVQQKDC5IZWxsZW5pYyBBY2FkZW1pYyBhbmQgUmVzZWFyY2ggSW5zdGl0dXRpb25zIENBMSQwIgYDVQQDDBtIQVJJQ0EgVExTIFJTQSBSb290IENBIDIwMjEwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCLwuevZZsFZ5bJDSS50A5k/M7iJBgshH93UcsEETa4Xu1pcaee5CUJl2fBR8LPkRY2Yj04BOFRgv+s0rRp3S7sEaNF7mtrO0y/jI2kHp0Ruek4+XoODJjiIx3RTmPU57hBRPtrr2vaH9PFkYhbpImS0YHmjDlYoNZpQ6mtmFJYbtsK+2vPaPrjpF46RXOYB+pfAnLeDKWzn66pHbcds/yKWeducmWt9TCUIwfzghZLNZicU7svyuRa2ceNHfyYmfsspIJr8CofjgtfcVxcrkJ7KYmBywOjmcqIngtACUEz2+ZYev2umXDAWg/WE4ZxL3Zp/JDd2y1u0fKb9Rprnm8VjHrwSyigIjiAJGw2pDvyMJHzeBPPwT81q/EdESO1QyKeAZK3GALlEdGC2xUAzGE3wSp8muHQurNQRu6CrJ0x+Psj4gMASHCjCSZ5FVNg8zhcrTjqgQBjFLkzXt0L26BFBxozCfhNtKcCpmn0wlkFiGWFVq5Ly+DePH0tGsjp+x+jYUrWKhOtd0waGJuRD1jYBlTFl/iqPyCKpoWmd/am/Bzi7m6UMyqDUIQK5U+G+FBFeACB61to4yaNzHtcUfQULEC+GmAdenJhHR9jLYiqzqJFkAj8a76zUCpa/ahIGEbWkECSkAqEXmgx+OvtDdMdxn2ZGFVWJ2UujUXFJOzO4wIDAQAB", 110, 550},
    {"MIGqMQswCQYDVQQGEwJHUjEPMA0GA1UEBxMGQXRoZW5zMUQwQgYDVQQKEztIZWxsZW5pYyBBY2FkZW1pYyBhbmQgUmVzZWFyY2ggSW5zdGl0dXRpb25zIENlcnQuIEF1dGhvcml0eTFEMEIGA1UEAxM7SGVsbGVuaWMgQWNhZGVtaWMgYW5kIFJlc2VhcmNoIEluc3RpdHV0aW9ucyBFQ0MgUm9vdENBIDIwMTUwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAASSoEHoS4KEXOL4MRGZhmROCSUvnUEvCq41T3SVslFka41r5j9wlfAFREemcjhQdpUCWo6uKJ75LU6Z7yxIb0wlKejRcVvfHcF1N7TX+nt6QpxqClZafGkLqoAJJGx+wUY=", 173, 120},
    {"MIGmMQswCQYDVQQGEwJHUjEPMA0GA1UEBxMGQXRoZW5zMUQwQgYDVQQKEztIZWxsZW5pYyBBY2FkZW1pYyBhbmQgUmVzZWFyY2ggSW5zdGl0dXRpb25zIENlcnQuIEF1dGhvcml0eTFAMD4GA1UEAxM3SGVsbGVuaWMgQWNhZGVtaWMgYW5kIFJlc2VhcmNoIEluc3RpdHV0aW9ucyBSb290Q0EgMjAxNTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAML4qT8bifw8PARdPZA2sJE6eTxmWu9tOQFJGrS3z39NI1O3kADjEyoopjHxkQDjKOyuIUHOH9r9fRJbAYMPubBfmeHyEoOATQY+36yv56GIazGv8IvQGDO420VqNPQCgCQoCgIVlV52Kg2ZOhRb9svLU7wTTQGIN5QlG0K8ItiOo5ZeOtky2z7o8BBl7XThL6d8ryc0uyl9m7bPCcjl0wr8iGVldArccxxczUCxHNS2hIxMUM9ojqhZrsInToKiNd0U9B//snfVhy+qbn0kJ+fGyybm5f5nB2PYRQ3dOlllOVh6kplyPZyEXoghuNX0LPzZcFJPeLi9PCuLlZj1s9FozyAUfkxcX+eL5fU1gRk31xEIt2a+00rOg1cAOsOB+BfLkjZd0aPYdRvhiyfqekhB/UUZBq0nmU7BcEfdtZ+BUxLlsYxIXTFDF+OMxnpjlkspME6ETmIZXjzOl5ClfwHrneD4i4ndJZg9krZ+79nxUVF9LSbIaVlh4KxquCo2EQR6UL0yhL4v3HLV1x0WR+RHZiA/9JbFr44BeqUPemT1DRiH2a6I1fqEwTrAaSgt8g1oUarjpXfGpJAOoTeLMSNHwQkI6273eJvXgvyEIJlJGbYSRrH7RVUWqaNlrJwHD+pr3B8uBnLshogS5C3bXwUv5PAD0yYz54DCzUKhFzQLAgMBAAE=", 169, 550},
    {"ME8xCzAJBgNVBAYTAlRXMSMwIQYDVQQKDBpDaHVuZ2h3YSBUZWxlY29tIENvLiwgTHRkLjEbMBkGA1UEAwwSSGlQS0kgUm9vdCBDQSAtIEcxMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA9B5/UnMyDHPkvRN0o9QwqNCuS9i233VHZvR85zkEHmpwINJaR3JnVfSl6J3VHiGh8Ge6zCFovkRTv4354twvVcg3Px+kwJyz5HdcoEb+d/oaoDjq7Zpy3iu9lFc6uux55199QmQ5eiY29yTw1S+6lZgRZq2XNdZ1AYDgr/SEYYwNHl98h5ZeQa/rh+r4XfEuiAU+TCK72h8q3VJGZDnzQs7ZngyzsHeXZJzA9KMuH5UHsBffMNsAGJZMoYFL3QRtU6M9/Aes1MU3guvklQgZKILSQjqj2FPseYlgSGDIcpJQ3AOPgz+yQlda22rpEZfdhSi8MEyr48KxRURHH+CKFgeW0iEPU8DtqX7UTuybCeyvQqww1r/REEXgphaypcXTT3OUM3ECoWqj1jOXTyFjHluP2cFeRXF3D4FdXyGarYPM+l7WjSNfGz1BryB1ZlpK9p/7qxj3ccC2HTHsOyDry+K49a6SsvfhhEvyovKTmiKe0xRvNlS9H15ZFblzqMF8b3ti6RZsR1pl8w4Rm0bZ/W3c1pzAtH2lsN0/Vm+h+fbkEkj9Bn8SV7apI09bA8PgcSojt/ewsTu8mL3WmKgMa/aOEmem8rJY5AIJEzypuxC00jBF8ez3ABHfZfjcK0NVvxaXxA/VLGGEqnKG/uY6fsI/fe78LxQ+5oXdUG+3Se0CAwEAAQ==", 81, 550},
    {"MG8xCzAJBgNVBAYTAkhLMRIwEAYDVQQIEwlIb25nIEtvbmcxEjAQBgNVBAcTCUhvbmcgS29uZzEWMBQGA1UEChMNSG9uZ2tvbmcgUG9zdDEgMB4GA1UEAxMXSG9uZ2tvbmcgUG9zdCBSb290IENBIDMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCziNfqzg8gTr7m1gNt7ln8wlffKWihgw4+aMdoWJwcYEuJQwy51BWy7sFOdem1p+/l6TWZ5Mwc50tfjTMwIDNT2aa71T4Tjukfh0mtUC1Qyhi+AViiE3CWu4mIVoBc+L0sPOFMV4i707mV78vH9toxdCim5lSJ9UExyuUmGs2C4HDaOym71QP1mbpV9WTRYA6ziUm4ii8F0oRFKHyPaFASePwLtVPLwpgchKOesL4jpNrcyCse2m5FHomY2vkALgbpDDtw1VAliJnLzXNg99X/NWfFobxeq81KuEXryGgeDQ0URhLj0mRiikKYvLTGCAj4/ahMZJx2Ab0vqWwzD9g/KLg8aQFChn5pwckGyuV6RmXpwtZQQS4/t+TtbNe/JgERohYpSms0BpDsE9K2+2p20jzt8NYt3eEV7KObLyzJPivkaTv/ciWxNoZbx39ri1UbSsUgYT2uy1DhCDq+sI9jQVMwCFk8mB13umOResoQUGC/8Ne8lYePl8X+l2oBlKN8W4UdKjk60FSh0Tlxnf0h+bV78OLgAo9uliQlLKAeLKjEiafv7ZkGL7YKTE/bosw3Gq9HhS2KX8Q0NEwA/RiTZxPRN+ZItIsGxVd7GYYKecsAyVKvQv83j+GjHno9UKtjBucVtT+2RTeUN7F+8kjDf8V1/peNRY8apxpyKBpADwIDAQAB", 113, 550},
    {"ME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBTZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgxMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAregkc/QUN/ObnitXKByHvty33ziQjG485legePd1wqL+9Wpu9gBPKNveaIZsRJO2sWP9FBJrvx/S6jGbIX7RMzy6SPXded+zuP8S8SGaS8GKhnFpSmZmbI9+PHC/rSkiBvPkwOaAruJLj7eZfpQDn9NHl3yZSCNT6DiuTwpvgy7RSVeMgHS22i/QOI17A3AhG3XyMDz6j67d2mOr6xZPwo4RS37PC+j/tXcu9LJ7SuBMEiUMcI0DKaDhUyTsE9nuGb8Qs0qMP4mjYVHerIcHlPRjcewu4m9bmIHhiVw0eWx27zuQYnnm26SaLybF0BDhDt7ZEI4W+7f3qPfH5QIHmI82CJXn4jeWDTZ1nvsOcrEdm7wD+UkF2IHdBbQq1kHprAF2lQoP2N/VvRIfNS8oF2zSmMGoCWR3bkc3us6sWV5onX9y1onFBkEpPlk+3Sb1JMkRp1qjTEAfRqGZtac6UW6GO559cqcSBXhZ7T5ReBULA4+N0C8Fsj57ShxLcwUS/Mbq4FATfEOTdLPKdOeOHwEI0DDUW3E2tAe6wTAwXEi3gjuYpn1giqKjKYLMur2DBBuigwNBodYF8RvCtvCofIY7RqhIKojcdpp2vx9qpT0Zj+s482TeyCsNCij/99viFULUItAnXeF5/hjncIitTubZizrG3SdRbv+8ZPUzQ08CAwEAAQ==", 81, 550},
    {"ME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBTZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgyMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0HttwW+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9ItgKbppbd9/w+kHsOdx1ymgHDB/q", 81, 120},
    {"MEoxCzAJBgNVBAYTAlVTMRIwEAYDVQQKEwlJZGVuVHJ1c3QxJzAlBgNVBAMTHklkZW5UcnVzdCBDb21tZXJjaWFsIFJvb3QgQ0EgMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAKdQGd4/mT3UM0bxb1FhgrKpT49niV2E2VPdDCjZ1/D/rpVDcpn5tV18isFC4TFQdNGBDXzNmyGrQ+KsrV6GbvMJih9aMr2i65T56FwK7P+Y0q9xs7RTn06H75K8vexPMjCISxdeV8RTwvYCl43ZYiu/JB9ijd/DuClLSXg8k2CIIvyZ2jbIwqLULFQAZzVuc78CWPCk3eWwoiZ6yuA2pRkW9f23764/QPVtWgT9zjTKJNx0IxtdMxMSXcQBJfYw3QJdn+DVR7206xuhu0lJ2J9bAvOK5CSQ5GJPT8Gviw50F6jRcohqegFJzLRGecYXsdqYHgdZ+nUhhWXdkFbO+6ulYJ3EnflSsIu9h/mPKyMKI3Y79zPhyQDzaflLouBOvH6TOYQH90Rwfv4HWuWxrNEYzPI15UlJCMpWyT37Dxh9izvBE8JNj8lPDjfpH6EOat9iLss1BlF5LMglOPT6S6eJXJzS4w05hkp0fNVZh8I/TgxcUvQ991KC8eqjrP1JNBoo80GIOhPu6N7/mR1fusvoHvK5UGDAMdNz5e++oO0zC3S+ICDEZ2zwCAN6VYB/Rk6Wp/QePuH22AnhM2QrY9cyXp/5wHsPeG+XvJOa+ZwSkHh6gIcV13J0nFV0eLG64W5wBLpPoLpow3v/MfBzPT2UKrELQQ6g/k2IZWt5M7TXAgMBAAE=", 76, 550},
    {"ME0xCzAJBgNVBAYTAlVTMRIwEAYDVQQKEwlJZGVuVHJ1c3QxKjAoBgNVBAMTIUlkZW5UcnVzdCBQdWJsaWMgU2VjdG9yIFJvb3QgQ0EgMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALYilPykSK/oR2sK+yd25PI/ijt6SiwxKoyNsKnDMWuod3aEJrasgUINCOtVWLt6+LxlffKgbYuoR+lidh4R7ggU0bJEFvTq0PoeL17by3NBrrwAsEorQLKs4TtLwi2d5KGb7Bo6HvAIs9DkJDUHn5y0yVJt2wfKj7Vb8IPzT8ctpcity5UgpDEoV1ha5I0bmqueDQzyCjM5IjkKly7zU3e5REX9hMs2IIFZLZpvbUhIYcpM31PRr1K8RJ+rL2uDcu91gNoGMxtdyNpjxk3NrGYxzdHePocQNuG5pHrvYFCyy8qmVuA3r6s0Ezkl6Dlm5Jh6qhKYnFlmhj6t8bDKPgYPe/ARSzegRG17y6iMcfTVtZE2zPAVxiveURexl0xQPbGVWXwFfS0h1QC/AWeiXnumXPL3IvGQDZPbqkRRZsx9dgPraqgqOBmXdg1rimH5vPbudv1wK90pPPgKHltCHItWL1UbHKEutccW5viqPJKOabYBwbWGnYkPCziUVOjq3J49JbxTJu3VqzmqxUBMVKuytNnZ+Ndy2xy8bb1lX++INSpmL+72s2XwM418mEFpRg9DHGn6m7XQYWrNykvZTJBGqxVZoUdUKS6DKF8cwqKrchcABo5F7IviMz1/2hlE5GJyw98ixvJW1N1flXLtbV/3SANb/cUqoPZzI4QQGwHnAgMBAAE=", 79, 550},
    {"MDgxCzAJBgNVBAYTAkVTMRQwEgYDVQQKDAtJWkVOUEUgUy5BLjETMBEGA1UEAwwKSXplbnBlLmNvbTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMnTesoPHqynhugWZWqxwhtFMnGV2f4QW8yv56V5AY+Jw8ryVXH3d753lPNypCxE2J6SmxQ6oeckkAoKVo7F2CaU4dlI4S0+2gpy3aOZFdqBoof0e24md4lYrdbrDLJBenNubdt6eEHpCIgSfocuZhFjbFT7PJ1ywLwu/8K33Q124zrX97RovqL144FuwUZvXY3gTcZUVYkaMzEKsVe5o4qYw+w7NMWVQWl+dcI8IMVhulFHoCCQk6GQS/NOfIVFVJrRBSZBsLVNHTO+xAPIJXzBcNs79AktVCdIrC/hxKw+yMuSTFM5NyPs0wH54AlETU1kwOENWocivK0bo/4mtRXzp/yEGensoYi0RGmEg/OJ0XQGqcwL1sLeJ4VQJsoXuMl6h1YsGgEebL4TrRCstST1OJGh1kva8bvS3ke18byB9llrzxlT6Y0Vy0rLqW9E5RtBz+GGp8rQap+8TI0GM1qiheWQNaBiXBZO8OOi+gMatCxxs1gs3nsL2xoP694hHwZ3BgOwye+Z/MC5TwuGKP7Suerj2qXDR2kS4Nvw9hmL7Xtw1wLW7YcYKCwEJEx35EiKGsY7mtQPyvp10gFAWo15v4vPS8+qFsGV5K1Mij4XkdSxYuWC5YAEpAN+jb/af6IPl08M0w3719Hlcn4cyHf/W5oPt64FRuXxqBbsR6QXAgMBAAE=", 58, 550},
    {"MIGCMQswCQYDVQQGEwJIVTERMA8GA1UEBwwIQnVkYXBlc3QxFjAUBgNVBAoMDU1pY3Jvc2VjIEx0ZC4xJzAlBgNVBAMMHk1pY3Jvc2VjIGUtU3ppZ25vIFJvb3QgQ0EgMjAwOTEfMB0GCSqGSIb3DQEJARYQaW5mb0BlLXN6aWduby5odTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOn4j/NjrdqG2KfgQvvPkd6mJviZpWNwrZuuyjNAfW2WbqEORO7hE52UQlKavXWFdCyoDh2Tthi3jCyoz/tccbna7P7ofo/kLx2yqHWH2Leh5TvPmUpG0IMZfcChEhyVbUr02MelTTMuhTlAdX4UfIASmFDHQWe4oIBhVKZsTh/gnQ4H6cm6M+f+wFUoLAKApxn1ntxVUwOXewdI/5n7N4okxFnMUBBjjqqpGrCEGob5X7uxUG6k0QrM1XF+H6cbfPVTbiJfyyvm1HxdrtbCxkzlBQHZ7Vf8wSN5/PrIJIOV87VqUQHQd9bpEqH5GoP7ghu5sJf0dgYzQ0mg/wu1+rUCAwEAAQ==", 133, 294},
    {"MGUxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xNjA0BgNVBAMTLU1pY3Jvc29mdCBFQ0MgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgMjAxNzB2MBAGByqGSM49AgEGBSuBBAAiA2IABNS8PQJCdUETI82ABIYCUS9qqIFiC2XM9sqdHm9KZlGiA9mdkfq2FrGMbt58zdt5pi/Ou85xL+WlqyjsYwRmmfj68pMQBeGBKELjxmj05huEYEqJr+15DzvO8fZE9QF4wA==", 103, 120},
    {"MGUxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xNjA0BgNVBAMTLU1pY3Jvc29mdCBSU0EgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgMjAxNzCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMpbvpQzjCmVkRYKlb1HYsGJ85k230aQyaXteGpvR5Fo+CdnUDMdoab74OVDo4QCVwFdnEhAglMQvL/HO2iQtoIt5fRl0MxtGcyV+XusSpStDt5LQx2HB5ITkICDZDU5BPzl6Wyzth9QlDhlUFwXRrm2hbUctRfo1kWd2LImsMrEcEquYKTds9ns/DvVV3K8P8jJst5La/gjbAPABb2Vx81zO2aAZOMarC75RwXyBrabc/V4M1vHofsnKqG0mpGMkdM6gj52QLTNUmFRcCg/xcVa8smMSbsUW03I/2dNTBKWrfX+eKiXh9f9XiCA3KFLIvvUia26zkeXR1V7j0XIZyiElRxoMO/vSeA1e2TnmLCU2k2FOz5VxCivV/OeE9tGJ58eol5Eg6SlytUTs0s/xOPC5oZhpFIwuXogT28POFPLMwwTK4/Wmr0qyC2xHH1LUcpH0Ugncl2H69VF5khlna9SkLpbohhlVxKfaLnUFWuUxGkimPQz4O35UY5BUMk0T3aQrPw4wdjhe7nj45ThRmnLDgpQaxO6rA83WrcStZCBHlauVyKG2cnS0ddR46s7xlX9Hg7TdArR2qrqabiXKI9IxAf4UkM69MpVNSywpmrAnPnygeESasBF2Wezzv8jookKVNQUuSqo1+z5q80lWDJ5j5BbmDnECAbBrH8OPQClAgMBAAE=", 103, 550},
    {"MGkxCzAJBgNVBAYTAktSMSYwJAYDVQQKDB1OQVZFUiBCVVNJTkVTUyBQTEFURk9STSBDb3JwLjEyMDAGA1UEAwwpTkFWRVIgR2xvYmFsIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC21PGTXLVAiQqrDZBbUGOukJR0F0Vy1ntlWilLp1agS7gvQnXp2XskWjFlqxcX0TM62RHcQDaH38dq6SZeWYp34+hInDEW+j6RscrJo+KfziFTowI2MMtSAuXaMl3Dxeb57hHHi8lEHoSTGEq0n+USZGnQJoViAbbJAh2+g1G7XNr4rRVqmfeSVPc0W+m/6imBEtRTkZazkVrd/pBzKPswRrXKCAfHcXLJZtM0l/aM9BhK4dA9WkW2aacp+yPOiNgSnABIqKYPszuSjXEOdMWLyEz59JuOuDxp7W87UC9Y7cSw0BwbagzivESq2M0UXZR4Yb8ObtoqvC8MC3GmsxY/nOb5zJ9TNeIDoKAYv7vxvvTWjIcNQvcGufFt7QSUqP620wbGQGHfnZ3zVHbOUzoBppJB7ASjjw2i1QnK1sua8e9DXcCrpUHPXFNwcMmIpi3Ua2FzUCaGYQ5fG8Ir4ozVu53BA0K6lNpfqbDKzE0K70dpAy8i+/Eozr9dUGWokG2zdLAIx6yo0es+nPxdGoMuK8u180SdOqcXYZaicdNwlhVNt0xz7hlcxVs+Qf6sdWA7G2POAN3aCJBitOUt7kinaxeZVL6HSuOpXgRM6xBtVNbv8ejyYhbLgGvtPe31HzClrkvJE+2KAQHJuFFYwGY6sWZLxNUxAmLpdIQM201GLQIDAQAB", 107, 550},
    {"MIGnMQswCQYDVQQGEwJIVTERMA8GA1UEBwwIQnVkYXBlc3QxFTATBgNVBAoMDE5ldExvY2sgS2Z0LjE3MDUGA1UECwwuVGFuw7pzw610dsOhbnlraWFkw7NrIChDZXJ0aWZpY2F0aW9uIFNlcnZpY2VzKTE1MDMGA1UEAwwsTmV0TG9jayBBcmFueSAoQ2xhc3MgR29sZCkgRsWRdGFuw7pzw610dsOhbnkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDEJF5zvkttFMOh9OOXkG7SMEUePO5n2WTgGop/yjDKg+MgweP0OtOUXxp8W22/ME+EJ/afH0m8xpkKkPIP9X9DhDdjUYt6pXD8eljNjpvtw0ZshHBd2vMBkCP8TjCpfuEnY+ftZDyguMkzY/4WkP+wuP3XqMDAlEMLttVZpp5W0CQfcHmv2zlUDWV12RVBlAGvXuz2jfH/rWT+IJrXXOv+ph8IZKOLdlWtHjsoYC6HJeiqrx/GZEYgt3B/PN5I25ZTtzl35BrixxaEdpdbL7sZFYX4aYX1maep8jSnqbamA/xvhj1UfHYEm2v5QF0ANMcumXWd5YgDqk34A9JCdsAbAgMAqIs=", 170, 294},
    {"MEsxCzAJBgNVBAYTAkNIMRkwFwYDVQQKDBBPSVNURSBGb3VuZGF0aW9uMSEwHwYDVQQDDBhPSVNURSBTZXJ2ZXIgUm9vdCBFQ0MgRzEwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQXL/oSvKwY8wr0RNZ2Qp6z6B+3ealYtvhl0TohT6jro76kMnLztgHJK/13hW5T3a3qqi4lkrbpIRGor7VMC/OWYJo75+oaeC60PeUo3hyAulxuDdsZpeM/nCq4QEvd5k8=", 77, 120},
    {"MEsxCzAJBgNVBAYTAkNIMRkwFwYDVQQKDBBPSVNURSBGb3VuZGF0aW9uMSEwHwYDVQQDDBhPSVNURSBTZXJ2ZXIgUm9vdCBSU0EgRzEwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCqrvSrgs/75TcL59WWrZDoSyncVWDjzLyzvC2SueSjevGBtJ5yckPfP6sLJrTuexpp+yjQclxKbWmZ8GMeDNKx/9aMNNDu26xIueqwFI7YB6kkmN3pCb6oIhtZOdEnh9wczfj7s+vpk3jtD843fCZ3bqHYLCFM5IpPxxM8bsfVE5eyqNskaYNW02nLghK7nRvy+DTymCsqjgRn9uOHoR2tbs42dA5eMzvL21GXlGqVPM4YWm5LxvwHjy4auUr3ZDQp3KaNUOGNi0vlSBtuLoAQP+SfG2U/EbTqV2mftADrhSSZJPUhnZeq+zR/AmsVkK27nloZf6SM2PptKPw4x+NMrWrO2U6Tko7MDGe/C0uWzmZnU2jLFxGOWfesnBu5jmhEtxiv5uUPZdyVCbCTErUfPpSlx4h1IbHeCSQqTOK87ExnR8IpiLkKuvnBdM6MGCZl2vdvxox7aFwL7mPAXkvxTsyfLw/h6Jp6k/HgyNu/J+ZlKXs24DMVc/Kda4QIaCs2BysnzHjYaocOR3T0qqATXWR+9NsUrvs65C/BZeO5ekBs8Aa3eyib1+FfOHOUrNlwky3chK9GHKJ6LD+BJkLn1NjFbIRmEYt3a1Qco7XYEPCuKfdnCIgXXLiXec/qKyruWDPlbekpqmUBDYIT7CULXSxAchUp05As9xpD1WrwaQIDAQAB", 77, 550},
    {"MG0xCzAJBgNVBAYTAkNIMRAwDgYDVQQKEwdXSVNlS2V5MSIwIAYDVQQLExlPSVNURSBGb3VuZGF0aW9uIEVuZG9yc2VkMSgwJgYDVQQDEx9PSVNURSBXSVNlS2V5IEdsb2JhbCBSb290IEdCIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2Be3HEokKtaXscriHvt9OO+Y9bI5mE4nuBFde9IllIiCFSZqGzG7qFshISvYD06fWvGxWuR51jIjK+FTzJlFXHtPrby/h0oLS5daqPZI7H17Dc0hBt+eFf1Biki3IPShehtX1F1Q/7pn2COZH8g/497/b1t3sWtuuMlk9+HKQUYOKXHQuSP8yYFfTvdv37+ErXNku7dCjmn21HYdfp2nuFeKUWdy19SouJVUQHMD9ur06/4oQnc/nSMbsrY9gBQHTC5P99UKFg29ZkM3fiNDecNAhvVMKdqOmq0NpQSHiB6F4+lT1ZvIiwNjeOvgGUpuuy9rM2RYk61pv48b74JIxwIDAQAB", 111, 294},
    {"MG0xCzAJBgNVBAYTAkNIMRAwDgYDVQQKEwdXSVNlS2V5MSIwIAYDVQQLExlPSVNURSBGb3VuZGF0aW9uIEVuZG9yc2VkMSgwJgYDVQQDEx9PSVNURSBXSVNlS2V5IEdsb2JhbCBSb290IEdDIENBMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAETOlQwMYPchi82PG6s4nieUqjFqdrVCTbUf/q9Akkwwsin8tqJ4KBDdLArzHkdIJuyiXZjHWd8dvQmqJLIX4Wp2OQ0jnUsYd4XxiWD1AbNTcPasbc2RNNpI6QN+a9WzGR", 111, 120},
    {"MEgxCzAJBgNVBAYTAkJNMRkwFwYDVQQKExBRdW9WYWRpcyBMaW1pdGVkMR4wHAYDVQQDExVRdW9WYWRpcyBSb290IENBIDEgRzMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCgvlAQjunybEC0BJyFuTHK3C3kEakEPBtVwedYMB0ktMPvhd6MLOHBPd+C5k+tR4ds7FtJwUrVu4/sh6x/gpqG7D0DmVIB0jWerNrwU8lmPNSsAgHaJNM7qAJGr6Qc4/hzWHa39g6QDbXwz8z6+cZM5cOGMAqNF34168Xfuw6cwI2H44g4hWf6Pser4BOcBRiYz5P1sZK0/CPTz9XEJ0ngnjybCKOLXSoh4Pw5qlPafX7PGglTvF0FBM+hSo+LdoINofjSxxR3W5A2B4GbPgb6Ul5jxaYA/qXpUhtStZI5cgMJYr2wYBZupt0lwgNm3fME0UDiTouG9G/lg6AnhF4EwfWQvTA9xO+oabw4m6SkltFi2mnAAZauy8RRNOoMqv8hjlmPSlzkYZqn0ukqeI1RPToV7qJZjqlc3sX5kCLliEVx3ZGZbHqfPT2YfF72vhZooF6uCyP8Wg+qInYtyaEQHeTTRCOQiJ/GKubX9ZqzWB4vMIkIG1SitZgj7Ah3HJVdYdHLiZxfokqRmu8hqkkWCKi9YSgxyXSthfbZxbGL0eUQMk1fiyA6PEkfM4VZDdvLCXVDaXP7a3F98N/ETH3Goy7IlXnLc6KOTk0k+17kBL5yG6YnLUlamXrXXAkgt3+UuU/xDRxeiEIbEbfnkduebPRq34wGmAOtzCjvpUfzUwIDAQAB", 74, 550},
    {"MEgxCzAJBgNVBAYTAkJNMRkwFwYDVQQKExBRdW9WYWRpcyBMaW1pdGVkMR4wHAYDVQQDExVRdW9WYWRpcyBSb290IENBIDIgRzMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQChriWyARjcV4g/Ruv5r+LrI3HimtFhZiFfqq8nUeVuGxbULX1QsFN3vXg6YOJkApt8hpvWGo6t/x8Vf9WVHhLL5hSEBMHfNrMWn4rjyduYNM7YMxcoRvynyfDStNVNCXJJ+fKH46nafaF9a7I6JaltUkSs+L5u+9ymc5GQYaYDFCDy54ejiK2toIz/pgslUiXnFgHVy7g1gQyjO/Dh4fxaXc6AcW34Sas+O7q414AB+6XrW7PFXmAqMaCvN+ggOp+oMiwMzAkd056OXbxMmO7FGmh77FOm6RQ1o9/NgJ8MSPsc9PG/Srj61YxxSscfrf5BmrODXfKEVu+lV0POKa2Mq1W/xPtbAd0jIaFYAI7D0GoT7RPjEiuA3GfmlbLNHiJuKvhB1PLKFAeNilUSxmn1uIZoL1NesNKqIcGY5jDjZ1XHm26sGahVpkUG0CM62+tlXSoREfA7T8pt9DTEceT/AFr2XK4jYIVz8eQQsSWu1ZK7E8EM4DnatDlXtas1qnIhO4M15zHfeiFuuDIIfR0ykRVKYnLP43ehvNURG3YBZwjgQQvD6xVu+KQZ2aKrr+InUlYrAoosFCT5v0ICvybIxo/gbjh9Uy3l7ZizlWNof/k19N+IxWA1ksB8aRxhlRbQ694Lrz4EEEVlWFA4r0jyWbYW8jwNkALGcC4BrTwV1wIDAQAB", 74, 550},
    {"MEgxCzAJBgNVBAYTAkJNMRkwFwYDVQQKExBRdW9WYWRpcyBMaW1pdGVkMR4wHAYDVQQDExVRdW9WYWRpcyBSb290IENBIDMgRzMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCzyw4QZ47qFJenMioKVjZ/aEzHs286IxSR/xl/pcqs7rN2nXrpixurazHb+gtTTK/FpRp5PIpM/6zfJd5O2YIyC0TeytuMrKNuFoM7pmRLMon7FhY4futD4tN0SsJiCnMK3UmzV9KwCoWdcTzeo8vAMvMBOSBDGzXRU7Ox7sWTaYI+FrUoRqHe6okJ7UO4BUaKhvVZR74bbwEhELn9qdIoyhA5CcoTNs+cra1AdHkrAj80//ogaX3T7mH1urPnMNA3I4ZyYUUpSFlob3emLoG+B01vr87ERRORFHAGjx+f+IdpsQ7vw4kZ6+ocYfx6bIrc1gMLnia6Et3UVDmrJqMz6nWB2i3ND0/kA9HvFZcba5DFApCTZgIhsUfei5pKgLlVj7WiL8DWM2fafsSntARE60f75li59wzweyuxwHApw0BiLTtIadwjPEjrewl5qW3aqDCYz4ByA4imW0aucnl8CAMhZa634RylsSqiMd5mBPfAdOhx3v89WcyWJhKLhZVXGqtrdQtEPREoPHtht+KPZ0/l7DxMYIBpVzgeAVuNVejH38DMdyM0SXV89pgR6y3e7UEuFAUCf+D+IOs15xGsIs5XPd7JMG0QA4XN8f+MFrXBsj6IbGB/kE+V9/YtrQE5BwT6dYB9v0lQ7e/JxHwc64B+27bQ3RP+ydOc17KXqQIDAQAB", 74, 550},
    {"MFoxCzAJBgNVBAYTAkpQMSYwJAYDVQQKEx1TRUNPTSBUcnVzdCBTeXN0ZW1zIENvLiwgTHRkLjEjMCEGA1UEAxMaU0VDT00gVExTIEVDQyBSb290IENBIDIwMjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATs3MUy2713NBdI0LXZ9puTT4aULl+KcHdHtdpmidFR3bRoWJY5u240EItRlJ+TpBcATnkGMRexdD80bzJSqJxeOy72SwMrclkGALAs5WHr6fAldO+yj90SP8ZRgfmYF2g=", 92, 120},
    {"MFoxCzAJBgNVBAYTAkpQMSYwJAYDVQQKEx1TRUNPTSBUcnVzdCBTeXN0ZW1zIENvLiwgTHRkLjEjMCEGA1UEAxMaU0VDT00gVExTIFJTQSBSb290IENBIDIwMjQwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDhOOLNTDPFsierxPHXWBoVg2Tl877fjLtPIzid6HRSAvkkhlsk0tPPbH/8v8HvX7mbpfqcYivt3iUUkEa2M7rvuFc7P3fORxZpRObdVs4CMGW3hhbGVxxwEde5nujdD7hH6iuasF0d4nUJNQQby21BsoiXcbs5lhucfz+k/xyM+5v/SQNU24zOnvGxVFE46KxC3nd8ygkuViCh5tu4ymE6owK2OQntHp58Qx8unxQBWL1lotGfvoRP8ImST3bmd26658LgFq1LiadcWbE3S9RdvSKP0Hw78PyCLFASxVIPgYrwVZE+HdtV3/pvN2QcYmPLbVcjTJ6NWiZlRtGs5827PRug8ZXWm3Xx8ULf0gdAS2Hc4Wpvk0M7cv4D1s2pOADJSBGYifi5yANz9GL8qbdXnW55mPLX/KTSrAlVp0BVWsK3nqI15c7IS/MkAMaANDwTU91qpy3wLKfP/jtF5Azra8Vg7TzBxCSuOZf+16yKSjVXXLJp/oR8/NQXOZobL3G+QLB1ubXkR1OiU6MTQfZVvn8A8M7IIUSiSJgaZdMt0BSXTApngjKOQcbPLSOGlpcRcT8YlwS8jpXMRyGNoEvTcQnSH2kbg6p4g7Jwq8CjcD5Nt63J+yuBjIfNTRrv+pSMZq3UACrWdWWMykqqmKc9Ox/930ff0VNR4kvtOnPONQIDAQAB", 92, 550},
    {"MH8xCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEQMA4GA1UEBwwHSG91c3RvbjEYMBYGA1UECgwPU1NMIENvcnBvcmF0aW9uMTQwMgYDVQQDDCtTU0wuY29tIEVWIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgRUNDMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEqhJHkJgb++/DQAeDIE7xMIKiBtHykoZh8vYhaMoAxMfqQwBUhtz9H98AuEFiXNxwFjLeH5nUzMUHyAgfYRYHUT19XAdT4zU4jN/Nn9kuDUq2GS5acFoG7b7wobDK0Akp", 129, 120},
    {"MIGCMQswCQYDVQQGEwJVUzEOMAwGA1UECAwFVGV4YXMxEDAOBgNVBAcMB0hvdXN0b24xGDAWBgNVBAoMD1NTTCBDb3Jwb3JhdGlvbjE3MDUGA1UEAwwuU1NMLmNvbSBFViBSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5IFJTQSBSMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAI82ZUDh1k3A17TpRtpr6jNHzUz5fX2+vS098Nt44Yal2boJV2jtVz6g0AhBg+coQSQf43IV0AEa+15wI7LLnznjz8VOxpJtJsZ7u7PaJ50KhumBNwX+8HFx7MMc6WOiFxSd7xtn04VVAgLWScnMWuGx928yn8nUO4hBqJy9y6vbbXsJH6JMcpDaKwj8zzxUzmcPqM9dlhkLxONy663RfR0n75LrEL9b6zuvz4DdwdKWBFt6fqSpPDh2pGKOoDle6nfPXQBZj2YsPgeiowUmEWmX6oW3D5YLS8hA4VC6LorL9w+aIud/mjcTzfJNE2sh0cDMIvKhRvZEaZzKYTUHAG/WYQgR6rq49umzYOVNueyfFGbJV1jbzYdp+IqGEgNHv2YTdqx3fTQkhYPN16qckBqfISx/eLdkuNjopvR4s1XLhNIyxHiuo49h3c4IU63siPwV5JoN5p8ad85Mj7gUFT1inIY4BgBmEuRZdlpTwAKYohAraER7jnnOM0p2qluBFhu1itjQAHteYrQJ1oZjDqYFlUm6KIuIk7I0HNikVW63HNDemVU7I/Qi4PkpZibsIFB320oLj77lAmBwQV7UrlA5IhQmy7I7c3RVRwd5gTmoMBNE5QSKrpYTJUIPuVPEm/zN5BzePPqr1gZKH2emmDAc3Szb3BiVV2bG/1yLVvV3AgMBAAE=", 133, 550},
    {"MHwxCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEQMA4GA1UEBwwHSG91c3RvbjEYMBYGA1UECgwPU1NMIENvcnBvcmF0aW9uMTEwLwYDVQQDDChTU0wuY29tIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgRUNDMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAERW6pUMSmIzaeXyiNF8uWImQ/3HqOHcwIs6JxJLqOSbkEG0eWWKstlcjtngg1yCfriYxTWOtiiv7wWw9rMVJjQTuJzezsto0Z0zQH3LvGBn/CRZXsy3+oI+AJ6YH680fT", 126, 120},
    {"MHwxCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVUZXhhczEQMA4GA1UEBwwHSG91c3RvbjEYMBYGA1UECgwPU1NMIENvcnBvcmF0aW9uMTEwLwYDVQQDDChTU0wuY29tIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgUlNBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA+Q/doyt9y9Aq/uxnhabnLhu6d+Hj9a+k7PpKXZHEV0drGHdrdvL9k+Q9D8IWngtmw1aUnheDhc5W7/IW/QBi9SIJVOhlF05BueBPRpeqG8i4bmJeabFf2yoCfvxsyvNB2O3Q6Pw/YUjtsAMUHRAOSxngu07shmX/NvNeZwILnYZVYf16OO3+4hkAt2+hUGJ1dDyg+sglkrRueiLH+B6h47LdkTGrKx0E/6VKBDfphaQzK/3i1lU0fBmkSmjHsqjTt8qhk4jrwZe8jPkd2SKEJHTHBD1qqSmTzOu4W+H+XyWqNFjIwSNUnRuYEcM4nH49hmylD0CGfAL0XAJPKMuucZ8POsgz/hElNer8usVgPdl8GNWyqdN1eANyIso6wx/vLOUuqfqeLLZRRv2vA9bqYGjqhRY2a4XpHsCz3cQk3IAqgUFtlD7I4MmBQQCeXr9/xQiYohgsQkCz+W84J0tOgPQ9gUfgiHzqHM61dVxRLhwrfxpyKOcAtdF0xtfkn60Hk7ZTNTX8N+TD9l0WviFz3pIK+KBjaryWkmo++LxlVZve9Q2JJgT8JRqmJWnLwm3KfOJZX5es6+8uyLzXG1k8K8zyGciTaydjGc/86Sb4ynGbf5P+NGeETpnr/LN4CTNwumamdu0bc+sapQ3EIhMglFYKTixsTrH9z5wJuqIz7YcCAwEAAQ==", 126, 550},
    {"ME4xCzAJBgNVBAYTAlVTMRgwFgYDVQQKDA9TU0wgQ29ycG9yYXRpb24xJTAjBgNVBAMMHFNTTC5jb20gVExTIEVDQyBSb290IENBIDIwMjIwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARFKTVz+sK4I84UfaixTaBbNu4qLFPDYAk1siRmJmnAs5XWXZJAGQ7GpRNw9O8SUShd58y9+TyFwc+UkMkrzpJCWFln/ZQnEGSMTwSxTUnke0+b9ecI+AOI96fDkksZVIE=", 80, 120},
    {"ME4xCzAJBgNVBAYTAlVTMRgwFgYDVQQKDA9TU0wgQ29ycG9yYXRpb24xJTAjBgNVBAMMHFNTTC5jb20gVExTIFJTQSBSb290IENBIDIwMjIwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDQpAlyT0CIEmE+NSOe7vZ0zy97WD3OPA0QKJAvl/eMSNig2CWxTLARTBdzUNAiSmO7gdMpbtW1CT4mGH+yEn+TmLev8Da/8u4Ynpw7UsVHGV1082Rm1V3HaLS/GxwGo7yPQCO2HsaEvVHEGznBldIp7Euuey2/Of20Yt6We0HGnKDgBnL78AeXCTmBdK/3NFkRVwrCW8Ek9DFzMILGnboC9z58RF+DDfPx3SBpFglQ4tRVtuCAcnZuTEe3dVVZtFN02ZTGQa1YijFmDx6iGylATi/fe+YWLC38v+zztPq+GPabSdTuBW7ZNPOc8ewBi9Egxg+gtbwXTkh7UcL86VxpN0dms2j4FSjwudOkFcxaT7pScKMSRd3Guk77wtD3qFInbW55tYz8e4zBFkzugH++8Ha+QVMSM65aOEKr1w8+QY12BzLVq4n2TmfZsUJ1I27zzUKy/FX1U4cXO8AzWPFS0vmApPDo8DuLOMykxpB/D5z9i9Gjz9qDp2nJUDbVXAXSCkF022MRN8GloJZLHowWEneulDR7Hn/CZgDkqoPqipCtzjZETdFR6bwf82oF/cB0HyUZQFFu6oJRQN+buQgqBgLVIxwT1unb28awest7J5v74NVGJO0QS2NLpQWPurgdK6b6keKSUr3s62eXbZotn4EyBWcy+0gIP9kluAQlLwIDAQAB", 80, 550},
    {"MFExCzAJBgNVBAYTAlBMMSgwJgYDVQQKDB9LcmFqb3dhIEl6YmEgUm96bGljemVuaW93YSBTLkEuMRgwFgYDVQQDDA9TWkFGSVIgUk9PVCBDQTIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC3vD5QqEvNQLXOYeeWyrSh2gwisPq1e3YAd4wLz32ohswmUeQgPYUM1ljj5/QqGJ3a0a4m7utT3PSQ1hNKDJA8w/Ta0o4NkjrcsbH/ON7Dui1fgLkCvUqdGw+0w8LBZwPd3BucPbOw3gAeqDRHu5rr/gsUvTaE2g0gv/pby6kWIK05YO4vdbbnl5z5Pv1+TW9NL++IDWr63fE9biCloBK0TXC5ztdyO4mTp4CEHCdJckm1/zuVnsHMyAHs6A6KCpbns6aH5db5BSsNl0BwPLqsdVqc1U2dAgrSS5tmS0YHF2Wtn2yIANwiieDhZNRnvDF5YTy7ykHNXGoAyDw4jlivAgMBAAE=", 83, 294},
    {"MF8xCzAJBgNVBAYTAkdCMRgwFgYDVQQKEw9TZWN0aWdvIExpbWl0ZWQxNjA0BgNVBAMTLVNlY3RpZ28gUHVibGljIFNlcnZlciBBdXRoZW50aWNhdGlvbiBSb290IEU0NjB2MBAGByqGSM49AgEGBSuBBAAiA2IABHb6maluIO3513fjBzuo2z1fOOirVaZWT9ZI6ux/LarDssV57JlhfxB5xwJa+QQ39TQ1K3fOfyCPUqMAiezVp6JtW+NLkpOggPUBlNzwaAceze7+JVK1IEMcG/7rGc5Dow==", 97, 120},
    {"MF8xCzAJBgNVBAYTAkdCMRgwFgYDVQQKEw9TZWN0aWdvIExpbWl0ZWQxNjA0BgNVBAMTLVNlY3RpZ28gUHVibGljIFNlcnZlciBBdXRoZW50aWNhdGlvbiBSb290IFI0NjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAJO+1TZSddgBI6AcR0JJ7mO2tyH9xJXVSCsmfBRTENp5/Su3LaTULPrqMt1Jwrm9D0g9e1qYVK+eXTF0Twf8UCHdpM9oTxsSY20lmUwqmfNIMGH6gXwepwhK3D4rHB8YTHGqNYyt+G7oO0rZ5ZQC1omEE6ptyE8zzFCWN5Iz3F+I559U2UjwmEPWZv2fFzhDxQFRC9fjIw8UXVsU50u+3fTI2gM30dY5oSFRMIOwbdcwTpZbkfBwJKu/RYFkQw29ITovPOmeDcsgtUInzNpvm+5kMJA5zZNlgSExtSNQMzci4zjt+DEwzEX+YvnRXTJ5QoffasxWGUBNzqq7+bV2SZTxJ/iRpYPlBrNjDoDc4BJVgKY7ZrQ5hy3I8NDRFOnkDU0O9l1XcsU7HEdWneLV+4FhjMxNgJA0W7fXFHXc2ARIn8DBKIi06RzKp7HxVrd7SUxZ5SAVqIQCKfo4lGmaSQaPzR95FBcSDIN63h+xl+75l3gopMhEkul9JgWmWHKbeRPYEV+uxThiNGiyhjCO+JBhnjJs9Qc2zaJMbuyKNu3y5pkVRHDDfLycOcC04Wv3gyUjV9kSgOVJ8HUP743rHJtUKLQhPPx8Cv/ve2t1/4sdoBkFq/r4K4FC6Di6u/uq/T3g88rfTpeXKe3zGFbppZasvcOQmLLg+aLUpkdDfG3PAgMBAAE=", 97, 550},
    {"MFExCzAJBgNVBAYTAkpQMSMwIQYDVQQKExpDeWJlcnRydXN0IEphcGFuIENvLiwgTHRkLjEdMBsGA1UEAxMUU2VjdXJlU2lnbiBSb290IENBMTQwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDF0nqh1oq/FjHQmNE6lPxauG4iwWL3pwon71D2LrGeaBLwbCRjOfHw3xDG3rdSINVSW0KZnvOgvlIfX8xnbacuUKLBl422+JX1sLrcneC+y9/3OPJH9aaakpUqYllQC6KxNedlsmGy6pJxaeQp8E+BgQQ8sqVb1MWoWWd7VRxJq3qdwudzTe/NCcLEVxLbAQ4jeQkHO6Lo/IrPj8BGJJw4J+CDnRugv3gVEOuGTgpa/d/aLIJ+7sr2KeH6caH3iGicnPCNvg9JkdjqOvn90Ghx2+m1K06Ckm9mH+Dw3EzsytHqunQG+bOEkJTRX45zGRBdAuVwpcAQ0BB8b8VYSbSwbprafZX1zNoCr7gsfXmPvkPx+SgojQlD+Ajda8iLLCSxjVIHvXiby8posqTdDEx5YMaZ0ZPxMBoH064iwurO8YQJzOAUbn8/ftKChazcqRZOhaBgy/ac18izju3Gm5h1DVXoX+WViwKkrkMpKBGk5hIwAUt1ax5mnXkvpXYvHUC0bcl9eQjs0Wq2XSqypWa9a4X0dFbD9ed1Uigspf9mR6XU/v6eVL9lfgHWMI+lNpyiUBzuOIABSMbHdPTGrMNASRZhdCyvjG817XsYAFs2PJxQDcqSMxDxJklt33UkN4Ii1+iW/RVLApY+B3KVfqs9TC7XyvDf4Fg/LS8EmjijAQIDAQAB", 83, 550},
    {"MFExCzAJBgNVBAYTAkpQMSMwIQYDVQQKExpDeWJlcnRydXN0IEphcGFuIENvLiwgTHRkLjEdMBsGA1UEAxMUU2VjdXJlU2lnbiBSb290IENBMTUwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQLUHSNZDKZmbPSYAi4Io5GdCx4wCtELW1fHcmuS1Iggz24FG1Th2CeX2yF2wYUleDHKP+dX+Sq8bOLbe1PL0vJSpSRZHX+AezB2Ot6lHhWGENfa4HL9rzatAy2KZMIaY8=", 83, 120},
    {"MGExCzAJBgNVBAYTAkpQMSUwIwYDVQQKExxTRUNPTSBUcnVzdCBTeXN0ZW1zIENPLixMVEQuMSswKQYDVQQDEyJTZWN1cml0eSBDb21tdW5pY2F0aW9uIEVDQyBSb290Q0ExMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEpKVvYAMDw70x9NMXnCuEdazl/T1Xbtdjv+YEiZKOgZzj6UduypASyBPgp533ZXQfbBCy6OTp722FMplEsV79zHYQ2Fu9osb51kLkV3bckMI1qUuIPBJHbVz/SU8aSlCx", 99, 120},
    {"MF0xCzAJBgNVBAYTAkpQMSUwIwYDVQQKExxTRUNPTSBUcnVzdCBTeXN0ZW1zIENPLixMVEQuMScwJQYDVQQLEx5TZWN1cml0eSBDb21tdW5pY2F0aW9uIFJvb3RDQTIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDQFTlSsVKzusVZgsRdUq46Q2WAS8fylrzbNpfWpmSMqF7w4woc99+XPUuu9l3sIbVBq825fnafvvk+NjSgO8H2MRFFdJM9V4DF+YmZyuWratS12kGQEMHW1kKJwr/0OBKVTFQF9zbkRYN7FGXW3AxN0d5+DKs7xBW+OlamWm92aVKpernI62qaXVLQLQprNRYJEITQaso6BgA3R+R+V08/i+tnuIiqxb5TVbKRxH25sIUZBngu22Ea+oX1SpGh5xbVjqI535S4cB8oP4v8QF5jgzyDKhqZa8/eWWo7/G8W1x/9ShDrToIWOqwnDFPxrdUksGsDUMEtPBbdRDQnGnX7AgMBAAE=", 95, 294},
    {"MIGPMQswCQYDVQQGEwJVUzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UEAxMpU3RhcmZpZWxkIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC97cED/PaP/AKxb1ufSNmdeeKitwNhVhjDR7bXyj01LolD96Fpm96KGv0TIJy0SXcyKVb9ueyM3SL6ctwnYZfu9lqE7G4ZuYks3IRb1XT7a1/FiaUQUolGVfS4dRzmf+RUrkv4VXJXAhn4F3FZ6x4oB3TFnUi+bLT0pLDzZDd5ksDsRl5/4W1TTGKvzR8LY7s6nfv8eQCYYXTPJoJAY/OycmoZDZnK1A51zDf7i4nBWfFif1+zX2Uw+Ke3TXZaHnZeNMDollaZirPwf6TNvdwyMXyRz+BfEfhrqklc0ZmU0aLjY1sJdrVWYuFLdB2W1CbUCARZ0JgODube/MPsH5DxAgMBAAE=", 146, 294},
    {"MIGYMQswCQYDVQQGEwJVUzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjE7MDkGA1UEAxMyU3RhcmZpZWxkIFNlcnZpY2VzIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVDDrEKvlO4vW+GZdfjohTsR8/y8+fIBNtKTrID30892t2OGPZNmCom15cAICyL1l/9of5JUOG52kbUpqQ4XHj2C0NTm/2yEnZtvMaVq4rtnQU68/7JuMauh2WLmo7WJSJR1b/JaCTcFOD2oR0FMNnngRoOt+OQFodSk7PQ5E751bWAHDLUu57fa4657wx+UX2wmDPE1kCK4DMNEffud6QZW0CzyyRpqbn3oUYSXxmTqM6bam17jQuug0DuDPfR+uxa40l2ZvOgdFFRjKWcIfeAg5JQ4W2bHO7ZOphQazJ1FTfhy/HIrImzJ9ZVGif/L4qL8RVHHVAYBeFAlU5i38FAgMBAAE=", 155, 294},
    {"MFExCzAJBgNVBAYTAkNIMRUwEwYDVQQKEwxTd2lzc1NpZ24gQUcxKzApBgNVBAMTIlN3aXNzU2lnbiBSU0EgVExTIFJvb3QgQ0EgMjAyMiAtIDEwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDLKmjiC8NXvDVjvHClO/OMPE5Xlm7DTjak9gLKHqquuN6orx122ro10JFwB9+zBvKK8i5VUXu7LCTLf5ImgKO0lPaCoaTo+nUdWfMHamFk4saMla+ju45vVs9xzF6BYQ1t8qsCLqSX5XH8irCRIFucdFJtrhUnWXjyCcplDn/L9Ovn3KlMd/YrFgSVrpxxpT8q2kFC5zyEEPThPYxr4iuRR1VPuFa+Rd4iUU1OKNlfGUEGjw5NBuBwQCMBauTLE5tzrE0USJIt/m2n+IdreXXhvhCxqohAWVTXz8TQm0SzOGlkjIHRI36qOTw7D59Ke4LKa2/KIj4x0LDQKhySio/YGZxH5D4MucLNvkEM+KRHBdvBFzA4OmnczcNpI/2aDwLOEGrOyvi5KaM2iYauC8BPY7kGWUleDsFpswrzd34unYyzJ5jSmY0lpx+Gs6ZUcDj8fV3oT4MM0ZPlEuRU2j7yrTrePjxF8CgPBrnh25d7mUWe3f6VWQQvdT/TromZhqwUtKiE+shdOxtYk8EXlFXIC+OCeYSf8wCENO7cMdWP8vpPlkwGqnj73mSiI80fPsWMvDdUDrtaclXvyFu1cvh43zcgTFeRc5JzrBh3Q4IgaezprClG5QtO+DdziZaKHG29777YtvTKwP1H8K4LWCDFyB02rpeNUIMmJCn3nTsPBQIDAQAB", 83, 550},
    {"MIGCMQswCQYDVQQGEwJERTErMCkGA1UECgwiVC1TeXN0ZW1zIEVudGVycHJpc2UgU2VydmljZXMgR21iSDEfMB0GA1UECwwWVC1TeXN0ZW1zIFRydXN0IENlbnRlcjElMCMGA1UEAwwcVC1UZWxlU2VjIEdsb2JhbFJvb3QgQ2xhc3MgMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKpf2htf6HOR5dpc9KLmR+XzaFVgBR0CpLObWfMeiq80rfwNwtlIGe5pj8kg/CGqBxntsFysZcdf7QJ8e3wtG9a6uYDCGIIWhPpmsAjGVCOB5M25ST/2T243SCg4D8W+52hw/TmXTdLHmJFQqsREsyN9OUfpUmLWEpNetzGWQgX7dqceo/XC/Ol6xWypcU/qy3i8YK/H3vTZy75+M6VulIPwNPohq+qOcqA/pN4wW++GTWqVW0NEqBAVHOUBV8WY8eYGKJGqIMW3UyZRQ7ILEZVY4cAPdtnAjXyB83Jwnm/+Go7ZXzXGsm80fL5IT+JaOdfYnXien4Y+A14Zi0Si1ccCAwEAAQ==", 133, 294},
    {"MIGCMQswCQYDVQQGEwJERTErMCkGA1UECgwiVC1TeXN0ZW1zIEVudGVycHJpc2UgU2VydmljZXMgR21iSDEfMB0GA1UECwwWVC1TeXN0ZW1zIFRydXN0IENlbnRlcjElMCMGA1UEAwwcVC1UZWxlU2VjIEdsb2JhbFJvb3QgQ2xhc3MgMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL11k/BiIm8kruB6dqx9vdkk1bi3/M3wQuDreIhWXpuaVB1NDIr2089w9FK12JME40aGcUFKK/AqLFUD1kjD4Dk47fJcPD9EvJM9YatOzQ2+8CAnWA5EfwQah6XXlhQ2kNBJe6F1+xprc7H4zqkJLPJT1cMURLiGpfaLKznaozNU2fpyGvciFRyIkWt/ZuXDaoCwJPPfhkWI/Rl/dYcfH7EbCnMkW7ll4CxUyGDTZhc/4cxUM3ORAjqmf3t2OaIflrY4rrXIk3Qdnrm05WCdL1bR4OteW0wScAxsRCCrEdj0GfbSnFI35/q2wjE7StQUma3HGvVdX/oHuHwNH9aDHrMCAwEAAQ==", 133, 294},
    {"MIHSMQswCQYDVQQGEwJUUjEYMBYGA1UEBxMPR2ViemUgLSBLb2NhZWxpMUIwQAYDVQQKEzlUdXJraXllIEJpbGltc2VsIHZlIFRla25vbG9qaWsgQXJhc3Rpcm1hIEt1cnVtdSAtIFRVQklUQUsxLTArBgNVBAsTJEthbXUgU2VydGlmaWthc3lvbiBNZXJrZXppIC0gS2FtdSBTTTE2MDQGA1UEAxMtVFVCSVRBSyBLYW11IFNNIFNTTCBLb2sgU2VydGlmaWthc2kgLSBTdXJ1bSAxMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAr3UwM6q7a9OZLBI3hNmNe5eA027n/5tQlT6QlVZC1xl8JoSNkvoBHToP4mQ4t4y86Ij5iySrLqP1N+RAjhgleYN1Hzv/bKjFxlb4tO2KRKOrbEz8HdDc72i9z+SqzvBV96I01INrN3wcwv61A+xXzry0tcXtAA9TNypN9E8Mg/uGz8v+jE69h/mniyFXnHrfA2eJLJ2XYacQuFWQfw4tJzh03+f92k4S400VIgLI4OD8D62K18lUUMw7D8oWgITQUVbDjlZ/iSIzL+aFCr2lqBs23tPcLG07xxO9WSMs5uWk99gL7eqQQESolbuT1dCANLZGeA4fAJNG4e7p+exPFwIDAQAB", 213, 294},
    {"MFAxCzAJBgNVBAYTAlRXMRIwEAYDVQQKEwlUQUlXQU4tQ0ExEDAOBgNVBAsTB1Jvb3QgQ0ExGzAZBgNVBAMTElRXQ0EgQ1lCRVIgUm9vdCBDQTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMb4yh7ZCSB+HWxOzo/jRzNEnMfJaao6W3jucNKS+ASzUlIdZ3Iood+LXZUK/urN7fcpzvBvf6zNPe+zHEVq9yiQ8WFXxQzEo1Bd3tS1yxnKgLl1zinO0oUi7AJjzEQwINrqkVtW5h0c1Z1mxz/fhspLU8TZjbId6vjcJ1OjR+FhzH21sPjuc5HFznNvzu4QHxoGz+knYMVPGeTrziImRddgmd3OTzfgf+djrbC4WbjQBmg1YNM2rnFDBPFpZXh88x/zyiifWiCVZrTNt+6PeKRFGOkmL42bKSixpLc6bbnUHDhyRVixXuvwKJu3gsr9z9YzD5/7l56xHJye6l9e26rdVOkwIShtjnnzdZKMJv7cxfbDsN9EWUOjtgMo9ggwqg0z4e+cqQci41lbQI/aiLdpCKi3Iy5ECVk3W8fjF/Ii6245UsXeVKeYyUsgldxGiV+0EvmFKY7ryCcVIMBL1Mx8DGw0DCabJjGmPKf22dBLomT/O5lBcsHgcJfxJLsrxHQisaxrIjIk03gqwMChL/FSBck/73Zm4kXYDT2tlcjHiSbID66nAy77wV/6IOFwrbBlIDczYLDVr9cMHMKQcNdKGLx+AbCw6xUeRAbNpE/oDNHDIBDhVGWetlHQGnZrQlpYdjTqtzcZri51+ZblwVn3lFcpJY06TKtNmkHQXyYDAgMBAAE=", 82, 550},
    {"MFExCzAJBgNVBAYTAlRXMRIwEAYDVQQKEwlUQUlXQU4tQ0ExEDAOBgNVBAsTB1Jvb3QgQ0ExHDAaBgNVBAMTE1RXQ0EgR2xvYmFsIFJvb3QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCwBdvI64zEbooh745NnHEKH1Jw7W2CnJfF10xORUnLQEK1EjRsGcJ0pDFfhQKX7EMzClPSnIyOt7h52yvVavKOZsTuKwEHktSz0ALfUPZVr2YOy+BHYC8rMjk1Ujoog/h7FsYYuGLWRyWRzvAZEk2tY/XTP3VfKfChMBwqoJimFb3u/Rk28OKRQ4/6ytYQJ0lM793B8YVwm8rqqFpD/G2Gb3PpN0Wp8DbHzIh1HrtsBv+baz4X7GGqcXzGHaL3SekVtTzWoWH1EfcFbx39Eb7QMAfCKbAJTibc46KokWofwpFFiFzlmLhxpRUZyXx1EcxwdE8tmx2RRP1WKKD+u4ZqyPpcC1jcxkt2yKsi2XMPpfRaAok/T54igu6idFMqPVMnaR1sjjIsZAAmY2E2TqNGtz99sy2sbZCilaLOz9qC5wc0GZbpuCGqKX6mOL6OKUohZnkfs8O1CWfe1tQHRvMq2uYiN2DLgbYPoA/pyJV/v1WRBXrPPRXAb94JlAGD1zQbzECl8LibZ9WYkTunhHiVJqRaCPgrdLQABDzfuBSO6N+pjWxnkjMdwLfS7JLIvgm/LCkFbwJrnu+8vyq8W8BQj0FwcYeyTbcEqYSjMq+u7msXi7Kx/mzhkIyIqJdIzshNy/MGz19qCkKxHh53L46g5pIOBvwFItIm4TFRfTLcDwIDAQAB", 83, 550},
    {"MF8xCzAJBgNVBAYTAlRXMRIwEAYDVQQKDAlUQUlXQU4tQ0ExEDAOBgNVBAsMB1Jvb3QgQ0ExKjAoBgNVBAMMIVRXQ0EgUm9vdCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALB+crikA5Tmp94JOJFKEUCHp3xZZBR7tREQ3f6/1cC7VuKFJfQ1cg/4U9BB4UQBwrQcwzFCFkeFMyJ2sgpvD+UlUE+Fhr6/mC4QZx6+EQWGBZDEWdB8eBCwgFy34ccrdct8n6610Z0jN2On3EKiLZIEG1DBe7g+G8lWBIsvUputqVbpwf+tqViHMLaB95dF/BlXOytv5Ef0mUX+HfH4l6OIHTccXI/gdiWaUPigVP9EkHYj0jLGw6sGv/z7v/OtfZJiAlsp0zWjk5pDZGBdsvoy/zsEr01AavnH4+8j/WvL5Q+LOA3uCvz+D5ifMDHdbFJl+YuBviLhHFgDupEbiQcCAwEAAQ==", 97, 294},
    {"MGMxCzAJBgNVBAYTAkRFMScwJQYDVQQKDB5EZXV0c2NoZSBUZWxla29tIFNlY3VyaXR5IEdtYkgxKzApBgNVBAMMIlRlbGVrb20gU2VjdXJpdHkgVExTIEVDQyBSb290IDIwMjAwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATOv/5XqL/VqvcQms280RGivWdCzJDrFRiQ2aLNDCol6z5PzrXSjw/zNdpDiwKAvm9RJB0PayvKn8JvUDLlNyC2IP+IDQ9tSbvbBqSHkJKU9AnQz3/IgAvBl7O7NSfJwhs=", 101, 120},
    {"MGMxCzAJBgNVBAYTAkRFMScwJQYDVQQKDB5EZXV0c2NoZSBUZWxla29tIFNlY3VyaXR5IEdtYkgxKzApBgNVBAMMIlRlbGVrb20gU2VjdXJpdHkgVExTIFJTQSBSb290IDIwMjMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDtNaGBgPPLSmlbwvtRg64m/eFu84ESfXFA/4d1Qikh7YFSLN8SwRmEicG9xSjV1UtsRNZM2weWSlV6yjaCBDaopfwn9knx1XKekfkj1nB7u/WbweyTzxnqZX6IcKBz/Pb/tVZi4XNqNJg+grislVP0AaAnB3KjAFOg5LKrgzhXMyWUn75IHZjho7qeXM0EcVF9dXir81mqxOBgvo+DUrh1GkE17bzzOmPpqRRF1+ZS0W7S3rzj9Qs75uDEvUNkE6bO9Jg3bIqVqJfIRw/wXhCL5x0c/rE7oAUzaAVBgsEDKwHI549Nq+i19s1rRLXn3Yvs6iW0ACJXTbCxsjHBFs7//RSEt0f6svFw3tuLbDZYpHyzEdHDd39ftiXgDcXSs/m4uHfbN3FxR+NgGE8ktnU3eLmjYq+9yXKOL8y7rtvkFVIZBzP7arctS5AognP+GIs1jdunBGq+6sFNNjsWNpEy77ZAiZFD4PKiqwQu5vJMDhY0IKyHwS1+yWZHFxQRpPP3oSSJq9gayKFcsaP3jG3IAclPyezE/KxRM9HIg9HJnx3URzQpPsuwDvqDCyhY5SncP3yon8m2Crum6EYWD5ble+RqekhtdpgFpdxtHkIeQtoa4FL3tYPAGnt4NSw49R/9SaMu0lljv4CwjJNzyzWmmZUiYWUDYPsvk0v6mpyAOwIDAQAB", 101, 550},
    {"MEoxCzAJBgNVBAYTAlNFMRkwFwYDVQQKDBBUZWxpYSBDb21wYW55IEFCMSAwHgYDVQQDDBdUZWxpYSBFQyBUTFMgUm9vdCBDQSB2MzB2MBAGByqGSM49AgEGBSuBBAAiA2IABMHIlhVDLbmFKUpW0iK4dpryT6emYOeS31JPwWnWPmkWRrAkTbPX40sQfHI9mpR7Rbktu3ngg6W+BBSXSechtMCnBmWXj/EaVlmV5cY1jD2HoTfhBQ3AacpCNMLJK4NpZQ==", 76, 120},
    {"MEsxCzAJBgNVBAYTAlNFMRkwFwYDVQQKDBBUZWxpYSBDb21wYW55IEFCMSEwHwYDVQQDDBhUZWxpYSBSU0EgVExTIFJvb3QgQ0EgdjMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCxXz0obX2EJ/hLUW+TwPdPIMRGGZy/HwXsqZvhYBPHeKN7WJ3cofFETRMqZw0JsBDntu8cURhriFHaXbRWNVp0TGo5b7aV330xsSI76NFU7P4FvEvEmeYbAKoj4F+5OONVF4PGzGNC+OAGwKVo7+ybmLvSeVlxYZG0iS9YMrc02RPlG3Bdz87w1MVWltSpPpxGMiPeiUBuVH6VF/gV/Lmj5D19puNif1n1LiK2u4SVwQV/0uOTt/B1PJxP+u8lb3D8HcbZrevR//Nc05Wu4gFyoTqmRKiykAJTZsbjZ9gqtsz8Zaenu76n0WvPiNo3Clvhge4R5LwGtq01w/xfraNci+8ofGWwwMkKdvhTwnMU4uxTqaiFVjnwF1mu+LQaT1Q66qaCgbp2yQeiSyNlOUrrUP8ifOYSgDXIBgkUNXKyNHENQ671wDAA1uqarvtYgUvoGmdmBWo+0x8brA3wGtMp1qm+KVWxhHn0EWAiwBUEzGsaH5b5By+Ylp9SkBK+UCsq9UbYUjiLo+IuNIFPT7oXoRAutip585AX98E0Rdv6eNRDRJRWsCq0fhj46MIj9rj8klCmnF56ZlxDRJRKGCIjkAznEc7mLJpStONgfjPFTP2P7UURsMf/Gmy2vWBcHKSym6n6FFNolxsD5Rukmq1Zmd0A910ma3pglD51TekO7wIDAQAB", 77, 550},
    {"MEQxCzAJBgNVBAYTAkZJMRowGAYDVQQKDBFUZWxpYSBGaW5sYW5kIE95ajEZMBcGA1UEAwwQVGVsaWEgUm9vdCBDQSB2MjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALLQPwe84nvQa5n44ndp586dpAO8gm2h/oFlH0wnrI4AuhZ76zBqAMCzdGh+sq/H1WKzej9Qyow2RCRj0jbpDIX2Q3bVTKFgcmfiKDOlyzG4OiIjNLh9vVYiQJ3q9HsDrWj8soFPmNB06o3lfc1jw6P23pLCWBnglrvFxKk9pXSW/q/5iaq9lRdU2HhE8Qx3FZLgmEKnpNaqIJLNwaCzlrI6hEKNfdWV5Nbb6WLEWLN5xYzTNTODn3WhUidhOPFZPY5Q4L15POdslv5e2QJltI5c0BE0312/UqeBAMN/mUWZFdUXyApT7GPzmX3MaRKGwhfwAZ6/hLzRUssbkmbOpFPlob/E2wnW5olWK8jjfN7j/4nlNW4o6GwLI1GpJQXrSPjdscr6bAhR77cYbETKJuFzxokGgeWKrLDiKca5JLNrRBH0pUPCTEPlcDaMtjNXepUugqD0XBCzYYP2AgWGLnwtbNwDRm41k9V6lS/eINhbfpSQBGq6WT0EBXWdN6IOLj3rwaRSg/7Qa9RmjtzG6RJOHSpXqhC8fF6CfaamyfItufUXJ63RDolUK5X6wK0dmBR4M0KGCqlztft0DbcbMBnEWg4cJ7faGND/isgFuvGqHKI3t+ZIpEYslOqodmJHixBTB0hXbOKSTbauBcvcwUpej6w9GU7C7WB1K9vBykLVAgMBAAE=", 70, 550},
    {"MFoxCzAJBgNVBAYTAkNOMSUwIwYDVQQKDBxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMSQwIgYDVQQDDBtUcnVzdEFzaWEgR2xvYmFsIFJvb3QgQ0EgRzMwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDAMYJhkuSUGwoqZdC+BqmHO1ES6nBBruL7dOoKjbmzTNyPtxNST1QY4SxzlZHFZjtqz6xjbYdT8PfxObegQ2OwxANdV6nnRM7EoYNl9lA+sX4WuDqKAtCWHwDNBSHvBm3dIZwZQ0WhxeiAysKtQGIXBsaqvPPW5vxQfmZCHzyLpnl5hkA1nyDvP+uLRx+PjsXUjrYsyUQE49RDdT/VP68czH5GX6zfZBCK70bwkPAPLfSIC7Epqq+FqklYqL9joDiR5rPmd2jE+SoZhLsO4fWvieylL1AgdB4SQXMeJNnKziyhWTXAyB1GJ2Faj/lN03J5Zh6fFZAhLf3ti1ZwA0pJPn9pMRJpxx5cynoTi+jm9WAPzJMshH/x/Gr8m0ed262IPfN2dTPXS6TIi/n1Q1hPy8gDVI+lhXgEGvNz8teHHUGf59gXzhqcD0r83ERoVGjiQTz+LISGNzzNPy+i2+f3VANfWdP3kXjHi3dqFuVJhZBFcnAvkV34PmVACxmZySYgWmjBNb9Pp1Hx2BErW+Canig7CjoKH8GB5S7wprlppYiU5msTf9FkPz2ccEblooV7WIQn3MSAPmeamseaMQ4w7OYXQJXZRe0Blqq/DPNL0WP3E1jAuPP6Z92bfW1K/zJMtSU7/xxnD4UiWQWRkUF3gdCFTIcQcf+eQxuulXUtgQIDAQAB", 92, 550},
    {"MFoxCzAJBgNVBAYTAkNOMSUwIwYDVQQKDBxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMSQwIgYDVQQDDBtUcnVzdEFzaWEgR2xvYmFsIFJvb3QgQ0EgRzQwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATxs8045CVD5d4ZCbuBeaIVXxVjAd7Cq92zphtnS4CDr5nLrBfbK5bKfFJV4hrhPVbwLxYI+hW8m7tH5j/uqOFMjPXTNvk4XatwmkcN4oFBButJ+bAp3TPsUKV/eSm4IJg=", 92, 120},
    {"MFgxCzAJBgNVBAYTAkNOMSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMSIwIAYDVQQDExlUcnVzdEFzaWEgVExTIEVDQyBSb290IENBMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEuH+lWz8BPn3wiG2uKZjhm1xTmdv3CP/VauCOy0Sm8MGMvU/UztSIU+heV9dOviw/8xKnAOmC4ypbMnxLmxQpnvgtg7XrjjFJPSZh6R1yi4m2CrucMzUP2uzeqUofyTOx", 90, 120},
    {"MFgxCzAJBgNVBAYTAkNOMSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMSIwIAYDVQQDExlUcnVzdEFzaWEgVExTIFJTQSBSb290IENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAwxa4G2qkRHPl1k70uc9bC8HRmoH1Y7CPQ8G7CFoaeuEHdiYfj2lWvv42YNAMg82U6ufFLVwvBRYCngqoL+VgJlSW+JhAHayunXRnL1T7lGOY4yZKg8aVtglDUM0Nfd5EDmAST1s1vb+ZOG19bNriaHMf+TGlGRBzBSrDMhmF6jSq3UIeMI0/nrUeYdVvvQBycq0SP6qmSXPyhhWVDBBfUWTO/3e4yW2s5dWY8ZkubOPJRPm1QycITfZ+3kR5u7KMHNrTS2wu1sN4t0zVpOTa3IqODv8Pw6ZgJijPNr/6VwrsWz/pMBZ7xNvE1GSgMPvl/R1xkt0pj0FY3gCtOv09fBqosRfwTjR4WCXWhSHreR3Q3KvPYjywx5eL1tCf0/483sXj/DqDcIQhHQnCofy7KCFlU2B6kG2WOFv/8dd6W23TyXBJSrodOtBQ2jIgGeSLP+vVFiY3PPt1nrAHcLgUYHfc9g9cUgq8XW2NUQraxRjIm23csIOzf6NOeEyYJavyfi4gXoIVptbYj6zlzTKGyPnk2oniOz6TxRNq/8f3GPxnte8YLFSrQNNsTe3FyLc4T2TElExPoM2upcYnJXUnbcbanI6wBZnVKBHFIdS4/MeWq0pkWWFrE/xLzJKmD6RRvw5YkNmCgzmmX7P6bpKzzTMOhajxKdOrWa3W297UK4ECAwEAAQ==", 90, 550},
    {"MGExCzAJBgNVBAYTAlROMTcwNQYDVQQKDC5BZ2VuY2UgTmF0aW9uYWxlIGRlIENlcnRpZmljYXRpb24gRWxlY3Ryb25pcXVlMRkwFwYDVQQDDBBUdW5UcnVzdCBSb290IENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAw83T/L0EU90MIDrViC4FS0H1g4J+91mfnp5j6HPa9gapTx+0+QsfOYyaINB+BtTsNNmGvHVbh4jw0tnUowqybBvrSSw+rF3YlAOg7DTlMMQ1ffsmTRtuMFTY9YBFnDmtnMklBE2akD5OQG6Ka80pZ8bMLeB06AVXCkhQ+npD2n7sW5oOYnb+6p0dhXLsEbs16B8nv8Ghx7tIFt1W18xOoOG5rNvVgxkahdGUl9fKo2UL8zj5Aq7d9mfPyT/1iixHGplvBQ390B2CMfwpzABYl5FMgAAcM4WWL8tBwosQhMMJJIkftQ/Z2XdHGJKUYFzHmQM8/veVp31QoYDCqYOtWJZVIduGWdSvxrzdgW4H22Bi/uwQbtpoAfSDG6k+olsj12TG39yifdhLuoLSUfhmvwZG5HkqJjZ5jx9OmR2yjwwOHP/JXcD9kBCmsTfzzTokbrSFkL+AuQyM1ZvWyPFWPxqAiXqp4hsyUSw+8t979l16KRmO5ci9NnGLXUzCHT+tWKLPPXBNplCYJdwj+bhYQQhxv0+4hKCPAFQV/JFtWKeWO+tLlifNa6KhhqwNfFTmZkxmX5C+IZoCRi3kg8KAuc9LPuh/PAHsj17Nf9IoQgGViuKXPRAhffadHMU0oewsDgpSLBJVcCQ9y8IUNUNdJ06+wL2qfJbn/J5hrUTTAJcCAwEAAQ==", 99, 550},
    {"MEcxCzAJBgNVBAYTAkNOMREwDwYDVQQKDAhVbmlUcnVzdDElMCMGA1UEAwwcVUNBIEV4dGVuZGVkIFZhbGlkYXRpb24gUm9vdDCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAKkJBygTArCZ4GSqHkMWenOxkaB1Pqj64zgAeuyJaiAPi8WwmzMDWobGWIbVwYW7T8acQE3Kvu5plritgTCafJIF6wUrmkjQuHY+lsggu9Kw8Y/YrEVG/6pnYLR3fmofPBpSegQ9BzyFDYTQH3YK92oU33LjNHxXTlYBPnnxqik7bPr4j21NyDXfruvcJO55RaeFtgWI3ohdJXyXZGcJ2b9aFQWG8wke7FgyMxHzd2Swdh/kEDUXG/IOsWykKqNz/AkfHjIZUxHn2bMsLnYuoaPefmqICejyB4r4ss0Q5+JzQJO7CNE/4fwLlLMl73ym19Gvn/+WmvWRe5gLd9R+6AfSYrWVOePz8W0PDmWEimNUxYC24J5LfUcmpwEIXdGIntfDMkT6gkoKaFR/OFMDzKQAM2RRWQujgpF6XuwWwvMq5mLaKttZYhAlSiqBC0cHQwZwh9L6kxEpekhN65THcE2vZ9VRsYAgAQG0egimkH9O4O8HQYevaqVei/vPULKaVK/DibpYLfUwmLE2cjl+SQT9KadMeeQFV9uUuRZTjUazHZVhV1Z/r/AWW2FYbzZQEQvYrCuVFhoOHwjNNjRlEGJm1YBfFCBfLQygeApo1izX6W8r0koFk/yeb2tn/4jxTqVpSlI3BerGFo3SxJnRgis7ujV191FRWPPIB93ktAN/AgMBAAE=", 73, 550},
    {"MD0xCzAJBgNVBAYTAkNOMREwDwYDVQQKDAhVbmlUcnVzdDEbMBkGA1UEAwwSVUNBIEdsb2JhbCBHMiBSb290MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAxeYrb3zvJgUno4Ek2m/LAfmZmqkywiKHYUGRO8vDaBsGxUypK8FnFyIdK+35KYmToni9kmugow2ifsqTs6bRjDXVdfkX9s9FxeV67HeToI8jrg4aA3++1NDtLnurRiNb/yzmVHqUwCoV8MmNsHo7JOHXaOIxPAYzRrZUEaalLyJUKlgNAQLx+hVRZ2zA+te2G3/RVogvGjqNO7uCEeBHANBSh6v7hn4PJGtAnTRnvI3HLYZveT6OqTwXS3+wmeOwcWDcC/Vkw85DvG1xudLeJ1uK6NjGruFZfc8oLTW4lVYa8bJYS7cSN8h8s+1LgOGN+jIjtm+3SJUIsUROhYw6AlQgL9+/V087OpAh18EmNVQg7Mc/R+zvWr9LesGtOxdQXGLYD0tK3Cv6brxzks3sx1DoQZbXqX5t2Okdj4q1uViSukqSKwxW/YDrCPBeKW4bHAyvj5OJrdu9o54hyokZ7N+1wxrrFv54NkzWbtA+FxyQF2smuvt6L78RHBgOLXMDj6DlNaBa4kx1HXHhOThTeEDMg5PXCp6dW4+K5OXgSORIskfNTip1KnvyIvbJvgmRlld6iIis7nCs+dwp4wwcOxJORNanTrAmyPPZGpeRaOrvjUYG0lZFWJo8DA+DuAUlwznPO6Q0ibd5Ei9Hxeepl2n8pndntd978XplFeRhVmUCAwEAAQ==", 63, 550},
    {"MIGIMQswCQYDVQQGEwJVUzETMBEGA1UECBMKTmV3IEplcnNleTEUMBIGA1UEBxMLSmVyc2V5IENpdHkxHjAcBgNVBAoTFVRoZSBVU0VSVFJVU1QgTmV0d29yazEuMCwGA1UEAxMlVVNFUlRydXN0IEVDQyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTB2MBAGByqGSM49AgEGBSuBBAAiA2IABBqsVFqp+Wgj53rVJG9TxlrYS6vG1bbR5nNxrt2c1gxh/dugiQO4BRTsV87uXT/iIbPO99SKeeCjg34tl9BhxPGZ3CWRY6t/MKO0cOLHoTOc878uXFOxX7N9Mn+KNON5eQ==", 139, 120},
    {"MIGIMQswCQYDVQQGEwJVUzETMBEGA1UECBMKTmV3IEplcnNleTEUMBIGA1UEBxMLSmVyc2V5IENpdHkxHjAcBgNVBAoTFVRoZSBVU0VSVFJVU1QgTmV0d29yazEuMCwGA1UEAxMlVVNFUlRydXN0IFJTQSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAIASZRc2DsPbCLPQrFcNdu3NJ9NMrVCDYeKqIE0JLWQJ3M6Jn8w9qez2z8Hc8dOx1ns3KBErR9o5xrw6GbRfpr19naNjQrZ28qk7K5H44m/Q7BYgkAk+4uh0yRi0kdRiZNt/owbxiBhqkCI8vP4T8IcUe/bkH47U5FHGEWdGCFHLhhRUP7wz/n5snP8WnRi9UY41pqdmyHJn2yFmsdSbeAPAUDrozPDcvJ5M/q8FljUfV1q3/875PbcstvZU3cjnEjpNrkyKt1yatLcgPcp/IjSufjtoZgFE5wFORlObM2D3lL5TN5BzQ/Myw1Pv26r+dE5px2uMYJPexMcM3+EyrsyTO1F4lWeL7j1W/gzQaQ8bD/MlJmszbfduR/pzQ+V+DqVmsSl8MoRjVYnEDcGTVDAZE6zTfTen6106bDVc20HXEtqpSQvf2ICKCZNijrVmzyWIzYS4sT+kOQ/ZAp7rEkyVfPNrBaleFoPMuGfi6BOdzFuC00yz7Vv/3uVzrCM7LQC/NVV0CUnYSVgaf5I25lGSDvMmfRxNF7zJ7EMm0L9BX0CpRET0medXh55QH1dUqD79dGMvsVBlCeZYQi5DGky08CVHWfoEHpPUJkZKUIGy3r54t/xnFeHJV4QeD2PW6WK61l9VLupcxigIBCU5uA4rqfJMlxwHPw1S9e3vL4IPAgMBAAE=", 139, 550},
    {"MEExCzAJBgNVBAYTAlJPMRQwEgYDVQQKEwtDRVJUU0lHTiBTQTEcMBoGA1UECxMTY2VydFNJR04gUk9PVCBDQSBHMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMDFdRmRfUR0dIf+DjuW3NgBFszuY5HnC2/OOwppGnzC46+CjobXXo9X69MhWf05N0IwvlDqtg+piNguLWkh59E3GE59kdUWX2tbAMI5Qw02hVK5U2UPHULlj88F0+7cDBrZuIt4ImfkabBoxTzkbFpG583H+u/E7Eu9aqSs/cwoUe+StCmrqzWaTOTECMYmzPhpn+Sc8CnTXPnGFiWeI8MgwT0PPzhAsP6CRDiqWhqKa2NYOLQV07YRaXseVO6MGiKscpc/I1mbySKEwQdPzH/iV8oScLumZfNpdWO9lfsbl83kqK/20U6o2YpxJM02PbyWxPFsqa7lzw1uKA2wDrXKUXt4FMMgL3/7FFXhEZn91QqhngLjYl/rNUssuHLoPj1PrCy7Lobio3aP5ZMqz6WryFyNSwb/EkaseMsUBzXgqd+L6a8VTxaJW732jcZZroiFDsGJ6x9nxUWO/203Nit4ZoORUSs9/1F3dmKh7Gc+PoGD4FapUB8fepmrY7+EF3fxDTvf95xhszWYijqy7DwaNz9+j5LP2RIUZNoQAhVB/0/E6xyjyfqZ90bp4RjZsbgyLcsUDFDYg2WD7rlcz8sFWkz6GZdr1l0T08JcVLwyc6B49fFtHsufpaafItzRUZ6CeWRgKRM+o/1Pcmqr4tTluCRVLERLiohEnMqE0yo7AgMBAAE=", 67, 550},
    {"MHExCzAJBgNVBAYTAkhVMREwDwYDVQQHDAhCdWRhcGVzdDEWMBQGA1UECgwNTWljcm9zZWMgTHRkLjEXMBUGA1UEYQwOVkFUSFUtMjM1ODQ0OTcxHjAcBgNVBAMMFWUtU3ppZ25vIFJvb3QgQ0EgMjAxNzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABJbcPYrYsHtvxie+RJCxs1YVe45DJH0ahFnuY2iyxl6H0BVIHqiQrb1TotreOpCmYF9oMrWGQd+HWyx7xf58eto=", 115, 91},
    {"MEcxCzAJBgNVBAYTAkNOMRwwGgYDVQQKExNpVHJ1c0NoaW5hIENvLixMdGQuMRowGAYDVQQDExF2VHJ1cyBFQ0MgUm9vdCBDQTB2MBAGByqGSM49AgEGBSuBBAAiA2IABGVQSq6MeZZKqhwIw6Oizf5ZVkF3/SaUQrsdzQjbc7JbdfPPnE6C9L/4YSaFbNaFW3Jw0v3bYrTfU4u9sURYYkIJx/p/WxDn/kD9wNjDKzLncKa3piBVHXuAXUuPZ0zxEA==", 73, 120},
    {"MEMxCzAJBgNVBAYTAkNOMRwwGgYDVQQKExNpVHJ1c0NoaW5hIENvLixMdGQuMRYwFAYDVQQDEw12VHJ1cyBSb290IENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAvVV8YdO4HQRiBaCubLdwtEHqSwNeED+QWhyLO7Bmi2xIphwiutVAku4zsiNZyY68WNqLntAZ8i9ZxoxjWrqfowuws5pcuhG4EukMu89ubICHKRQDLI0kmshkg7VqrBMsM/Gf3CxhPBo/cFWbrQBSf88Euf42+pzAFq5i/pZMQ35VFL4as9Jtwq92ZpVrKrCUd4VeBA9iHWN192vny1uacOw+ZwXw/gcIgM8o2wXGFCcvhn3wJ97/5n4zSOcLHljRJytTDldKZdf7ooBg/Ey8NVMBapdygq/xHXDonPXvXsJsx0d+WpSFJk07uutM6LAJwmXCnZ0Jm061lwWs9Qag9zYFfvSQsmvEtPlk6ukaCsgNqO0nydTns7mrgiKQJz0q6HyQ77xP/eIKJKfeZSSkXerAdjDTd1D4DQSblDYBc8oGWKbTO9z6BEYTVYrJREe4UTkaLug04nnLWUoKf7ym7x8DZ2pZKyVik9lTGWY8J2Iphk2ka+7/1E661bTijkhaABkJ8QXZzpGx9+vpOU/2bwRDmlX1PgUUvb+zWbTYjjOEo5BSqrMClWD5DExo+e7VFw34cVe1JeQp7mVdr9HuPBcLWkPFpYbqJJ7iBQfcNEISkdY5dK5MQYLb8qZI0bOb8zOq86bAxU719J12Y+YCxiJLwZU/UGQsVOW28Dwpz1cCAwEAAQ==", 69, 550},
    {"MFIxEjAQBgNVBAoTCUFudGhyb3BpYzE8MDoGA1UEAxMzc2FuZGJveC1lZ3Jlc3MtZ2F0ZXdheS1wcm9kdWN0aW9uIEVncmVzcyBHYXRld2F5IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqQHl428M6LZgP0LX+gb0ypFPHgxgiHfrSCtbwAoaPkCwJS6ulQ4UOWN+h34MqB3NCwSy4kDsL/ft+StENoH2u8KX4b27NcoGWYuJlJFX8M5Bsuln7x8SjdsFKz/43yVJvlK70XF8sMUUwMufZL4r9ZKagA+FNXTk+pNar3YmrTlH6aGWu4WSVUShiP9qKiXiEzlTFE+LrR2KOc8PqNhL2BhIh6qjVixggw68AeAvQvX7TjmIKyA52L3bo4n1P45N9FEJvidDUBrX6y6lXimzl4XYfhSNVAB7lqHxuS2Xlyxej3sSRzXv9YntruYMoXhlS1/psPbcM5VR/QI2BlDrMQIDAQAB", 84, 294},
    {"ME8xEjAQBgNVBAoTCUFudGhyb3BpYzE5MDcGA1UEAxMwc2FuZGJveC1lZ3Jlc3MtZ2F0ZXdheS1zdGFnaW5nIEVncmVzcyBHYXRld2F5IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAjWGzaK3z9d9ksFHQUdU/hOC+NwajSpTKpR+k3wVCCPcw3pjVuXvB8IwXWKuqJIHO8+FubMi4j18KZLzig+7qMof9A88zczyCbzWbsLfT5ri8R6KjjV/DvyNf9oVE20+NOD8Ya/Q8poziB1Yaj+e5CU6LilQ8HokNZKRo25SJelNONm4XWdaCYgbFMaFcGY3qPw339yjcB2B1/tNMjbf4Isb1b7N0Hf5w+Wne56JAJgwrMxp8Gg1ADVtElB9xqsVsL9wWQEFzWrbFtFz03El0s5SRoIWkst67oF4ZuLylPe/B+imP665BK+u67Je4y76MEVRr/l/wRHleOJJIH7yWtwIDAQAB", 81, 294},
};

static int base64_value(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static uint32_t base64_decode(const char *text, uint8_t *output)
{
    uint32_t produced = 0, accumulator = 0, bits = 0;
    for (; *text && *text != '='; text++) {
        accumulator = (accumulator << 6) | (uint32_t)base64_value(*text);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output[produced++] = (uint8_t)(accumulator >> bits);
        }
    }
    return produced;
}

static uint8_t builtin_anchor_pool[TLS_BUILTIN_POOL_SIZE];
static uint32_t builtin_anchor_offsets[TLS_BUILTIN_ANCHOR_COUNT];
static uint32_t builtin_anchor_ready;

int tls_trust_store_add_builtin(struct tls_trust_store *store)
{
    if (!builtin_anchor_ready) {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < TLS_BUILTIN_ANCHOR_COUNT; i++) {
            builtin_anchor_offsets[i] = offset;
            offset += base64_decode(tls_builtin_anchors[i].data, builtin_anchor_pool + offset);
        }
        builtin_anchor_ready = 1;
    }
    for (uint32_t i = 0; i < TLS_BUILTIN_ANCHOR_COUNT; i++) {
        const uint8_t *base = builtin_anchor_pool + builtin_anchor_offsets[i];
        if (tls_trust_store_add_anchor(store, base, tls_builtin_anchors[i].subject_length,
                                       base + tls_builtin_anchors[i].subject_length,
                                       tls_builtin_anchors[i].spki_length)) return -1;
    }
    return 0;
}

static struct tls_trust_store default_trust_store;
static uint32_t default_trust_store_ready;

const struct tls_trust_store *tls_default_trust_store(void)
{
    if (!default_trust_store_ready) {
        tls_trust_store_init(&default_trust_store);
        tls_trust_store_add_builtin(&default_trust_store);
        default_trust_store_ready = 1;
    }
    return &default_trust_store;
}

static int certificate_time_valid(const struct x509_certificate *certificate, uint64_t now)
{
    if (now == 0) return 1;
    return now >= certificate->not_before && now <= certificate->not_after;
}

static int certificate_can_sign(const struct x509_certificate *certificate, uint32_t intermediates_below)
{
    if (!certificate->basic_constraints_present || !certificate->is_ca) return 0;
    if (certificate->key_usage_present && !(certificate->key_usage & 0x04)) return 0;
    if (certificate->path_length >= 0 && intermediates_below > (uint32_t)certificate->path_length) return 0;
    return 1;
}

int tls_verify_certificate_chain(const uint8_t *list, uint32_t list_length, const char *server_name,
                                 uint64_t unix_time, const struct tls_trust_store *store, struct x509_key *leaf_key)
{
    struct x509_certificate chain[TLS_MAX_CHAIN];
    uint32_t used[TLS_MAX_CHAIN];
    uint32_t count = 0;
    uint32_t offset = 0;
    uint32_t current = 0;
    uint32_t intermediates = 0;
    const uint8_t *der;
    uint32_t der_length;
    int status;
    if (!store) return TLS_ERROR_CERTIFICATE;
    while ((status = tls_certificate_list_next(list, list_length, &offset, &der, &der_length)) > 0) {
        if (count >= TLS_MAX_CHAIN) return TLS_ERROR_CERTIFICATE;
        if (x509_parse_certificate(der, der_length, &chain[count])) return TLS_ERROR_CERTIFICATE;
        used[count++] = 0;
    }
    if (status < 0 || count == 0) return TLS_ERROR_CERTIFICATE;
    if (chain[0].key.type == X509_KEY_NONE) return TLS_ERROR_CERTIFICATE;
    if (server_name && server_name[0] && !x509_matches_host(&chain[0], server_name)) return TLS_ERROR_CERTIFICATE;
    if (chain[0].ext_usage_present && !chain[0].server_auth) return TLS_ERROR_CERTIFICATE;
    if (chain[0].key_usage_present && !(chain[0].key_usage & 0x80)) return TLS_ERROR_CERTIFICATE;
    if (!certificate_time_valid(&chain[0], unix_time)) return TLS_ERROR_CERTIFICATE;
    used[0] = 1;
    crypto_copy(leaf_key, &chain[0].key, sizeof(*leaf_key));
    for (uint32_t depth = 0; depth <= count; depth++) {
        const struct x509_certificate *certificate = &chain[current];
        int found = -1;
        for (uint32_t i = 0; i < store->count; i++) {
            struct x509_key anchor_key;
            if (bytes_equal_lengths(store->anchors[i].subject, store->anchors[i].subject_length,
                                    certificate->subject, certificate->subject_length) &&
                bytes_equal_lengths(store->anchors[i].spki, store->anchors[i].spki_length,
                                    certificate->spki, certificate->spki_length)) return TLS_OK;
            if (!bytes_equal_lengths(store->anchors[i].subject, store->anchors[i].subject_length,
                                     certificate->issuer, certificate->issuer_length)) continue;
            if (x509_parse_public_key(store->anchors[i].spki, store->anchors[i].spki_length, &anchor_key)) continue;
            if (x509_verify_signature(certificate, &anchor_key)) return TLS_OK;
        }
        for (uint32_t i = 0; i < count; i++) {
            if (used[i]) continue;
            if (!bytes_equal_lengths(chain[i].subject, chain[i].subject_length, certificate->issuer, certificate->issuer_length)) continue;
            if (chain[i].key.type == X509_KEY_NONE || !x509_verify_signature(certificate, &chain[i].key)) continue;
            if (!certificate_can_sign(&chain[i], intermediates) || !certificate_time_valid(&chain[i], unix_time)) continue;
            found = (int)i;
            break;
        }
        if (found < 0) return TLS_ERROR_CERTIFICATE;
        used[found] = 1;
        current = (uint32_t)found;
        intermediates++;
    }
    return TLS_ERROR_CERTIFICATE;
}

#define RECORD_CHANGE_CIPHER_SPEC 20
#define RECORD_ALERT 21
#define RECORD_HANDSHAKE 22
#define RECORD_APPLICATION_DATA 23

#define HANDSHAKE_HELLO_REQUEST 0
#define HANDSHAKE_CLIENT_HELLO 1
#define HANDSHAKE_SERVER_HELLO 2
#define HANDSHAKE_NEW_SESSION_TICKET 4
#define HANDSHAKE_ENCRYPTED_EXTENSIONS 8
#define HANDSHAKE_CERTIFICATE 11
#define HANDSHAKE_SERVER_KEY_EXCHANGE 12
#define HANDSHAKE_CERTIFICATE_REQUEST 13
#define HANDSHAKE_SERVER_HELLO_DONE 14
#define HANDSHAKE_CERTIFICATE_VERIFY 15
#define HANDSHAKE_CLIENT_KEY_EXCHANGE 16
#define HANDSHAKE_FINISHED 20
#define HANDSHAKE_KEY_UPDATE 24
#define HANDSHAKE_MESSAGE_HASH 254

#define EXTENSION_SERVER_NAME 0
#define EXTENSION_SUPPORTED_GROUPS 10
#define EXTENSION_EC_POINT_FORMATS 11
#define EXTENSION_SIGNATURE_ALGORITHMS 13
#define EXTENSION_ALPN 16
#define EXTENSION_EXTENDED_MASTER_SECRET 23
#define EXTENSION_SUPPORTED_VERSIONS 43
#define EXTENSION_COOKIE 44
#define EXTENSION_KEY_SHARE 51
#define EXTENSION_SIGNATURE_ALGORITHMS_CERT 50
#define EXTENSION_RENEGOTIATION_INFO 0xff01

#define GROUP_X25519 0x001d
#define GROUP_SECP256R1 0x0017
#define GROUP_SECP384R1 0x0018

#define ALERT_CLOSE_NOTIFY 0
#define ALERT_UNEXPECTED_MESSAGE 10
#define ALERT_BAD_RECORD_MAC 20
#define ALERT_RECORD_OVERFLOW 22
#define ALERT_HANDSHAKE_FAILURE 40
#define ALERT_BAD_CERTIFICATE 42
#define ALERT_ILLEGAL_PARAMETER 47
#define ALERT_DECODE_ERROR 50
#define ALERT_DECRYPT_ERROR 51
#define ALERT_PROTOCOL_VERSION 70
#define ALERT_INTERNAL_ERROR 80
#define ALERT_USER_CANCELED 90
#define ALERT_NO_RENEGOTIATION 100
#define ALERT_MISSING_EXTENSION 109
#define ALERT_UNSUPPORTED_EXTENSION 110

static const struct tls_cipher_suite tls_suites[] = {
    {0x1301, TLS_AEAD_AES_GCM, 16, HASH_SHA256, 0},
    {0x1302, TLS_AEAD_AES_GCM, 32, HASH_SHA384, 0},
    {0x1303, TLS_AEAD_CHACHA20_POLY1305, 32, HASH_SHA256, 0},
    {0xC02B, TLS_AEAD_AES_GCM, 16, HASH_SHA256, 1},
    {0xC02F, TLS_AEAD_AES_GCM, 16, HASH_SHA256, 1},
    {0xC02C, TLS_AEAD_AES_GCM, 32, HASH_SHA384, 1},
    {0xC030, TLS_AEAD_AES_GCM, 32, HASH_SHA384, 1},
    {0xCCA9, TLS_AEAD_CHACHA20_POLY1305, 32, HASH_SHA256, 1},
    {0xCCA8, TLS_AEAD_CHACHA20_POLY1305, 32, HASH_SHA256, 1}
};

static const uint16_t tls_default_signature_schemes[] = {
    TLS_SCHEME_ECDSA_SECP256R1_SHA256,
    TLS_SCHEME_ECDSA_SECP384R1_SHA384,
    TLS_SCHEME_RSA_PSS_RSAE_SHA256,
    TLS_SCHEME_RSA_PSS_RSAE_SHA384,
    TLS_SCHEME_RSA_PSS_RSAE_SHA512,
    TLS_SCHEME_RSA_PKCS1_SHA256,
    TLS_SCHEME_RSA_PKCS1_SHA384,
    TLS_SCHEME_RSA_PKCS1_SHA512
};

static const uint16_t tls_supported_groups[] = {GROUP_X25519, GROUP_SECP256R1, GROUP_SECP384R1};

static const uint8_t tls_hello_retry_request_random[32] = {
    0xCF,0x21,0xAD,0x74,0xE5,0x9A,0x61,0x11,0xBE,0x1D,0x8C,0x02,0x1E,0x65,0xB8,0x91,
    0xC2,0xA2,0x11,0x16,0x7A,0xBB,0x8C,0x5E,0x07,0x9E,0x09,0xE2,0xC8,0xA8,0x33,0x9C
};

static void put16(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static void put24(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 16);
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)value;
}

static uint32_t get16(const uint8_t *data)
{
    return ((uint32_t)data[0] << 8) | data[1];
}

static uint32_t get24(const uint8_t *data)
{
    return ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
}

const char *tls_error_string(int error)
{
    switch (error) {
    case TLS_OK: return "ok";
    case TLS_ERROR_TRANSPORT: return "transport error";
    case TLS_ERROR_PROTOCOL: return "protocol error";
    case TLS_ERROR_UNSUPPORTED: return "unsupported feature";
    case TLS_ERROR_DECRYPT: return "record decryption failed";
    case TLS_ERROR_CERTIFICATE: return "certificate verification failed";
    case TLS_ERROR_HANDSHAKE: return "handshake failure";
    case TLS_ERROR_ALERT: return "alert received";
    case TLS_ERROR_BUFFER: return "buffer too small";
    case TLS_ERROR_ENTROPY: return "no entropy available";
    case TLS_ERROR_STATE: return "invalid state";
    case TLS_ERROR_FINISHED: return "finished verification failed";
    case TLS_ERROR_TRUNCATED: return "connection closed without close_notify";
    case TLS_ERROR_ARGUMENT: return "invalid argument";
    default: return "unknown error";
    }
}

int tls_client_is_connected(const struct tls_client *client)
{
    return client->state == TLS_STATE_CONNECTED;
}

uint16_t tls_client_cipher_suite(const struct tls_client *client)
{
    return client->suite ? client->suite->identifier : 0;
}

uint16_t tls_client_version(const struct tls_client *client)
{
    if (!client->suite) return 0;
    return client->version12 ? 0x0303 : 0x0304;
}

int tls_client_last_alert(const struct tls_client *client)
{
    return client->last_alert;
}

int tls_certificate_list_next(const uint8_t *list, uint32_t list_length, uint32_t *offset,
                              const uint8_t **certificate, uint32_t *certificate_length)
{
    uint32_t position = *offset;
    uint32_t length;
    uint32_t extensions;
    if (position == list_length) return 0;
    if (position + 3 > list_length) return -1;
    length = get24(list + position);
    if (length == 0 || position + 3 + length + 2 > list_length) return -1;
    extensions = get16(list + position + 3 + length);
    if (position + 3 + length + 2 + extensions > list_length) return -1;
    *certificate = list + position + 3;
    *certificate_length = length;
    *offset = position + 3 + length + 2 + extensions;
    return 1;
}

static int transport_send_all(struct tls_client *client, const uint8_t *data, uint32_t length)
{
    while (length) {
        int sent = client->transport.send(client->transport.context, data, length);
        if (sent <= 0 || (uint32_t)sent > length) return TLS_ERROR_TRANSPORT;
        data += sent;
        length -= (uint32_t)sent;
    }
    return TLS_OK;
}

static int transport_receive_exact(struct tls_client *client, uint8_t *data, uint32_t length)
{
    while (length) {
        int received = client->transport.receive(client->transport.context, data, length);
        if (received == 0) return TLS_ERROR_TRUNCATED;
        if (received < 0 || (uint32_t)received > length) return TLS_ERROR_TRANSPORT;
        data += received;
        length -= (uint32_t)received;
    }
    return TLS_OK;
}

static void cipher_wipe(struct tls_cipher_state *state)
{
    crypto_zero(state, sizeof(*state));
}

static void client_wipe_secrets(struct tls_client *client)
{
    cipher_wipe(&client->read);
    cipher_wipe(&client->write);
    crypto_zero(client->master_secret, sizeof(client->master_secret));
    crypto_zero(client->private_key, sizeof(client->private_key));
    crypto_zero(client->ec_private, sizeof(client->ec_private));
}

static int cipher_set_secret(struct tls_client *client, struct tls_cipher_state *state, const uint8_t *secret)
{
    uint8_t local[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(client->suite->hash);
    enum hash_algorithm hash = client->suite->hash;
    int result = 0;
    crypto_copy(local, secret, digest_size);
    crypto_zero(state, sizeof(*state));
    crypto_copy(state->secret, local, digest_size);
    result |= tls13_expand_label_ex(hash, local, digest_size, "key", 0, 0, state->key, client->suite->key_length);
    result |= tls13_expand_label_ex(hash, local, digest_size, "iv", 0, 0, state->iv, 12);
    if (!result && client->suite->aead == TLS_AEAD_AES_GCM)
        result = aes_gcm_init(&state->gcm, state->key, client->suite->key_length);
    crypto_zero(local, sizeof(local));
    if (result) return TLS_ERROR_HANDSHAKE;
    state->sequence = 0;
    state->active = 1;
    return TLS_OK;
}

static int cipher_set_keys12(struct tls_client *client, struct tls_cipher_state *state,
                             const uint8_t *key, const uint8_t *iv, uint32_t iv_length)
{
    crypto_zero(state, sizeof(*state));
    crypto_copy(state->key, key, client->suite->key_length);
    crypto_copy(state->iv, iv, iv_length);
    if (client->suite->aead == TLS_AEAD_AES_GCM && aes_gcm_init(&state->gcm, state->key, client->suite->key_length))
        return TLS_ERROR_HANDSHAKE;
    state->active = 1;
    return TLS_OK;
}

static void cipher_nonce(const struct tls_cipher_state *state, uint8_t nonce[12])
{
    crypto_copy(nonce, state->iv, 12);
    for (uint32_t i = 0; i < 8; i++) nonce[11 - i] ^= (uint8_t)(state->sequence >> (8 * i));
}

static void record_additional_data(const struct tls_cipher_state *state, uint8_t type, uint32_t length, uint8_t aad[13])
{
    for (uint32_t i = 0; i < 8; i++) aad[i] = (uint8_t)(state->sequence >> (56 - 8 * i));
    aad[8] = type;
    aad[9] = 3;
    aad[10] = 3;
    put16(aad + 11, length);
}

static int send_record(struct tls_client *client, uint8_t type, const uint8_t *data, uint32_t length)
{
    uint8_t *record = client->send_record;
    if (length > TLS_MAX_PLAINTEXT) return TLS_ERROR_ARGUMENT;
    if (!client->write.active) {
        record[0] = type;
        record[1] = 3;
        record[2] = (type == RECORD_HANDSHAKE && length && data[0] == HANDSHAKE_CLIENT_HELLO) ? 1 : 3;
        put16(record + 3, length);
        crypto_copy(record + TLS_RECORD_HEADER_SIZE, data, length);
        return transport_send_all(client, record, TLS_RECORD_HEADER_SIZE + length);
    } else if (client->version12) {
        uint8_t nonce[12];
        uint8_t aad[13];
        uint32_t explicit_size = client->suite->aead == TLS_AEAD_AES_GCM ? 8 : 0;
        uint32_t total = explicit_size + length + 16;
        int result;
        record_additional_data(&client->write, type, length, aad);
        record[0] = type;
        record[1] = 3;
        record[2] = 3;
        put16(record + 3, total);
        crypto_copy(record + TLS_RECORD_HEADER_SIZE + explicit_size, data, length);
        if (explicit_size) {
            crypto_copy(nonce, client->write.iv, 4);
            crypto_copy(nonce + 4, aad, 8);
            crypto_copy(record + TLS_RECORD_HEADER_SIZE, aad, 8);
            aes_gcm_seal(&client->write.gcm, nonce, aad, 13, record + TLS_RECORD_HEADER_SIZE + 8, length,
                         record + TLS_RECORD_HEADER_SIZE + 8 + length);
        } else {
            cipher_nonce(&client->write, nonce);
            chacha20_poly1305_seal(client->write.key, nonce, aad, 13, record + TLS_RECORD_HEADER_SIZE, length,
                                   record + TLS_RECORD_HEADER_SIZE + length);
        }
        client->write.sequence++;
        crypto_zero(nonce, sizeof(nonce));
        result = transport_send_all(client, record, TLS_RECORD_HEADER_SIZE + total);
        crypto_zero(record, TLS_RECORD_HEADER_SIZE + total);
        return result;
    } else {
        uint8_t nonce[12];
        uint32_t inner = length + 1;
        uint32_t total = inner + 16;
        int result;
        record[0] = RECORD_APPLICATION_DATA;
        record[1] = 3;
        record[2] = 3;
        put16(record + 3, total);
        crypto_copy(record + TLS_RECORD_HEADER_SIZE, data, length);
        record[TLS_RECORD_HEADER_SIZE + length] = type;
        cipher_nonce(&client->write, nonce);
        if (client->suite->aead == TLS_AEAD_AES_GCM)
            aes_gcm_seal(&client->write.gcm, nonce, record, TLS_RECORD_HEADER_SIZE,
                         record + TLS_RECORD_HEADER_SIZE, inner, record + TLS_RECORD_HEADER_SIZE + inner);
        else
            chacha20_poly1305_seal(client->write.key, nonce, record, TLS_RECORD_HEADER_SIZE,
                                   record + TLS_RECORD_HEADER_SIZE, inner, record + TLS_RECORD_HEADER_SIZE + inner);
        client->write.sequence++;
        crypto_zero(nonce, sizeof(nonce));
        result = transport_send_all(client, record, TLS_RECORD_HEADER_SIZE + total);
        crypto_zero(record, TLS_RECORD_HEADER_SIZE + total);
        return result;
    }
}

static void send_alert(struct tls_client *client, uint8_t level, uint8_t description)
{
    uint8_t alert[2];
    alert[0] = level;
    alert[1] = description;
    send_record(client, RECORD_ALERT, alert, 2);
}

static int fail(struct tls_client *client, uint8_t alert, int error)
{
    if (alert && client->state != TLS_STATE_FAILED && client->state != TLS_STATE_CLOSED)
        send_alert(client, 2, alert);
    client->state = TLS_STATE_FAILED;
    client_wipe_secrets(client);
    return error;
}

static int read_record(struct tls_client *client, uint8_t *type, uint32_t *length)
{
    uint32_t ignored_change_cipher_specs = 0;
    for (;;) {
        uint8_t record_type;
        uint32_t record_length;
        int result = transport_receive_exact(client, client->record, TLS_RECORD_HEADER_SIZE);
        if (result) return fail(client, 0, result);
        record_type = client->record[0];
        record_length = get16(client->record + 3);
        if (client->record[1] != 3) return fail(client, ALERT_PROTOCOL_VERSION, TLS_ERROR_PROTOCOL);
        if (record_length > TLS_MAX_CIPHERTEXT) return fail(client, ALERT_RECORD_OVERFLOW, TLS_ERROR_PROTOCOL);
        result = transport_receive_exact(client, client->record + TLS_RECORD_HEADER_SIZE, record_length);
        if (result) return fail(client, 0, result);
        client->payload_offset = TLS_RECORD_HEADER_SIZE;
        if (record_type == RECORD_CHANGE_CIPHER_SPEC && !client->version12) {
            if (client->state == TLS_STATE_CONNECTED || record_length != 1 ||
                client->record[TLS_RECORD_HEADER_SIZE] != 1 || ++ignored_change_cipher_specs > 4)
                return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
            continue;
        }
        if (!client->read.active) {
            *type = record_type;
            *length = record_length;
            return TLS_OK;
        } else if (client->version12) {
            uint8_t nonce[12];
            uint8_t aad[13];
            uint32_t explicit_size = client->suite->aead == TLS_AEAD_AES_GCM ? 8 : 0;
            uint32_t overhead = explicit_size + 16;
            uint32_t plain_length;
            int valid;
            if (record_type != RECORD_ALERT && record_type != RECORD_HANDSHAKE && record_type != RECORD_APPLICATION_DATA)
                return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
            if (record_length < overhead) return fail(client, ALERT_BAD_RECORD_MAC, TLS_ERROR_DECRYPT);
            plain_length = record_length - overhead;
            if (plain_length > TLS_MAX_PLAINTEXT) return fail(client, ALERT_RECORD_OVERFLOW, TLS_ERROR_PROTOCOL);
            record_additional_data(&client->read, record_type, plain_length, aad);
            if (explicit_size) {
                crypto_copy(nonce, client->read.iv, 4);
                crypto_copy(nonce + 4, client->record + TLS_RECORD_HEADER_SIZE, 8);
                valid = aes_gcm_open(&client->read.gcm, nonce, aad, 13, client->record + TLS_RECORD_HEADER_SIZE + 8,
                                     plain_length, client->record + TLS_RECORD_HEADER_SIZE + 8 + plain_length);
            } else {
                cipher_nonce(&client->read, nonce);
                valid = chacha20_poly1305_open(client->read.key, nonce, aad, 13, client->record + TLS_RECORD_HEADER_SIZE,
                                               plain_length, client->record + TLS_RECORD_HEADER_SIZE + plain_length);
            }
            crypto_zero(nonce, sizeof(nonce));
            if (!valid) return fail(client, ALERT_BAD_RECORD_MAC, TLS_ERROR_DECRYPT);
            client->read.sequence++;
            client->payload_offset = TLS_RECORD_HEADER_SIZE + explicit_size;
            *type = record_type;
            *length = plain_length;
            return TLS_OK;
        } else {
            uint8_t nonce[12];
            uint32_t ciphertext_length;
            uint32_t index;
            int valid;
            if (record_type != RECORD_APPLICATION_DATA)
                return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
            if (record_length < 17) return fail(client, ALERT_BAD_RECORD_MAC, TLS_ERROR_DECRYPT);
            ciphertext_length = record_length - 16;
            cipher_nonce(&client->read, nonce);
            if (client->suite->aead == TLS_AEAD_AES_GCM)
                valid = aes_gcm_open(&client->read.gcm, nonce, client->record, TLS_RECORD_HEADER_SIZE,
                                     client->record + TLS_RECORD_HEADER_SIZE, ciphertext_length,
                                     client->record + TLS_RECORD_HEADER_SIZE + ciphertext_length);
            else
                valid = chacha20_poly1305_open(client->read.key, nonce, client->record, TLS_RECORD_HEADER_SIZE,
                                               client->record + TLS_RECORD_HEADER_SIZE, ciphertext_length,
                                               client->record + TLS_RECORD_HEADER_SIZE + ciphertext_length);
            crypto_zero(nonce, sizeof(nonce));
            if (!valid) return fail(client, ALERT_BAD_RECORD_MAC, TLS_ERROR_DECRYPT);
            client->read.sequence++;
            index = ciphertext_length;
            while (index > 0 && client->record[TLS_RECORD_HEADER_SIZE + index - 1] == 0) index--;
            if (index == 0) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
            *type = client->record[TLS_RECORD_HEADER_SIZE + index - 1];
            *length = index - 1;
            if (*length > TLS_MAX_PLAINTEXT) return fail(client, ALERT_RECORD_OVERFLOW, TLS_ERROR_PROTOCOL);
            return TLS_OK;
        }
    }
}

static int process_alert(struct tls_client *client, const uint8_t *data, uint32_t length)
{
    if (length != 2) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    client->last_alert = data[1];
    if (data[1] == ALERT_CLOSE_NOTIFY) {
        client->peer_closed = 1;
        return TLS_OK;
    }
    if (data[0] == 1 && data[1] == ALERT_USER_CANCELED) return TLS_OK;
    return fail(client, 0, TLS_ERROR_ALERT);
}

static int handshake_peek(struct tls_client *client, uint8_t *type, uint32_t *length)
{
    uint32_t message_length;
    if (client->handshake_length < 4) return 0;
    message_length = get24(client->handshake + 1);
    if (message_length + 4 > TLS_HANDSHAKE_BUFFER_SIZE) return -1;
    if (client->handshake_length < message_length + 4) return 0;
    *type = client->handshake[0];
    *length = message_length;
    return 1;
}

static void handshake_consume(struct tls_client *client, uint32_t total)
{
    uint32_t remaining = client->handshake_length - total;
    crypto_copy(client->handshake, client->handshake + total, remaining);
    crypto_zero(client->handshake + remaining, total);
    client->handshake_length = remaining;
}

static int handshake_append(struct tls_client *client, const uint8_t *data, uint32_t length)
{
    if (length == 0) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    if (client->handshake_length + length > TLS_HANDSHAKE_BUFFER_SIZE)
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_BUFFER);
    crypto_copy(client->handshake + client->handshake_length, data, length);
    client->handshake_length += length;
    return TLS_OK;
}

static int fetch_handshake(struct tls_client *client, uint8_t *type, uint32_t *length)
{
    for (;;) {
        uint8_t record_type;
        uint32_t record_length;
        int result = handshake_peek(client, type, length);
        if (result < 0) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_BUFFER);
        if (result > 0) return TLS_OK;
        result = read_record(client, &record_type, &record_length);
        if (result) return result;
        if (record_type == RECORD_ALERT) {
            result = process_alert(client, client->record + client->payload_offset, record_length);
            if (result) return result;
            if (client->peer_closed) return fail(client, 0, TLS_ERROR_TRUNCATED);
            continue;
        }
        if (record_type != RECORD_HANDSHAKE) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        result = handshake_append(client, client->record + client->payload_offset, record_length);
        if (result) return result;
    }
}

static void transcript_snapshot(const struct tls_client *client, uint8_t *digest)
{
    hash_snapshot(&client->transcript, digest);
}

static int send_handshake(struct tls_client *client, uint8_t type, const uint8_t *body, uint32_t length, int track)
{
    uint8_t message[4 + 320];
    if (length > 320) return TLS_ERROR_ARGUMENT;
    message[0] = type;
    put24(message + 1, length);
    crypto_copy(message + 4, body, length);
    if (track) hash_update(&client->transcript, message, 4 + length);
    return send_record(client, RECORD_HANDSHAKE, message, 4 + length);
}

static int compute_finished(struct tls_client *client, const uint8_t *base_secret, const uint8_t *transcript_hash, uint8_t *output)
{
    uint8_t finished_key[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(client->suite->hash);
    int result = tls13_expand_label_ex(client->suite->hash, base_secret, digest_size, "finished", 0, 0,
                                       finished_key, digest_size);
    if (!result) result = hmac_compute(client->suite->hash, finished_key, digest_size, transcript_hash, digest_size, output);
    crypto_zero(finished_key, sizeof(finished_key));
    return result;
}

static int scheme_allowed(const struct tls_client *client, uint16_t scheme)
{
    const uint16_t *list = client->config.signature_schemes ? client->config.signature_schemes : tls_default_signature_schemes;
    uint32_t count = client->config.signature_schemes ? client->config.signature_scheme_count :
                     (uint32_t)(sizeof(tls_default_signature_schemes) / sizeof(tls_default_signature_schemes[0]));
    for (uint32_t i = 0; i < count; i++) if (list[i] == scheme) return 1;
    return 0;
}

static int suite_enabled(const struct tls_client *client, const struct tls_cipher_suite *suite)
{
    if (suite->aead == TLS_AEAD_CHACHA20_POLY1305 && (client->config.flags & TLS_FLAG_DISABLE_CHACHA20)) return 0;
    if (suite->aead == TLS_AEAD_AES_GCM && suite->key_length == 32 && (client->config.flags & TLS_FLAG_DISABLE_AES256)) return 0;
    if (suite->tls12 && (client->config.flags & TLS_FLAG_DISABLE_TLS12)) return 0;
    return 1;
}

static const struct tls_cipher_suite *find_suite(const struct tls_client *client, uint32_t identifier)
{
    for (uint32_t i = 0; i < sizeof(tls_suites) / sizeof(tls_suites[0]); i++)
        if (tls_suites[i].identifier == identifier && suite_enabled(client, &tls_suites[i])) return &tls_suites[i];
    return 0;
}

static uint32_t group_share_length(uint32_t group)
{
    if (group == GROUP_X25519) return 32;
    if (group == GROUP_SECP256R1) return 65;
    if (group == GROUP_SECP384R1) return 97;
    return 0;
}

static const struct ec_curve *group_curve(uint32_t group)
{
    return group == GROUP_SECP256R1 ? &ec_p256 : group == GROUP_SECP384R1 ? &ec_p384 : 0;
}

static int build_client_hello(struct tls_client *client, uint32_t group, const uint8_t *share, uint32_t share_length, int retry)
{
    uint8_t *p = client->client_hello;
    uint8_t *extensions_length;
    uint8_t *suites_length;
    uint8_t *extension_start;
    uint32_t name_length = crypto_string_length(client->server_name);
    const uint16_t *schemes = client->config.signature_schemes ? client->config.signature_schemes : tls_default_signature_schemes;
    uint32_t scheme_count = client->config.signature_schemes ? client->config.signature_scheme_count :
                            (uint32_t)(sizeof(tls_default_signature_schemes) / sizeof(tls_default_signature_schemes[0]));
    uint32_t suite_count = 0;
    uint32_t tls12_enabled = !(client->config.flags & TLS_FLAG_DISABLE_TLS12);
    if (scheme_count == 0 || scheme_count > 32) return TLS_ERROR_ARGUMENT;
    if (!retry) {
        if (crypto_random_bytes(client->client_random, 32)) return TLS_ERROR_ENTROPY;
        if (crypto_random_bytes(client->session_id, 32)) return TLS_ERROR_ENTROPY;
    }
    *p++ = HANDSHAKE_CLIENT_HELLO;
    p += 3;
    put16(p, 0x0303); p += 2;
    crypto_copy(p, client->client_random, 32); p += 32;
    *p++ = 32;
    crypto_copy(p, client->session_id, 32); p += 32;
    suites_length = p; p += 2;
    for (uint32_t i = 0; i < sizeof(tls_suites) / sizeof(tls_suites[0]); i++) {
        if (!suite_enabled(client, &tls_suites[i])) continue;
        put16(p, tls_suites[i].identifier); p += 2;
        suite_count++;
    }
    if (!suite_count) return TLS_ERROR_ARGUMENT;
    put16(suites_length, suite_count * 2);
    *p++ = 1;
    *p++ = 0;
    extensions_length = p; p += 2;
    extension_start = p;
    if (name_length) {
        put16(p, EXTENSION_SERVER_NAME); p += 2;
        put16(p, name_length + 5); p += 2;
        put16(p, name_length + 3); p += 2;
        *p++ = 0;
        put16(p, name_length); p += 2;
        crypto_copy(p, client->server_name, name_length); p += name_length;
    }
    put16(p, EXTENSION_SUPPORTED_VERSIONS); p += 2;
    put16(p, tls12_enabled ? 5 : 3); p += 2;
    *p++ = tls12_enabled ? 4 : 2;
    put16(p, 0x0304); p += 2;
    if (tls12_enabled) { put16(p, 0x0303); p += 2; }
    put16(p, EXTENSION_SUPPORTED_GROUPS); p += 2;
    put16(p, 2 + 2 * (sizeof(tls_supported_groups) / sizeof(tls_supported_groups[0]))); p += 2;
    put16(p, 2 * (sizeof(tls_supported_groups) / sizeof(tls_supported_groups[0]))); p += 2;
    for (uint32_t i = 0; i < sizeof(tls_supported_groups) / sizeof(tls_supported_groups[0]); i++) {
        put16(p, tls_supported_groups[i]); p += 2;
    }
    if (tls12_enabled) {
        put16(p, EXTENSION_EC_POINT_FORMATS); p += 2;
        put16(p, 2); p += 2;
        *p++ = 1;
        *p++ = 0;
        put16(p, EXTENSION_EXTENDED_MASTER_SECRET); p += 2;
        put16(p, 0); p += 2;
        put16(p, EXTENSION_RENEGOTIATION_INFO); p += 2;
        put16(p, 1); p += 2;
        *p++ = 0;
    }
    put16(p, EXTENSION_ALPN); p += 2;
    put16(p, 11); p += 2;
    put16(p, 9); p += 2;
    *p++ = 8;
    crypto_copy(p, "http/1.1", 8); p += 8;
    put16(p, EXTENSION_SIGNATURE_ALGORITHMS); p += 2;
    put16(p, scheme_count * 2 + 2); p += 2;
    put16(p, scheme_count * 2); p += 2;
    for (uint32_t i = 0; i < scheme_count; i++) { put16(p, schemes[i]); p += 2; }
    put16(p, EXTENSION_SIGNATURE_ALGORITHMS_CERT); p += 2;
    put16(p, scheme_count * 2 + 2); p += 2;
    put16(p, scheme_count * 2); p += 2;
    for (uint32_t i = 0; i < scheme_count; i++) { put16(p, schemes[i]); p += 2; }
    put16(p, EXTENSION_KEY_SHARE); p += 2;
    put16(p, share_length + 6); p += 2;
    put16(p, share_length + 4); p += 2;
    put16(p, group); p += 2;
    put16(p, share_length); p += 2;
    crypto_copy(p, share, share_length); p += share_length;
    if (client->cookie_length) {
        put16(p, EXTENSION_COOKIE); p += 2;
        put16(p, client->cookie_length + 2); p += 2;
        put16(p, client->cookie_length); p += 2;
        crypto_copy(p, client->cookie, client->cookie_length); p += client->cookie_length;
    }
    put16(extensions_length, (uint32_t)(p - extension_start));
    client->client_hello_length = (uint32_t)(p - client->client_hello);
    if (client->client_hello_length > TLS_CLIENT_HELLO_BUFFER_SIZE) return TLS_ERROR_BUFFER;
    put24(client->client_hello + 1, client->client_hello_length - 4);
    return TLS_OK;
}

static int parse_hello_retry_request(struct tls_client *client, const uint8_t *body, uint32_t length)
{
    uint32_t p = 2 + 32;
    uint32_t session_length;
    uint32_t extensions_length;
    uint32_t seen_version = 0, seen_group = 0, seen_cookie = 0;
    if (length < p + 1) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    if (get16(body) != 0x0303) return fail(client, ALERT_PROTOCOL_VERSION, TLS_ERROR_PROTOCOL);
    session_length = body[p++];
    if (session_length != 32 || p + session_length + 5 > length ||
        !crypto_equal(body + p, client->session_id, 32)) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    p += session_length;
    client->retry_suite = find_suite(client, get16(body + p));
    p += 2;
    if (!client->retry_suite || client->retry_suite->tls12 || body[p++] != 0)
        return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    extensions_length = get16(body + p); p += 2;
    if (p + extensions_length != length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    while (p < length) {
        uint32_t type, size;
        if (p + 4 > length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
        type = get16(body + p);
        size = get16(body + p + 2);
        p += 4;
        if (p + size > length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
        if (type == EXTENSION_SUPPORTED_VERSIONS) {
            if (seen_version || size != 2 || get16(body + p) != 0x0304) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            seen_version = 1;
        } else if (type == EXTENSION_KEY_SHARE) {
            if (seen_group || size != 2) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            client->retry_group = get16(body + p);
            seen_group = 1;
        } else if (type == EXTENSION_COOKIE) {
            uint32_t cookie_length;
            if (seen_cookie || size < 3) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            cookie_length = get16(body + p);
            if (cookie_length + 2 != size || cookie_length == 0 || cookie_length > sizeof(client->cookie))
                return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            crypto_copy(client->cookie, body + p + 2, cookie_length);
            client->cookie_length = cookie_length;
            seen_cookie = 1;
        } else {
            return fail(client, ALERT_UNSUPPORTED_EXTENSION, TLS_ERROR_PROTOCOL);
        }
        p += size;
    }
    if (!seen_version) return fail(client, ALERT_MISSING_EXTENSION, TLS_ERROR_PROTOCOL);
    if (seen_group && (client->retry_group == GROUP_X25519 || !group_curve(client->retry_group)))
        return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_UNSUPPORTED);
    if (!seen_group && !seen_cookie) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    if (!seen_group) client->retry_group = GROUP_X25519;
    return TLS_OK;
}

static int parse_server_hello(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t *is_tls12)
{
    uint32_t p = 2;
    uint32_t session_length;
    const uint8_t *session;
    uint32_t extensions_length;
    uint32_t seen_version = 0;
    uint32_t seen_key_share = 0;
    client->extended_master_secret = 0;
    if (length < 2 + 32 + 1) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    if (get16(body) != 0x0303) return fail(client, ALERT_PROTOCOL_VERSION, TLS_ERROR_PROTOCOL);
    if (crypto_equal(body + p, tls_hello_retry_request_random, 32))
        return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    crypto_copy(client->server_random, body + p, 32);
    p += 32;
    session_length = body[p++];
    if (session_length > 32 || p + session_length + 2 + 1 + 2 > length)
        return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    session = body + p;
    p += session_length;
    client->suite = find_suite(client, get16(body + p));
    p += 2;
    if (!client->suite) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_UNSUPPORTED);
    if (body[p++] != 0) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    extensions_length = get16(body + p); p += 2;
    if (p + extensions_length != length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    while (p < length) {
        uint32_t type, size;
        if (p + 4 > length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
        type = get16(body + p);
        size = get16(body + p + 2);
        p += 4;
        if (p + size > length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
        if (type == EXTENSION_SUPPORTED_VERSIONS) {
            if (seen_version || size != 2 || get16(body + p) != 0x0304)
                return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            seen_version = 1;
        } else if (type == EXTENSION_KEY_SHARE) {
            uint32_t group, share_length;
            if (seen_key_share || size < 4) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            group = get16(body + p);
            share_length = get16(body + p + 2);
            if (group_share_length(group) != share_length || size != 4 + share_length)
                return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            client->server_group = group;
            client->server_share_length = share_length;
            crypto_copy(client->server_share, body + p + 4, share_length);
            seen_key_share = 1;
        } else if (type == EXTENSION_EXTENDED_MASTER_SECRET && size == 0) {
            client->extended_master_secret = 1;
        } else if (type == EXTENSION_EC_POINT_FORMATS || type == EXTENSION_ALPN || type == EXTENSION_SERVER_NAME ||
                   (type == EXTENSION_RENEGOTIATION_INFO && size == 1 && body[p] == 0)) {
        } else {
            return fail(client, ALERT_UNSUPPORTED_EXTENSION, TLS_ERROR_PROTOCOL);
        }
        p += size;
    }
    if (seen_version) {
        if (client->suite->tls12 || client->extended_master_secret) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        if (!seen_key_share) return fail(client, ALERT_MISSING_EXTENSION, TLS_ERROR_PROTOCOL);
        if (session_length != 32 || !crypto_equal(session, client->session_id, 32))
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        if (client->retry_suite && client->suite != client->retry_suite)
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        *is_tls12 = 0;
    } else {
        static const uint8_t downgrade_tls12[8] = {0x44,0x4F,0x57,0x4E,0x47,0x52,0x44,0x01};
        static const uint8_t downgrade_tls11[8] = {0x44,0x4F,0x57,0x4E,0x47,0x52,0x44,0x00};
        if (!client->suite->tls12 || seen_key_share || client->retry_suite)
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        if (crypto_equal(client->server_random + 24, downgrade_tls12, 8) || crypto_equal(client->server_random + 24, downgrade_tls11, 8))
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        *is_tls12 = 1;
    }
    return TLS_OK;
}

static int derive_handshake_keys(struct tls_client *client, const uint8_t *shared, uint32_t shared_length)
{
    enum hash_algorithm hash = client->suite->hash;
    uint32_t digest_size = hash_digest_size(hash);
    uint8_t zeros[HASH_MAX_DIGEST_SIZE];
    uint8_t early[HASH_MAX_DIGEST_SIZE];
    uint8_t empty_hash[HASH_MAX_DIGEST_SIZE];
    uint8_t derived[HASH_MAX_DIGEST_SIZE];
    uint8_t handshake_secret[HASH_MAX_DIGEST_SIZE];
    uint8_t transcript[HASH_MAX_DIGEST_SIZE];
    uint8_t client_secret[HASH_MAX_DIGEST_SIZE];
    uint8_t server_secret[HASH_MAX_DIGEST_SIZE];
    int result = 0;
    crypto_zero(zeros, sizeof(zeros));
    crypto_zero(derived, sizeof(derived));
    result |= hkdf_extract(hash, 0, 0, zeros, digest_size, early);
    result |= hash_digest(hash, 0, 0, empty_hash);
    result |= tls13_derive_secret(hash, early, digest_size, "derived", empty_hash, derived);
    result |= hkdf_extract(hash, derived, digest_size, shared, shared_length, handshake_secret);
    transcript_snapshot(client, transcript);
    result |= tls13_derive_secret(hash, handshake_secret, digest_size, "c hs traffic", transcript, client_secret);
    result |= tls13_derive_secret(hash, handshake_secret, digest_size, "s hs traffic", transcript, server_secret);
    result |= tls13_derive_secret(hash, handshake_secret, digest_size, "derived", empty_hash, derived);
    result |= hkdf_extract(hash, derived, digest_size, zeros, digest_size, client->master_secret);
    if (!result) result = cipher_set_secret(client, &client->write, client_secret);
    if (!result) result = cipher_set_secret(client, &client->read, server_secret);
    crypto_zero(early, sizeof(early));
    crypto_zero(derived, sizeof(derived));
    crypto_zero(handshake_secret, sizeof(handshake_secret));
    crypto_zero(client_secret, sizeof(client_secret));
    crypto_zero(server_secret, sizeof(server_secret));
    return result ? TLS_ERROR_HANDSHAKE : TLS_OK;
}

static int validate_extensions(const uint8_t *data, uint32_t length)
{
    uint32_t p = 0;
    while (p < length) {
        uint32_t size;
        if (p + 4 > length) return 0;
        size = get16(data + p + 2);
        p += 4;
        if (p + size > length) return 0;
        p += size;
    }
    return 1;
}

static int verify_server_signature(struct tls_client *client, uint16_t scheme, const uint8_t *signature,
                                   uint32_t signature_length, const uint8_t *content, uint32_t content_length)
{
    if (client->config.verify) {
        if (!client->config.verify(client->config.verify_context, client->server_name,
                                   client->certificate_list, client->certificate_list_length,
                                   scheme, signature, signature_length, content, content_length))
            return fail(client, ALERT_BAD_CERTIFICATE, TLS_ERROR_CERTIFICATE);
        return TLS_OK;
    }
    if (client->config.flags & TLS_FLAG_ALLOW_INSECURE) return TLS_OK;
    {
        struct x509_key leaf_key;
        const struct tls_trust_store *store = client->config.trust_store ? client->config.trust_store : tls_default_trust_store();
        if (tls_verify_certificate_chain(client->certificate_list, client->certificate_list_length, client->server_name,
                                         client->config.unix_time, store, &leaf_key))
            return fail(client, ALERT_BAD_CERTIFICATE, TLS_ERROR_CERTIFICATE);
        if (!x509_verify_handshake_signature(&leaf_key, scheme, content, content_length, signature, signature_length))
            return fail(client, ALERT_DECRYPT_ERROR, TLS_ERROR_CERTIFICATE);
    }
    return TLS_OK;
}

static int handle_encrypted_extensions(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t total)
{
    if (length < 2 || 2 + get16(body) != length || !validate_extensions(body + 2, length - 2))
        return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    hash_update(&client->transcript, client->handshake, total);
    client->state = TLS_STATE_WAIT_CERTIFICATE_OR_REQUEST;
    return TLS_OK;
}

static int handle_certificate_request(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t total)
{
    uint32_t context_length;
    if (length < 3) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    context_length = body[0];
    if (1 + context_length + 2 > length || 1 + context_length + 2 + get16(body + 1 + context_length) != length ||
        !validate_extensions(body + 1 + context_length + 2, length - 1 - context_length - 2))
        return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    crypto_copy(client->certificate_request_context, body + 1, context_length);
    client->certificate_request_context_length = context_length;
    client->certificate_requested = 1;
    hash_update(&client->transcript, client->handshake, total);
    client->state = TLS_STATE_WAIT_CERTIFICATE;
    return TLS_OK;
}

static int handle_certificate(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t total)
{
    uint32_t context_length;
    uint32_t list_length;
    uint32_t offset = 0;
    const uint8_t *certificate;
    uint32_t certificate_length;
    int entry;
    if (length < 4) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    context_length = body[0];
    if (context_length != 0) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    list_length = get24(body + 1);
    if (list_length + 4 != length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    if (list_length == 0) return fail(client, ALERT_BAD_CERTIFICATE, TLS_ERROR_CERTIFICATE);
    if (list_length > TLS_CERTIFICATE_BUFFER_SIZE) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_BUFFER);
    while ((entry = tls_certificate_list_next(body + 4, list_length, &offset, &certificate, &certificate_length)) > 0) {}
    if (entry < 0) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    crypto_copy(client->certificate_storage, body + 4, list_length);
    client->certificate_list = client->certificate_storage;
    client->certificate_list_length = list_length;
    hash_update(&client->transcript, client->handshake, total);
    client->state = TLS_STATE_WAIT_CERTIFICATE_VERIFY;
    return TLS_OK;
}

static int handle_certificate_verify(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t total)
{
    static const char context_string[] = "TLS 1.3, server CertificateVerify";
    uint8_t signed_content[64 + 33 + 1 + HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(client->suite->hash);
    uint32_t content_length = 0;
    uint32_t scheme;
    uint32_t signature_length;
    int result;
    if (length < 4) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    scheme = get16(body);
    signature_length = get16(body + 2);
    if (4 + signature_length != length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    if (!scheme_allowed(client, (uint16_t)scheme) || scheme == TLS_SCHEME_RSA_PKCS1_SHA256 ||
        scheme == TLS_SCHEME_RSA_PKCS1_SHA384 || scheme == TLS_SCHEME_RSA_PKCS1_SHA512)
        return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    for (uint32_t i = 0; i < 64; i++) signed_content[content_length++] = 0x20;
    crypto_copy(signed_content + content_length, context_string, 33);
    content_length += 33;
    signed_content[content_length++] = 0;
    transcript_snapshot(client, signed_content + content_length);
    content_length += digest_size;
    result = verify_server_signature(client, (uint16_t)scheme, body + 4, signature_length, signed_content, content_length);
    if (result) return result;
    hash_update(&client->transcript, client->handshake, total);
    client->state = TLS_STATE_WAIT_FINISHED;
    return TLS_OK;
}

static int handle_server_finished(struct tls_client *client, const uint8_t *body, uint32_t length, uint32_t total)
{
    enum hash_algorithm hash = client->suite->hash;
    uint32_t digest_size = hash_digest_size(hash);
    uint8_t transcript[HASH_MAX_DIGEST_SIZE];
    uint8_t expected[HASH_MAX_DIGEST_SIZE];
    uint8_t client_secret[HASH_MAX_DIGEST_SIZE];
    uint8_t server_secret[HASH_MAX_DIGEST_SIZE];
    uint8_t verify_data[HASH_MAX_DIGEST_SIZE];
    int result;
    if (length != digest_size) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
    transcript_snapshot(client, transcript);
    if (compute_finished(client, client->read.secret, transcript, expected))
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    if (!crypto_equal(expected, body, digest_size)) return fail(client, ALERT_DECRYPT_ERROR, TLS_ERROR_FINISHED);
    hash_update(&client->transcript, client->handshake, total);
    transcript_snapshot(client, transcript);
    result = tls13_derive_secret(hash, client->master_secret, digest_size, "c ap traffic", transcript, client_secret);
    result |= tls13_derive_secret(hash, client->master_secret, digest_size, "s ap traffic", transcript, server_secret);
    if (result) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    if (client->certificate_requested) {
        uint8_t message[1 + 255 + 3];
        uint32_t size = 0;
        message[size++] = (uint8_t)client->certificate_request_context_length;
        crypto_copy(message + size, client->certificate_request_context, client->certificate_request_context_length);
        size += client->certificate_request_context_length;
        message[size++] = 0; message[size++] = 0; message[size++] = 0;
        result = send_handshake(client, HANDSHAKE_CERTIFICATE, message, size, 1);
        if (result) return fail(client, 0, result);
    }
    transcript_snapshot(client, transcript);
    if (compute_finished(client, client->write.secret, transcript, verify_data))
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    result = send_handshake(client, HANDSHAKE_FINISHED, verify_data, digest_size, 1);
    if (result) return fail(client, 0, result);
    result = cipher_set_secret(client, &client->write, client_secret);
    if (!result) result = cipher_set_secret(client, &client->read, server_secret);
    crypto_zero(client_secret, sizeof(client_secret));
    crypto_zero(server_secret, sizeof(server_secret));
    crypto_zero(expected, sizeof(expected));
    crypto_zero(verify_data, sizeof(verify_data));
    if (result) return fail(client, ALERT_INTERNAL_ERROR, result);
    crypto_zero(client->master_secret, sizeof(client->master_secret));
    client->state = TLS_STATE_CONNECTED;
    return TLS_OK;
}

static int handle_handshake_message(struct tls_client *client, uint8_t type, uint32_t length)
{
    const uint8_t *body = client->handshake + 4;
    uint32_t total = length + 4;
    switch (client->state) {
    case TLS_STATE_WAIT_ENCRYPTED_EXTENSIONS:
        if (type == HANDSHAKE_ENCRYPTED_EXTENSIONS) return handle_encrypted_extensions(client, body, length, total);
        break;
    case TLS_STATE_WAIT_CERTIFICATE_OR_REQUEST:
        if (type == HANDSHAKE_CERTIFICATE_REQUEST) return handle_certificate_request(client, body, length, total);
        if (type == HANDSHAKE_CERTIFICATE) return handle_certificate(client, body, length, total);
        break;
    case TLS_STATE_WAIT_CERTIFICATE:
        if (type == HANDSHAKE_CERTIFICATE) return handle_certificate(client, body, length, total);
        break;
    case TLS_STATE_WAIT_CERTIFICATE_VERIFY:
        if (type == HANDSHAKE_CERTIFICATE_VERIFY) return handle_certificate_verify(client, body, length, total);
        break;
    case TLS_STATE_WAIT_FINISHED:
        if (type == HANDSHAKE_FINISHED) return handle_server_finished(client, body, length, total);
        break;
    default:
        break;
    }
    return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
}

static int tls12_finish(struct tls_client *client, uint32_t group, const uint8_t *server_point, uint32_t point_length)
{
    enum hash_algorithm hash = client->suite->hash;
    uint32_t digest_size = hash_digest_size(hash);
    uint32_t key_length = client->suite->key_length;
    uint32_t iv_length = client->suite->aead == TLS_AEAD_AES_GCM ? 4 : 12;
    uint8_t premaster[48];
    uint32_t premaster_length;
    uint8_t message[1 + 97];
    uint8_t seed[64];
    uint8_t transcript[HASH_MAX_DIGEST_SIZE];
    uint8_t master[48];
    uint8_t block[2 * 32 + 2 * 12];
    uint8_t verify[12];
    uint8_t expected[12];
    uint8_t type;
    uint32_t length;
    int result;
    if (client->certificate_requested) {
        static const uint8_t empty_list[3] = {0, 0, 0};
        result = send_handshake(client, HANDSHAKE_CERTIFICATE, empty_list, 3, 1);
        if (result) return fail(client, 0, result);
    }
    if (group == GROUP_X25519) {
        x25519_public_key(client->private_key, message + 1);
        if (!x25519_shared_secret(client->private_key, server_point, premaster))
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        premaster_length = 32;
        message[0] = 32;
    } else {
        const struct ec_curve *curve = group_curve(group);
        uint8_t private_key[48];
        result = ecdh_generate(curve, private_key, message + 1);
        if (result) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_ENTROPY);
        if (ecdh_shared(curve, private_key, server_point, point_length, premaster)) {
            crypto_zero(private_key, sizeof(private_key));
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        }
        crypto_zero(private_key, sizeof(private_key));
        premaster_length = curve->size;
        message[0] = (uint8_t)(1 + 2 * curve->size);
    }
    crypto_zero(client->private_key, sizeof(client->private_key));
    result = send_handshake(client, HANDSHAKE_CLIENT_KEY_EXCHANGE, message, 1 + message[0], 1);
    if (result) return fail(client, 0, result);
    if (client->extended_master_secret) {
        transcript_snapshot(client, transcript);
        result = tls12_prf(hash, premaster, premaster_length, "extended master secret", transcript, digest_size, master, 48);
    } else {
        crypto_copy(seed, client->client_random, 32);
        crypto_copy(seed + 32, client->server_random, 32);
        result = tls12_prf(hash, premaster, premaster_length, "master secret", seed, 64, master, 48);
    }
    crypto_zero(premaster, sizeof(premaster));
    if (result) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    crypto_copy(seed, client->server_random, 32);
    crypto_copy(seed + 32, client->client_random, 32);
    if (tls12_prf(hash, master, 48, "key expansion", seed, 64, block, 2 * key_length + 2 * iv_length))
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    {
        uint8_t change_cipher_spec = 1;
        result = send_record(client, RECORD_CHANGE_CIPHER_SPEC, &change_cipher_spec, 1);
        if (result) return fail(client, 0, result);
    }
    result = cipher_set_keys12(client, &client->write, block, block + 2 * key_length, iv_length);
    if (result) return fail(client, ALERT_INTERNAL_ERROR, result);
    transcript_snapshot(client, transcript);
    if (tls12_prf(hash, master, 48, "client finished", transcript, digest_size, verify, 12))
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    result = send_handshake(client, HANDSHAKE_FINISHED, verify, 12, 1);
    if (result) return fail(client, 0, result);
    if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    for (;;) {
        result = read_record(client, &type, &length);
        if (result) return result;
        if (type == RECORD_ALERT) {
            result = process_alert(client, client->record + client->payload_offset, length);
            if (result) return result;
            return fail(client, 0, TLS_ERROR_TRUNCATED);
        }
        if (type != RECORD_CHANGE_CIPHER_SPEC || length != 1 || client->record[client->payload_offset] != 1)
            return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        break;
    }
    result = cipher_set_keys12(client, &client->read, block + key_length, block + 2 * key_length + iv_length, iv_length);
    crypto_zero(block, sizeof(block));
    if (result) return fail(client, ALERT_INTERNAL_ERROR, result);
    result = fetch_handshake(client, &type, &length);
    if (result) return result;
    if (type != HANDSHAKE_FINISHED || length != 12) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    transcript_snapshot(client, transcript);
    if (tls12_prf(hash, master, 48, "server finished", transcript, digest_size, expected, 12))
        return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
    if (!crypto_equal(expected, client->handshake + 4, 12)) return fail(client, ALERT_DECRYPT_ERROR, TLS_ERROR_FINISHED);
    handshake_consume(client, 16);
    crypto_zero(master, sizeof(master));
    if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    client->state = TLS_STATE_CONNECTED;
    return TLS_OK;
}

static int tls12_handshake(struct tls_client *client)
{
    uint8_t type;
    uint32_t length;
    uint32_t got_certificate = 0, got_key_exchange = 0, got_done = 0;
    uint32_t group = 0;
    uint32_t point_length = 0;
    uint8_t server_point[97];
    int result;
    client->state = TLS_STATE_WAIT_CERTIFICATE;
    while (!got_done) {
        const uint8_t *body;
        result = fetch_handshake(client, &type, &length);
        if (result) return result;
        body = client->handshake + 4;
        if (type == HANDSHAKE_CERTIFICATE && !got_certificate && !got_key_exchange) {
            uint32_t list_length, offset = 0, out = 0;
            if (length < 3) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            list_length = get24(body);
            if (list_length + 3 != length || list_length == 0) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            while (offset < list_length) {
                uint32_t size;
                if (offset + 3 > list_length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
                size = get24(body + 3 + offset);
                if (size == 0 || offset + 3 + size > list_length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
                if (out + 3 + size + 2 > TLS_CERTIFICATE_BUFFER_SIZE) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_BUFFER);
                put24(client->certificate_storage + out, size);
                crypto_copy(client->certificate_storage + out + 3, body + 3 + offset + 3, size);
                out += 3 + size;
                client->certificate_storage[out++] = 0;
                client->certificate_storage[out++] = 0;
                offset += 3 + size;
            }
            client->certificate_list = client->certificate_storage;
            client->certificate_list_length = out;
            got_certificate = 1;
        } else if (type == HANDSHAKE_SERVER_KEY_EXCHANGE && got_certificate && !got_key_exchange) {
            uint8_t content[64 + 4 + 97];
            uint32_t scheme, signature_length;
            if (length < 4 || body[0] != 3) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_UNSUPPORTED);
            group = get16(body + 1);
            point_length = body[3];
            if (!group_share_length(group) || group_share_length(group) != point_length)
                return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_UNSUPPORTED);
            if (4 + point_length + 4 > length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            scheme = get16(body + 4 + point_length);
            signature_length = get16(body + 6 + point_length);
            if (8 + point_length + signature_length != length) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            if (!scheme_allowed(client, (uint16_t)scheme)) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            crypto_copy(content, client->client_random, 32);
            crypto_copy(content + 32, client->server_random, 32);
            crypto_copy(content + 64, body, 4 + point_length);
            result = verify_server_signature(client, (uint16_t)scheme, body + 8 + point_length, signature_length,
                                             content, 64 + 4 + point_length);
            if (result) return result;
            crypto_copy(server_point, body + 4, point_length);
            got_key_exchange = 1;
        } else if (type == HANDSHAKE_CERTIFICATE_REQUEST && got_key_exchange && !client->certificate_requested) {
            client->certificate_requested = 1;
        } else if (type == HANDSHAKE_SERVER_HELLO_DONE && got_key_exchange) {
            if (length != 0) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            got_done = 1;
        } else {
            return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        }
        hash_update(&client->transcript, client->handshake, length + 4);
        handshake_consume(client, length + 4);
    }
    if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    return tls12_finish(client, group, server_point, point_length);
}

int tls_client_connect(struct tls_client *client, const struct tls_transport *transport,
                       const struct tls_config *config)
{
    uint8_t public_key[32];
    uint8_t shared[48];
    uint8_t type;
    uint32_t length;
    uint32_t name_length;
    uint32_t is_tls12 = 0;
    uint32_t shared_length;
    uint32_t retried = 0;
    int result;
    if (!client || !transport || !transport->send || !transport->receive || !config) return TLS_ERROR_ARGUMENT;
    name_length = config->server_name ? crypto_string_length(config->server_name) : 0;
    if (name_length > TLS_SERVER_NAME_MAX) return TLS_ERROR_ARGUMENT;
    crypto_zero(client, sizeof(*client));
    crypto_copy(&client->transport, transport, sizeof(client->transport));
    crypto_copy(&client->config, config, sizeof(client->config));
    crypto_copy(client->server_name, config->server_name, name_length);
    client->config.server_name = client->server_name;
    if (crypto_random_bytes(client->private_key, 32)) return fail(client, 0, TLS_ERROR_ENTROPY);
    x25519_public_key(client->private_key, public_key);
    result = build_client_hello(client, GROUP_X25519, public_key, 32, 0);
    if (result) return fail(client, 0, result);
    client->state = TLS_STATE_WAIT_SERVER_HELLO;
    result = send_record(client, RECORD_HANDSHAKE, client->client_hello, client->client_hello_length);
    if (result) return fail(client, 0, result);
    result = fetch_handshake(client, &type, &length);
    if (result) return result;
    if (type != HANDSHAKE_SERVER_HELLO) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    if (length >= 34 && crypto_equal(client->handshake + 4 + 2, tls_hello_retry_request_random, 32)) {
        uint8_t first_hash[HASH_MAX_DIGEST_SIZE];
        uint8_t header[4];
        uint8_t share[97];
        const struct ec_curve *curve;
        uint32_t digest_size;
        result = parse_hello_retry_request(client, client->handshake + 4, length);
        if (result) return result;
        digest_size = hash_digest_size(client->retry_suite->hash);
        hash_init(&client->transcript, client->retry_suite->hash);
        hash_digest(client->retry_suite->hash, client->client_hello, client->client_hello_length, first_hash);
        header[0] = HANDSHAKE_MESSAGE_HASH; header[1] = 0; header[2] = 0; header[3] = (uint8_t)digest_size;
        hash_update(&client->transcript, header, 4);
        hash_update(&client->transcript, first_hash, digest_size);
        hash_update(&client->transcript, client->handshake, length + 4);
        handshake_consume(client, length + 4);
        if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        {
            uint8_t change_cipher_spec = 1;
            result = send_record(client, RECORD_CHANGE_CIPHER_SPEC, &change_cipher_spec, 1);
            if (result) return fail(client, 0, result);
        }
        curve = group_curve(client->retry_group);
        if (!curve) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_UNSUPPORTED);
        if (ecdh_generate(curve, client->ec_private, share)) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_ENTROPY);
        result = build_client_hello(client, client->retry_group, share, 1 + 2 * curve->size, 1);
        if (result) return fail(client, 0, result);
        result = send_record(client, RECORD_HANDSHAKE, client->client_hello, client->client_hello_length);
        if (result) return fail(client, 0, result);
        hash_update(&client->transcript, client->client_hello, client->client_hello_length);
        retried = 1;
        result = fetch_handshake(client, &type, &length);
        if (result) return result;
        if (type != HANDSHAKE_SERVER_HELLO) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    }
    result = parse_server_hello(client, client->handshake + 4, length, &is_tls12);
    if (result) return result;
    client->version12 = is_tls12;
    if (!retried) {
        if (hash_init(&client->transcript, client->suite->hash)) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
        hash_update(&client->transcript, client->client_hello, client->client_hello_length);
    }
    hash_update(&client->transcript, client->handshake, length + 4);
    handshake_consume(client, length + 4);
    if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    if (is_tls12) return tls12_handshake(client);
    if (client->server_group != (retried ? client->retry_group : GROUP_X25519))
        return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
    if (client->server_group == GROUP_X25519) {
        if (!x25519_shared_secret(client->private_key, client->server_share, shared))
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        shared_length = 32;
    } else {
        const struct ec_curve *curve = group_curve(client->server_group);
        if (ecdh_shared(curve, client->ec_private, client->server_share, client->server_share_length, shared))
            return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
        shared_length = curve->size;
    }
    crypto_zero(client->private_key, sizeof(client->private_key));
    crypto_zero(client->ec_private, sizeof(client->ec_private));
    if (!retried) {
        uint8_t change_cipher_spec = 1;
        result = send_record(client, RECORD_CHANGE_CIPHER_SPEC, &change_cipher_spec, 1);
        if (result) return fail(client, 0, result);
    }
    result = derive_handshake_keys(client, shared, shared_length);
    crypto_zero(shared, sizeof(shared));
    if (result) return fail(client, ALERT_INTERNAL_ERROR, result);
    client->state = TLS_STATE_WAIT_ENCRYPTED_EXTENSIONS;
    while (client->state != TLS_STATE_CONNECTED) {
        result = fetch_handshake(client, &type, &length);
        if (result) return result;
        result = handle_handshake_message(client, type, length);
        if (result) return result;
        handshake_consume(client, length + 4);
    }
    if (client->handshake_length) return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
    return TLS_OK;
}

static int update_traffic_secret(struct tls_client *client, struct tls_cipher_state *state)
{
    uint8_t next[HASH_MAX_DIGEST_SIZE];
    uint32_t digest_size = hash_digest_size(client->suite->hash);
    int result = tls13_expand_label_ex(client->suite->hash, state->secret, digest_size, "traffic upd", 0, 0, next, digest_size);
    if (!result) result = cipher_set_secret(client, state, next);
    crypto_zero(next, sizeof(next));
    return result;
}

static int process_post_handshake(struct tls_client *client)
{
    for (;;) {
        uint8_t type;
        uint32_t length;
        int result = handshake_peek(client, &type, &length);
        if (result < 0) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_BUFFER);
        if (result == 0) return TLS_OK;
        if (type == HANDSHAKE_NEW_SESSION_TICKET) {
            if (length < 13) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
        } else if (type == HANDSHAKE_KEY_UPDATE) {
            uint8_t request;
            if (length != 1) return fail(client, ALERT_DECODE_ERROR, TLS_ERROR_PROTOCOL);
            request = client->handshake[4];
            if (request > 1) return fail(client, ALERT_ILLEGAL_PARAMETER, TLS_ERROR_PROTOCOL);
            if (update_traffic_secret(client, &client->read)) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
            if (request == 1) {
                uint8_t update = 0;
                result = send_handshake(client, HANDSHAKE_KEY_UPDATE, &update, 1, 0);
                if (result) return fail(client, 0, result);
                if (update_traffic_secret(client, &client->write)) return fail(client, ALERT_INTERNAL_ERROR, TLS_ERROR_HANDSHAKE);
            }
        } else {
            return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        }
        handshake_consume(client, length + 4);
    }
}

int tls_client_send(struct tls_client *client, const uint8_t *data, uint32_t length)
{
    uint32_t remaining = length;
    if (!client || (!data && length)) return TLS_ERROR_ARGUMENT;
    if (client->state != TLS_STATE_CONNECTED) return TLS_ERROR_STATE;
    if (length > 0x7fffffffu) return TLS_ERROR_ARGUMENT;
    while (remaining) {
        uint32_t chunk = remaining < TLS_MAX_PLAINTEXT ? remaining : TLS_MAX_PLAINTEXT;
        int result = send_record(client, RECORD_APPLICATION_DATA, data, chunk);
        if (result) return fail(client, 0, result);
        data += chunk;
        remaining -= chunk;
    }
    return (int)length;
}

int tls_client_receive(struct tls_client *client, uint8_t *buffer, uint32_t capacity)
{
    if (!client || !buffer || capacity == 0) return TLS_ERROR_ARGUMENT;
    if (capacity > 0x7fffffffu) capacity = 0x7fffffffu;
    for (;;) {
        uint8_t type;
        uint32_t length;
        int result;
        if (client->plain_length) {
            uint32_t take = client->plain_length < capacity ? client->plain_length : capacity;
            crypto_copy(buffer, client->record + client->plain_offset, take);
            client->plain_offset += take;
            client->plain_length -= take;
            return (int)take;
        }
        if (client->peer_closed) return 0;
        if (client->state != TLS_STATE_CONNECTED) return TLS_ERROR_STATE;
        result = read_record(client, &type, &length);
        if (result) return result;
        if (type == RECORD_APPLICATION_DATA) {
            client->plain_offset = client->payload_offset;
            client->plain_length = length;
        } else if (type == RECORD_HANDSHAKE) {
            if (client->version12) {
                if (length >= 4 && client->record[client->payload_offset] == HANDSHAKE_HELLO_REQUEST) {
                    send_alert(client, 1, ALERT_NO_RENEGOTIATION);
                    continue;
                }
                return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
            }
            result = handshake_append(client, client->record + client->payload_offset, length);
            if (result) return result;
            result = process_post_handshake(client);
            if (result) return result;
        } else if (type == RECORD_ALERT) {
            result = process_alert(client, client->record + client->payload_offset, length);
            if (result) return result;
            if (client->peer_closed) {
                client->state = TLS_STATE_CLOSED;
                return 0;
            }
        } else {
            return fail(client, ALERT_UNEXPECTED_MESSAGE, TLS_ERROR_PROTOCOL);
        }
    }
}

int tls_client_close(struct tls_client *client)
{
    if (!client) return TLS_ERROR_ARGUMENT;
    if (client->state == TLS_STATE_CONNECTED) send_alert(client, 1, ALERT_CLOSE_NOTIFY);
    client->state = TLS_STATE_CLOSED;
    client_wipe_secrets(client);
    return TLS_OK;
}

struct text_writer {
    uint8_t *buffer;
    uint32_t capacity;
    uint32_t length;
    int overflow;
};

static void writer_bytes(struct text_writer *writer, const uint8_t *data, uint32_t length)
{
    if (writer->overflow || writer->length + length > writer->capacity) {
        writer->overflow = 1;
        return;
    }
    crypto_copy(writer->buffer + writer->length, data, length);
    writer->length += length;
}

static void writer_text(struct text_writer *writer, const char *text)
{
    writer_bytes(writer, (const uint8_t *)text, crypto_string_length(text));
}

static void writer_number(struct text_writer *writer, uint32_t value)
{
    char digits[11];
    uint32_t count = 0;
    do {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
    } while (value);
    while (count) writer_bytes(writer, (const uint8_t *)&digits[--count], 1);
}

static int contains_line_break(const char *text)
{
    for (; *text; text++) if (*text == '\r' || *text == '\n') return 1;
    return 0;
}

static char lower_case(char value)
{
    return (value >= 'A' && value <= 'Z') ? (char)(value + 32) : value;
}

static int starts_with_ignore_case(const char *text, const char *prefix)
{
    while (*prefix) {
        if (lower_case(*text) != lower_case(*prefix)) return 0;
        text++;
        prefix++;
    }
    return 1;
}

static int contains_token_ignore_case(const char *text, const char *token)
{
    uint32_t token_length = crypto_string_length(token);
    for (; *text; text++) if (starts_with_ignore_case(text, token)) return token_length > 0;
    return 0;
}

static const char *skip_spaces(const char *text)
{
    while (*text == ' ' || *text == '\t') text++;
    return text;
}

static void copy_limited(char *destination, uint32_t capacity, const char *source)
{
    uint32_t length = crypto_string_length(source);
    while (length && (source[length - 1] == ' ' || source[length - 1] == '\t')) length--;
    if (length >= capacity) length = capacity - 1;
    crypto_copy(destination, source, length);
    destination[length] = 0;
}

const char *https_error_string(int error)
{
    switch (error) {
    case HTTPS_ERROR_ARGUMENT: return "invalid argument";
    case HTTPS_ERROR_MALFORMED: return "malformed http response";
    case HTTPS_ERROR_HEADER_TOO_LARGE: return "http header too large";
    case HTTPS_ERROR_STATE: return "invalid connection state";
    case HTTPS_ERROR_URL: return "invalid url";
    default: return tls_error_string(error);
    }
}

int https_parse_url(const char *url, struct https_url *result)
{
    static const char scheme[] = "https://";
    uint32_t host_length = 0;
    uint32_t path_length = 0;
    uint32_t port = 443;
    if (!url || !result) return HTTPS_ERROR_ARGUMENT;
    if (!starts_with_ignore_case(url, scheme)) return HTTPS_ERROR_URL;
    url += sizeof(scheme) - 1;
    while (*url && *url != ':' && *url != '/' && *url != '?' && *url != '#') {
        if (host_length >= HTTPS_HOST_MAX || *url == '@' || *url == ' ' || *url == '\r' || *url == '\n')
            return HTTPS_ERROR_URL;
        result->host[host_length++] = *url++;
    }
    if (host_length == 0) return HTTPS_ERROR_URL;
    result->host[host_length] = 0;
    if (*url == ':') {
        uint32_t digits = 0;
        port = 0;
        url++;
        while (*url >= '0' && *url <= '9') {
            port = port * 10 + (uint32_t)(*url - '0');
            if (port > 65535) return HTTPS_ERROR_URL;
            digits++;
            url++;
        }
        if (digits == 0 || port == 0) return HTTPS_ERROR_URL;
    }
    result->port = (uint16_t)port;
    if (*url == '?') result->path[path_length++] = '/';
    else if (*url == 0 || *url == '#') result->path[path_length++] = '/';
    while (*url && *url != '#') {
        if (path_length + 1 >= HTTPS_PATH_MAX || *url == ' ' || *url == '\r' || *url == '\n') return HTTPS_ERROR_URL;
        result->path[path_length++] = *url++;
    }
    result->path[path_length] = 0;
    return TLS_OK;
}

int https_connect(struct https_connection *connection, const struct tls_transport *transport,
                  const struct tls_config *config)
{
    if (!connection) return HTTPS_ERROR_ARGUMENT;
    connection->io_offset = 0;
    connection->io_length = 0;
    connection->end_of_stream = 0;
    return tls_client_connect(&connection->tls, transport, config);
}

static int stream_fill(struct https_connection *connection)
{
    int received;
    if (connection->io_offset < connection->io_length) return 1;
    if (connection->end_of_stream) return 0;
    received = tls_client_receive(&connection->tls, connection->io, HTTPS_IO_BUFFER_SIZE);
    if (received == 0 || received == TLS_ERROR_TRUNCATED) {
        connection->end_of_stream = 1;
        return received == 0 ? 0 : TLS_ERROR_TRUNCATED;
    }
    if (received < 0) return received;
    connection->io_offset = 0;
    connection->io_length = (uint32_t)received;
    return 1;
}

static int stream_read_line(struct https_connection *connection, char *line, uint32_t capacity, int discard_excess)
{
    uint32_t length = 0;
    for (;;) {
        int status = stream_fill(connection);
        char value;
        if (status <= 0) return status == 0 ? HTTPS_ERROR_MALFORMED : status;
        value = (char)connection->io[connection->io_offset++];
        if (value == '\n') {
            if (length && line[length - 1] == '\r') length--;
            line[length] = 0;
            return (int)length;
        }
        if (length + 1 < capacity) line[length++] = value;
        else if (!discard_excess) return HTTPS_ERROR_HEADER_TOO_LARGE;
    }
}

static void sink_body(const struct https_request_params *params, struct https_response *response,
                      const uint8_t *data, uint32_t length)
{
    uint32_t room = params->response_capacity > response->body_length ? params->response_capacity - response->body_length : 0;
    uint32_t take = length < room ? length : room;
    if (take && params->response_buffer) crypto_copy(params->response_buffer + response->body_length, data, take);
    response->body_length += take;
    response->body_total += length;
    if (take < length) response->truncated = 1;
}

static int read_counted_body(struct https_connection *connection, const struct https_request_params *params,
                             struct https_response *response, uint32_t count)
{
    while (count) {
        uint32_t available;
        uint32_t take;
        int status = stream_fill(connection);
        if (status <= 0) return status == 0 ? HTTPS_ERROR_MALFORMED : status;
        available = connection->io_length - connection->io_offset;
        take = available < count ? available : count;
        sink_body(params, response, connection->io + connection->io_offset, take);
        connection->io_offset += take;
        count -= take;
    }
    return TLS_OK;
}

static int hex_value(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int read_chunked_body(struct https_connection *connection, const struct https_request_params *params,
                             struct https_response *response)
{
    char line[128];
    for (;;) {
        uint32_t size = 0;
        uint32_t digits = 0;
        int status = stream_read_line(connection, line, sizeof(line), 0);
        if (status < 0) return status;
        for (const char *p = line; hex_value(*p) >= 0; p++) {
            if (size > 0x0FFFFFFFu) return HTTPS_ERROR_MALFORMED;
            size = size * 16 + (uint32_t)hex_value(*p);
            digits++;
        }
        if (digits == 0) return HTTPS_ERROR_MALFORMED;
        if (size == 0) break;
        status = read_counted_body(connection, params, response, size);
        if (status) return status;
        status = stream_read_line(connection, line, sizeof(line), 0);
        if (status < 0) return status;
        if (status != 0) return HTTPS_ERROR_MALFORMED;
    }
    for (;;) {
        int status = stream_read_line(connection, line, sizeof(line), 1);
        if (status < 0) return status;
        if (status == 0) return TLS_OK;
    }
}

static int read_until_close(struct https_connection *connection, const struct https_request_params *params,
                            struct https_response *response)
{
    for (;;) {
        uint32_t available;
        int status = stream_fill(connection);
        if (status == 0 || status == TLS_ERROR_TRUNCATED) return TLS_OK;
        if (status < 0) return status;
        available = connection->io_length - connection->io_offset;
        sink_body(params, response, connection->io + connection->io_offset, available);
        connection->io_offset += available;
    }
}

static int parse_status_line(const char *line, int *status)
{
    uint32_t value = 0;
    if (!starts_with_ignore_case(line, "HTTP/1.")) return 0;
    line += 7;
    if (*line != '0' && *line != '1') return 0;
    line++;
    if (*line != ' ') return 0;
    line++;
    for (uint32_t i = 0; i < 3; i++) {
        if (line[i] < '0' || line[i] > '9') return 0;
        value = value * 10 + (uint32_t)(line[i] - '0');
    }
    if (line[3] != 0 && line[3] != ' ') return 0;
    *status = (int)value;
    return 1;
}

static int read_head(struct https_connection *connection, struct https_response *response)
{
    char line[2048];
    for (;;) {
        int status = stream_read_line(connection, line, sizeof(line), 0);
        int code = 0;
        uint32_t header_count = 0;
        if (status < 0) return status;
        if (!parse_status_line(line, &code)) return HTTPS_ERROR_MALFORMED;
        response->status_code = code;
        response->content_length = HTTPS_UNKNOWN_LENGTH;
        response->chunked = 0;
        response->connection_close = 0;
        response->location[0] = 0;
        response->content_type[0] = 0;
        for (;;) {
            const char *value;
            uint32_t colon = 0;
            status = stream_read_line(connection, line, sizeof(line), 1);
            if (status < 0) return status;
            if (status == 0) break;
            if (++header_count > 1024) return HTTPS_ERROR_HEADER_TOO_LARGE;
            while (line[colon] && line[colon] != ':') colon++;
            if (!line[colon]) continue;
            line[colon] = 0;
            value = skip_spaces(line + colon + 1);
            if (starts_with_ignore_case(line, "content-length") && crypto_string_length(line) == 14) {
                uint32_t length = 0;
                uint32_t digits = 0;
                for (; *value >= '0' && *value <= '9'; value++) {
                    if (length > 0x0FFFFFFFu) return HTTPS_ERROR_MALFORMED;
                    length = length * 10 + (uint32_t)(*value - '0');
                    digits++;
                }
                if (!digits) return HTTPS_ERROR_MALFORMED;
                response->content_length = length;
            } else if (starts_with_ignore_case(line, "transfer-encoding") && crypto_string_length(line) == 17) {
                if (contains_token_ignore_case(value, "chunked")) response->chunked = 1;
            } else if (starts_with_ignore_case(line, "connection") && crypto_string_length(line) == 10) {
                if (contains_token_ignore_case(value, "close")) response->connection_close = 1;
            } else if (starts_with_ignore_case(line, "location") && crypto_string_length(line) == 8) {
                copy_limited(response->location, sizeof(response->location), value);
            } else if (starts_with_ignore_case(line, "content-type") && crypto_string_length(line) == 12) {
                copy_limited(response->content_type, sizeof(response->content_type), value);
            }
        }
        if (code >= 100 && code < 200 && code != 101) continue;
        return TLS_OK;
    }
}

int https_request(struct https_connection *connection, const struct https_request_params *params,
                  struct https_response *response)
{
    struct text_writer writer;
    int is_head;
    int result;
    if (!connection || !params || !response || !params->method || !params->host || !params->path)
        return HTTPS_ERROR_ARGUMENT;
    if (!tls_client_is_connected(&connection->tls)) return HTTPS_ERROR_STATE;
    if (params->body_length && !params->body) return HTTPS_ERROR_ARGUMENT;
    if (params->response_capacity && !params->response_buffer) return HTTPS_ERROR_ARGUMENT;
    if (contains_line_break(params->method) || contains_line_break(params->host) || contains_line_break(params->path) ||
        (params->user_agent && contains_line_break(params->user_agent)) || params->path[0] != '/' || !params->method[0])
        return HTTPS_ERROR_ARGUMENT;
    if (connection->io_offset < connection->io_length) return HTTPS_ERROR_STATE;
    crypto_zero(response, sizeof(*response));
    connection->io_offset = 0;
    connection->io_length = 0;
    writer.buffer = connection->io;
    writer.capacity = HTTPS_IO_BUFFER_SIZE;
    writer.length = 0;
    writer.overflow = 0;
    writer_text(&writer, params->method);
    writer_text(&writer, " ");
    writer_text(&writer, params->path);
    writer_text(&writer, " HTTP/1.1\r\nHost: ");
    writer_text(&writer, params->host);
    writer_text(&writer, "\r\nUser-Agent: ");
    writer_text(&writer, params->user_agent ? params->user_agent : "Mozilla/5.0 (compatible; tarixenOS/10)");
    writer_text(&writer, "\r\nAccept: */*\r\nAccept-Encoding: identity\r\n");
    if (params->body_length || params->method[0] == 'P') {
        writer_text(&writer, "Content-Length: ");
        writer_number(&writer, params->body_length);
        writer_text(&writer, "\r\n");
    }
    if (params->close_connection) writer_text(&writer, "Connection: close\r\n");
    if (params->extra_headers && params->extra_headers[0]) {
        uint32_t length = crypto_string_length(params->extra_headers);
        writer_text(&writer, params->extra_headers);
        if (params->extra_headers[length - 1] != '\n') writer_text(&writer, "\r\n");
    }
    writer_text(&writer, "\r\n");
    if (writer.overflow) return HTTPS_ERROR_HEADER_TOO_LARGE;
    result = tls_client_send(&connection->tls, connection->io, writer.length);
    if (result < 0) return result;
    if (params->body_length) {
        result = tls_client_send(&connection->tls, params->body, params->body_length);
        if (result < 0) return result;
    }
    connection->io_offset = 0;
    connection->io_length = 0;
    result = read_head(connection, response);
    if (result) return result;
    is_head = params->method[0] == 'H' && params->method[1] == 'E' && params->method[2] == 'A' &&
              params->method[3] == 'D' && params->method[4] == 0;
    if (is_head || response->status_code == 204 || response->status_code == 304 ||
        (response->status_code >= 100 && response->status_code < 200)) return TLS_OK;
    if (response->chunked) result = read_chunked_body(connection, params, response);
    else if (response->content_length != HTTPS_UNKNOWN_LENGTH)
        result = read_counted_body(connection, params, response, response->content_length);
    else {
        response->connection_close = 1;
        result = read_until_close(connection, params, response);
    }
    return result;
}

int https_get(struct https_connection *connection, const char *host, const char *path,
              uint8_t *buffer, uint32_t capacity, struct https_response *response)
{
    struct https_request_params params;
    crypto_zero(&params, sizeof(params));
    params.method = "GET";
    params.host = host;
    params.path = path;
    params.response_buffer = buffer;
    params.response_capacity = capacity;
    return https_request(connection, &params, response);
}

static int resolve_location(struct https_url *target, const char *location)
{
    char absolute[HTTPS_LOCATION_MAX + 8];
    char path[HTTPS_PATH_MAX];
    uint32_t length;
    uint32_t directory = 0;
    if (starts_with_ignore_case(location, "https://")) return https_parse_url(location, target);
    if (starts_with_ignore_case(location, "http://")) return HTTPS_ERROR_URL;
    if (location[0] == '/' && location[1] == '/') {
        crypto_copy(absolute, "https:", 6);
        copy_limited(absolute + 6, sizeof(absolute) - 6, location);
        return https_parse_url(absolute, target);
    }
    if (location[0] == '/') {
        length = crypto_string_length(location);
        if (length >= HTTPS_PATH_MAX) return HTTPS_ERROR_URL;
        crypto_copy(path, location, length + 1);
    } else {
        uint32_t base = 0;
        uint32_t location_length = crypto_string_length(location);
        for (uint32_t i = 0; target->path[i] && target->path[i] != '?'; i++) if (target->path[i] == '/') directory = i + 1;
        base = directory ? directory : 1;
        if (base + location_length >= HTTPS_PATH_MAX) return HTTPS_ERROR_URL;
        if (directory) crypto_copy(path, target->path, base); else path[0] = '/';
        crypto_copy(path + base, location, location_length + 1);
    }
    for (uint32_t i = 0; path[i]; i++) if (path[i] == '#') { path[i] = 0; break; }
    for (uint32_t i = 0; path[i]; i++) if (path[i] == '\r' || path[i] == '\n' || path[i] == ' ') return HTTPS_ERROR_URL;
    crypto_copy(target->path, path, crypto_string_length(path) + 1);
    return TLS_OK;
}

int https_fetch(struct https_connection *connection, const struct https_dialer *dialer,
                const struct tls_config *config, const char *url, uint8_t *buffer, uint32_t capacity,
                struct https_response *response, uint32_t max_redirects)
{
    struct https_url target;
    struct tls_transport transport;
    struct tls_config local;
    struct https_request_params params;
    char host_header[HTTPS_HOST_MAX + 8];
    int result;
    if (!connection || !dialer || !dialer->open || !config || !url || !response) return HTTPS_ERROR_ARGUMENT;
    result = https_parse_url(url, &target);
    if (result) return result;
    for (uint32_t hop = 0;; hop++) {
        uint32_t host_length = crypto_string_length(target.host);
        crypto_copy(host_header, target.host, host_length + 1);
        if (target.port != 443) {
            char digits[6];
            uint32_t count = 0;
            uint32_t value = target.port;
            do { digits[count++] = (char)('0' + value % 10); value /= 10; } while (value);
            host_header[host_length++] = ':';
            while (count) host_header[host_length++] = digits[--count];
            host_header[host_length] = 0;
        }
        if (dialer->open(dialer->context, target.host, target.port, &transport)) return TLS_ERROR_TRANSPORT;
        crypto_copy(&local, config, sizeof(local));
        local.server_name = target.host;
        result = https_connect(connection, &transport, &local);
        if (!result) {
            crypto_zero(&params, sizeof(params));
            params.method = "GET";
            params.host = host_header;
            params.path = target.path;
            params.response_buffer = buffer;
            params.response_capacity = capacity;
            params.close_connection = 1;
            result = https_request(connection, &params, response);
            https_close(connection);
        }
        if (dialer->close) dialer->close(dialer->context, &transport);
        if (result) return result;
        if (!(response->status_code == 301 || response->status_code == 302 || response->status_code == 303 ||
              response->status_code == 307 || response->status_code == 308) ||
            !response->location[0] || hop >= max_redirects) return TLS_OK;
        result = resolve_location(&target, response->location);
        if (result) return result;
    }
}

int https_close(struct https_connection *connection)
{
    if (!connection) return HTTPS_ERROR_ARGUMENT;
    return tls_client_close(&connection->tls);
}

#endif