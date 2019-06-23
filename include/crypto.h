#ifndef __CRYPTO_HEADER__
#define __CRYPTO_HEADER__

#include "rsa.h"

/**
 * Encrypt the data in the string msg using pub_key as the encryption key.
 *
 * @param cipher_text - the struct to store the resulting encipherment in
 * @param msg - the message to encrypt
 * @param pub_key - the key to use to encrypt msg
 * @return status - function error status, 1 on failure, 0 on success
 */
int encrypt(BIGNUM* cipher_text, const char* msg, const key_pair_t* pub_key);

/**
 * Dencrypt the data in cipher_text using priv_key as the encryption key.
 *
 * @param msg - the location to store the decrypted clear text
 * @param cipher_text - the enciphered data to decrypt
 * @param priv_key - the key to use to dencrypt cipher_text
 * @return status - function error status, 1 on failure, 0 on success
 */
int decrypt(char** msg, const BIGNUM* cipher_text, const key_pair_t* priv_key);


#endif
