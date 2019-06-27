/*
 * A small collection of functions that will be useful for "secure"
 * (my implementation probably isn't professional quality) encrypted
 * communication over the network.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/bn.h>

#include "pad.h"
#include "rsa.h"
#include "crypto.h"


/**
 * Encrypt the data in the string msg using pub_key as the encryption key.
 *
 * @param cipher_text - the struct to store the resulting encipherment in
 * @param msg - the message to encrypt
 * @param pub_key - the key to use to encrypt msg
 * @return status - function error status, 1 on failure, 0 on success
 */
int encrypt_str(BIGNUM* cipher_text, const char* msg, const key_pair_t* pub_key) {
  int status = 0;
  BN_CTX *ctx = BN_CTX_new();
  BIGNUM *temp = BN_new();

  // pad msg
  if(text_to_num(temp, msg)) {
    fprintf(stderr, "%s\n", "Message padding failed");
    status = 1;
  }

  // encrypt
  if(!BN_mod_exp(cipher_text, temp, pub_key->power, pub_key->mod, ctx)) {
    fprintf(stderr, "%s\n", "Message encryption failed");
    status = 1;
  }

  BN_CTX_free(ctx);
  BN_free(temp);
  return status;
}

/**
 * Dencrypt the data in cipher_text using priv_key as the encryption key.
 *
 * @param msg - the location to store the decrypted clear text
 * @param cipher_text - the enciphered data to decrypt
 * @param priv_key - the key to use to dencrypt cipher_text
 * @return status - function error status, 1 on failure, 0 on success
 */
int decrypt_str(char** msg, const BIGNUM* cipher_text, const key_pair_t* priv_key) {
  int status = 0;
  BN_CTX *ctx = BN_CTX_new();
  BIGNUM *ct = BN_new();

  // decrypt
  if(!BN_mod_exp(ct, cipher_text, priv_key->power, priv_key->mod, ctx)) {
    fprintf(stderr, "%s\n", "Message decryption failed");
    status = 1;
  }

  // unpad decrypted cipher text into msg
  if(num_to_text(msg, ct)) {
    fprintf(stderr, "%s\n", "Message unpadding failed");
    status = 1;
  }

  BN_CTX_free(ctx);
  return status;
}


// Print a BIGNUM struct
void bn_print(BIGNUM *bn) {
  char* debug;
  debug = BN_bn2dec(bn);
  printf("%s\n", debug);

  OPENSSL_free(debug);
}


/**
 * Read a BIGNUM struct over the network from the specified socket.
 * Really just reads a string and converts to BIGNUM behind the scenes.
 *
 * @param socket - the socket_fd associated with a connected machine
 * @param bn - the BIGNUM struct to read data into
 * @return status - the error status; 1 on failure, else 0
 */
int bn_read(int socket, BIGNUM *bn) {
  int status = 0;

  // get size of string data to read
  int bn_size = 0;
  int total_bytes_read = 0;
  do {
    int bytes_read = read(socket, &bn_size, sizeof(int)-total_bytes_read);
    if(bytes_read < 0) {
      fprintf(stderr, "%s\n", "Reading BIGNUM size failed");
      return 1;
    }
    total_bytes_read += bytes_read;
  } while(total_bytes_read < sizeof(int));

  // read string of size bn_size
  char *bn_str = malloc(sizeof(char)*bn_size);
  total_bytes_read = 0;
  do {
    int bytes_read = read(socket,
                          bn_str + total_bytes_read,
                          bn_size - total_bytes_read);

    if(bytes_read < 0) {
      fprintf(stderr, "%s\n", "Reading BIGNUM string failed");
      return 1;
    }
    total_bytes_read += bytes_read;
  } while(total_bytes_read < bn_size);

  bn_str[bn_size] = '\0'; // enforced null termination

  // write to target bn (MUST BE INITIALIZED BEFORE HERE)
  BN_dec2bn(&bn, bn_str);

  free(bn_str);
  return status;
}


/**
 * Write a BIGNUM struct over the network to the specified socket.
 * Really just converts BIGNUM to string and writes char* behind the scenes.
 *
 * @param socket - the socket_fd associated with a connected machine
 * @param bn - the BIGNUM struct to write data from
 * @return status - the error status; 1 on failure, else 0
 */
int bn_write(int socket, const BIGNUM *bn) {
  int status = 0;

  // convert BN to string
  char *bn_str;
  bn_str = BN_bn2dec(bn);
  // get size of string and write it to receiving machine
  // write the string itself after receiver knows how many bytes to read
  int bn_size = strlen(bn_str);
  if(write(socket, &bn_size, sizeof(int)) == -1 ||
     write(socket, bn_str, sizeof(char)*bn_size) == -1) {
    fprintf(stderr, "%s\n", "Writing BN failed");
    status = 1;
  }

  OPENSSL_free(bn_str);
  return status;
}


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
 * @return status - error status; 1 on failure, 0 on success
 */
int get_encrypted_message(int socket, char **plaintext, key_pair_t *priv_key) {
  int status = 0;
  // read message sent from client (it's an encrypted BIGNUM struct)
  BIGNUM *message = BN_new();
  if(bn_read(socket, message)) status = 1;

  // decrypt to string
  if(decrypt_str(plaintext, message, priv_key)) status = 1;

  BN_free(message);
  return status;
}

/**
 * Send a message over the network to the client (implicitly writes in chunks
 * if large enough message size). Then, encrypt plaintext message input by user
 * using THEIR public key so they can decrypt it.
 *
 * @param server_socket - the socket_fd for communicating with the client
 * @param plaintext - plain text string to encrypt and then send over network
 * @param priv_key - the public key generated by message receiver. For encrypting
 *                    the message
 * @return status - error status; 1 on failure, 0 on success
 */
int send_encrypted_message(int socket, char *plaintext, key_pair_t *their_pub_key) {
  int status = 0;
  BIGNUM *cipher_text = BN_new();
  // encrypt message
  if(encrypt_str(cipher_text, plaintext, their_pub_key)) status = 1;

  // interact with client by echoing re-encrypted message back to them
  if(bn_write(socket, cipher_text)) status = 1;

  BN_free(cipher_text);
  return status;
}


/**
 * Public key cryptography; exchange public keys with other machine (they don't
 * need to be sent securely since they are useless without the private key, which
 * is kept secret and is NEVER sent over the network). Knowing our recipient's
 * public key and them knowing ours allows the machines to communicate securely.
 *
 * Notice read/write order is reversed from server version. Wouldn't want to
 * deadlock on each waiting to hear from each other first.
 *
 * @param their_pub_key - the struct to save the key read over network into
 * @param my_pub_key - the key to send over the network to the client
 * @param server_socket - the socket_fd to send/read the keys to/from
 * @return int - error status; 1 on failure, 0 on success
 */
int crypto_handshake_with_server(key_pair_t *their_pub_key,
                                  key_pair_t *my_pub_key,
                                  int server_socket) {
  // send them our public key
  if(bn_write(server_socket, my_pub_key->power)) {
    fprintf(stderr, "%s\n", "Writing client key power failed");
    return 1;
  }

  if(bn_write(server_socket, my_pub_key->mod)) {
    fprintf(stderr, "%s\n", "Writing client key mod failed");
    return 1;
  }

  // read power part of key over network
  their_pub_key->power = BN_new();
  if(bn_read(server_socket, their_pub_key->power)) {
    fprintf(stderr, "%s\n", "Reading server key power failed");
    return 1;
  }

  // read in the mod part of key over the network
  their_pub_key->mod = BN_new();
  if(bn_read(server_socket, their_pub_key->mod)) {
    fprintf(stderr, "%s\n", "Reading server key mod failed");
    return 1;
  }

  return 0;
}

/**
 * Public key cryptography; exchange public keys with other machine (they don't
 * need to be sent securely since they are useless without the private key, which
 * is kept secret and is NEVER sent over the network). Knowing our recipient's
 * public key and them knowing ours allows the machines to communicate securely.
 *
 * @param their_pub_key - the struct to save the key read over network into
 * @param my_pub_key - the key to send over the network to the client
 * @param client_socket - the socket_fd to send/read the keys to/from
 * @return int - error status; 1 on failure, 0 on success
 */
int crypto_handshake_with_client(key_pair_t *their_pub_key,
                                  key_pair_t *my_pub_key,
                                  int client_socket) {
  // read power part of key over network
  their_pub_key->power = BN_new();
  if(bn_read(client_socket, their_pub_key->power)) {
    fprintf(stderr, "%s\n", "Reading client key power failed");
    return 1;
  }

  // read in the mod part of key over the network
  their_pub_key->mod = BN_new();
  if(bn_read(client_socket, their_pub_key->mod)){
    fprintf(stderr, "%s\n", "Reading client key mod failed");
    return 1;
  }

  // send them our public key
  if(bn_write(client_socket, my_pub_key->power) ||
     bn_write(client_socket, my_pub_key->mod)) {
    fprintf(stderr, "%s\n", "Writing server key to client failed");
    return 1;
  }

  return 0;
}
