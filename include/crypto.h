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
int encrypt_str(BIGNUM* cipher_text, const char* msg, const key_pair_t* pub_key);

/**
 * Dencrypt the data in cipher_text using priv_key as the encryption key.
 *
 * @param msg - the location to store the decrypted clear text
 * @param cipher_text - the enciphered data to decrypt
 * @param priv_key - the key to use to dencrypt cipher_text
 * @return status - function error status, 1 on failure, 0 on success
 */
int decrypt_str(char** msg, const BIGNUM* cipher_text, const key_pair_t* priv_key);

/**
 * Read a message over the network from the client (in chunks since I assume
 * BIGNUM is a large struct). Then, decrypt the enciphered message they sent
 * (encrypted using OUR public key) using our private key. Resulting message
 * is stored in plaintext.
 *
 * @param server_socket - the socket_fd for communicating with the client
 * @param plaintext - location to save the resulting plain text string to
 * @param priv_key - the private key generated by message receiver. For decrypting
 *                    the message
 */
void get_encrypted_message(int socket, char **plaintext, key_pair_t *priv_key);

/**
 * Send a message over the network to the client (implicitly writes in chunks
 * if large enough message size). Then, encrypt plaintext message input by user
 * using THEIR public key so they can decrypt it.
 *
 * @param server_socket - the socket_fd for communicating with the client
 * @param plaintext - plain text string to encrypt and then send over network
 * @param priv_key - the public key generated by message receiver. For encrypting
 *                    the message
 */
void send_encrypted_message(int socket, char *plaintext, key_pair_t *their_pub_key);

/**
 * Public key cryptography; exchange public keys with other machine (they don't
 * need to be sent securely since they are useless without the private key, which
 * is kept secret and is NEVER sent over the network). Knowing our recipient's
 * public key and them knowing ours allows the machines to communicate securely.
 *
 * @param their_pub_key - the struct to save the key read over network into
 * @param my_pub_key - the key to send over the network to the client
 * @param client_socket - the socket_fd to send/read the keys to/from
 */
void crypto_handshake_with_client(key_pair_t *their_pub_key,
                                  key_pair_t *my_pub_key,
                                  int client_socket);

/**
 * Public key cryptography; exchange public keys with other machine (they don't
 * need to be sent securely since they are useless without the private key, which
 * is kept secret and is NEVER sent over the network). Knowing our recipient's
 * public key and them knowing ours allows the machines to communicate securely.
 *
 * Notice read/write order is reversed from client version. Wouldn't want to
 * deadlock on each machine waiting to hear from each other first.
 *
 * @param their_pub_key - the struct to save the key read over network into
 * @param my_pub_key - the key to send over the network to the client
 * @param server_socket - the socket_fd to send/read the keys to/from
 */
void crypto_handshake_with_server(key_pair_t *their_pub_key,
                                  key_pair_t *my_pub_key,
                                  int server_socket);

// Print a BIGNUM struct
void bn_print(BIGNUM *bn);

#endif
