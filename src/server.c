#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/bn.h>

#include "rsa.h"
#include "crypto.h"

#define FAIL 2



//TODO network code in demo.sh is failing because client is being launched b4 server sets up bcus of rsa stuff

// Print a BIGNUM struct
void bn_print(BIGNUM *bn) {
  char* debug;
  debug = BN_bn2dec(bn);
  printf("%s\n", debug);

  OPENSSL_free(debug);
}

//TODO add encryption to messaging
void get_message(int client_socket, char **plaintext, key_pair_t *priv_key) {
  // read message sent from client (it's an encrypted BIGNUM struct)
  int total_bytes_read = 0;
  do {
    int bytes_read = read(client_socket,
                          their_pub_key + total_bytes_read,
                          sizeof(key_pair_t) - total_bytes_read);
    total_bytes_read += bytes_read;
  } while(total_bytes_read < sizeof(key_pair_t));

  //TODO decrypt to string

}

void send_message(int client_socket, char *plaintext, key_pair_t *their_pub_key) {
  //TODO encrypt message


  // interact with client by echoing re-encrypted message back to them
  write(client_socket, msg, sizeof(msg));
}

//TODO handshake code
/**
 * Public key cryptography, exchange public keys with other server to
 * communicate securely.
 */
void crypto_handshake(key_pair_t *their_pub_key, key_pair_t *my_pub_key, int client_socket) {
  // read in their public key (by chunks if necessary..)
  int total_bytes_read = 0;
  do {
    int bytes_read = read(client_socket,
                          their_pub_key + total_bytes_read,
                          sizeof(key_pair_t) - total_bytes_read);

    if(bytes_read < 0) break;
    total_bytes_read += bytes_read;
  } while(total_bytes_read < sizeof(key_pair_t));

  // send them our public key
  write(client_socket, my_pub_key, sizeof(key_pair_t));
}


/*
 * A simple TCP/IP echo server using RSA encryption to make sure what you're
 * sending yourself isn't seen by any eavesdroppers.
 *
 * (to test; run the server executable and then in a separate window,
 * `nc localhost 4444` to get test server functionality)
 */
int main(int argc, char** argv) {
  // get RSA encryption keys
  key_pair_t *pub = malloc(sizeof(key_pair_t));
  key_pair_t *priv = malloc(sizeof(key_pair_t));
  get_keys(pub, priv);

  //get socket file descriptor
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1) {
    perror("Socket failed: ");
    exit(FAIL);
  }

  //init internet socket address
  struct sockaddr_in addr = {
    .sin_addr.s_addr = INADDR_ANY,
    .sin_family = AF_INET,
    .sin_port = htons(4444) //arbitrary 'unique' port number to listen at
  };

  //bind the sock addr to file descriptor
  if(bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("Binding socket failed: ");
    exit(FAIL);
  }

  //tell socket to listen at port number (4444)
  if(listen(s, 2)) { //up to 2 connections at a time to this port
    perror("Listen failed: ");
    exit(FAIL);
  }

  //accept a incoming connection heard by listen
  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(struct sockaddr_in);
  int client_socket = accept(s, (struct sockaddr*)&client_addr, &client_addr_length);
  if(client_socket == -1) {
    perror("Accept failed: ");
    exit(FAIL);
  }

  crypto_handshake();
  
  while(1) {
    get_message();

    send_message();
  }

  //clean up server
  close(client_socket);
  close(s);

  return 0;
}
