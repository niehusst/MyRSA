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




/*
 * A simple TCP/IP echo server using RSA encryption to make sure what you're
 * sending yourself isn't seen by any eavesdroppers.
 * Optionally, a port number can be given as a command line argument. If
 * port number isn't specified, the OS chooses.
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
  unsigned short port;
  //
  port = argc==2 ? (unsigned short)atoi(argv[1]) : 0;
  struct sockaddr_in addr = {
    .sin_addr.s_addr = INADDR_ANY,
    .sin_family = AF_INET,
    .sin_port = htons(port) //OS chooses arbitrary open port number to listen at
  };

  //bind the sock addr to file descriptor
  if(bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("Binding socket failed: ");
    exit(FAIL);
  }

  //get socket info so client can connect
  socklen_t server_addr_len = sizeof(struct sockaddr_in);
  if(getsockname(s, (struct sockaddr*)&addr, &server_addr_len)) {
    close(s);
    exit(FAIL);
  }
  port = ntohs(addr.sin_port);
  printf("Sever is listening on port: %d\n", port);

  //tell socket to listen at port number
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

  // exchange keys with client
  key_pair_t *their_pub_key = malloc(sizeof(key_pair_t));
  if(crypto_handshake_with_client(their_pub_key, pub, client_socket)) {
    fprintf(stderr, "%s\n", "Failed to do cryptographic handshake with client");
    exit(2);
  }

  // communicate infinitely with them (break on client program exit)
  while(1) {
    char *msg = NULL; //DEBUG this prob causes problesm
    get_encrypted_message(client_socket, &msg, priv);

    send_encrypted_message(client_socket, msg, their_pub_key);
  }

  //clean up server
  close(client_socket);
  close(s);
  free(their_pub_key);

  return 0;
}
