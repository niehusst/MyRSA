#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define FAIL 2

/* 
  A simple client program to connect to a TCP/IC host server
  - takes a host name in command line args
 */
int main(int argc, char** argv) {
  if(argc < 2) {
    printf("Usage: ./%s <HOST_NAME>\n", argv[0]);
    exit(0);
  }
  
  //get host name from command line and convert to IP addr
  struct hostent* server = gethostbyname(argv[1]);
  if(server == NULL) {
    //no actual error occurred, so fprintf to stderr
    fprintf(stderr, "Unable to find host %s", argv[1]);
    exit(1);
  }

  //set up socket to server
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1) {
    perror("socket failed: ");
    exit(FAIL);
  }

  //init socket address info
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(4444)
  };

  //fill host address into server variable
  //copy bytes from server to addr
  bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length);
  
  //connect to the server
  if(connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("connect failed: ");
    exit(FAIL);
  }

  //get a message
  const int size = 256;
  char msg[size];
  printf("Enter a message:\n");
  fgets(msg, size, stdin);

  //send message to server
  write(s, msg, strlen(msg));

  //read data sent from server
  char buffer[size]; //data buffer of arbitrary max size
  int bytes_read = read(s, buffer, size);
  if(bytes_read < 0) {
    perror("read failed: ");
    exit(FAIL);
  }
  
  printf("Server sent message: %s\n", buffer);

  //clean up open sockets
  close(s);

  return 0;
}
