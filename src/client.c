#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "pad.h"
#include "rsa.h"
#include "crypto.h"

#define FAIL 2



/*
 * A simple client program to connect via TCP/IC to a host server
 * - takes a host name in command line args
 */
int main(int argc, char** argv) {
  if(argc < 3) {
    printf("Usage: ./%s <SERVER_HOST_NAME> <PORT_NUMBER>\n", argv[0]);
    printf("Try 'localhost' if you are running server.c on the same computer.\n");
    exit(0);
  }

  // get RSA encryption keys
  key_pair_t *pub = malloc(sizeof(key_pair_t));
  key_pair_t *priv = malloc(sizeof(key_pair_t));
  get_keys(pub, priv);

  //get host name from command line and convert to IP addr
  struct hostent* server = gethostbyname(argv[1]);
  if(server == NULL) {
    //no actual error occurred, so fprintf to stderr
    fprintf(stderr, "Unable to find host %s", argv[1]);
    exit(1);
  }

  //set up socket to server
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1) {
    perror("socket failed: ");
    exit(FAIL);
  }

  //init socket address info
  unsigned short port = (unsigned short) atoi(argv[2]);
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port)
  };

  //fill host address into server variable
  //copy bytes from server to addr
  bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length);

  //connect to the server
  if(connect(server_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("connect failed: ");
    exit(FAIL);
  }

  // do handshake
  key_pair_t *their_pub_key = malloc(sizeof(key_pair_t));
  if(crypto_handshake_with_server(their_pub_key, pub, server_socket)) {
    fprintf(stderr, "%s\n", "Failed to do cryptographic handshake with server");
    exit(2);
  }

  // infinitely communicate with server
  printf("You are now connected with the server!\nPress '^C' to exit program\n");
  while(1) {
    //get a message from user via stdin
    const int size = 256;
    char msg[size];
    printf("Enter a message to send to the server:\n");
    fgets(msg, size, stdin);

    //DEBUG
    printf("%s\n", "about to send message");

    //send message to server
    send_encrypted_message(server_socket, msg, their_pub_key);

    printf("%s\n", "about to get message"); //DEBUG

    //read data sent from server
    char *return_msg;
    get_encrypted_message(server_socket, &return_msg, priv);

    printf("Server sent message: %s\n", return_msg);
  }

  //clean up open sockets
  close(server_socket);
  free(their_pub_key);

  return 0;
}
