#ifndef __RSA_HEADER__
#define __RSA_HEADER__
#include <openssl/bn.h>

/**
 * Struct for holding an encryption key
 * (Defined as a pair of numbers used to exponentiate and modulo plaintext)
 */
typedef struct key_pair {
  BIGNUM *mod;
  BIGNUM *power;
} key_pair_t;

/**
 * Generate an RSA public key and private key pair. Keep the private key to
 * yourself ;)
 *
 * @param pub_key - the struct to store the resulting public key in
 * @param priv_key - the struct to store the resulting private key in
 * @return status - boolean error status, 1 on failure, 0 on success
 */
int RSA_keys_generate(key_pair_t *pub_key, key_pair_t *priv_key);

/**
 * Generate a public/private key pair that actually works.
 * Caller is responsible for allocating and freeing key_pair_t memory.
 *
 * @param pub - the struct to store the resulting public key in
 * @param priv - the struct to store the resulting private key in
 */
void get_keys(key_pair_t *pub, key_pair_t *priv);

#endif
