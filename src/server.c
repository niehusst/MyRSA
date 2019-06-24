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

//TODO fix to make do more than 1 communication round
//TODO add encryption to messaging
//TODO network code in demo.sh is failing because client is being launched b4 server sets up bcus of rsa stuff
//TODO handshake code
// Print a BIGNUM struct
void bn_print(BIGNUM *bn) {
  char* debug;
  debug = BN_bn2dec(bn);
  printf("%s\n", debug);

  OPENSSL_free(debug);
}


/*
  A simple TCP/IP echo server
  (to test; run the server executable and then in a separate window,
  `nc localhost 4444` to get test server functionality)
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

  //read message sent from client
  const int size = 256;
  char msg[size];
  int bytes_read = read(client_socket, msg, size);
  if(bytes_read < 0) {
    perror("read failed: ");
    exit(FAIL);
  }

  //interact with client by echoing message back to them
  write(client_socket, msg, strlen(msg));

  //clean up server
  close(client_socket);
  close(s);

  return 0;
}
