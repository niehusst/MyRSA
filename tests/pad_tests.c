/*
Credit file reading code: user411313
https://stackoverflow.com/questions/3747086/reading-the-whole-text-file-into-a-char-array-in-c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>

#include "pad.h"

int pad_depad(char *f_text) {
  BIGNUM *padded = BN_new();
  char *msg = malloc(sizeof(char)*strlen(f_text));

  text_to_num(padded, f_text);
  num_to_text(&msg, padded);

  int ret = strcmp(f_text, msg);

  BN_free(padded);
  free(msg);
  return ret;
}

// test that padding is an invertible procedure with some medium sized files
void test_padding_invertible1() {
  printf("\n%s\n", "Running test_padding_invertible1");
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( "test1.txt" , "rb" );
  if( !fp ) perror("Failed to open test1.txt"),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the buffer */
  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  /* compare */
  if(pad_depad(buffer))
    printf("%s\n", "FAILURE");
  else
    printf("%s\n", "SUCCESS");

  fclose(fp);
  free(buffer);
}

void test_padding_invertible2() {
  printf("\n%s\n", "Running test_padding_invertible2");
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( "test2.txt" , "rb" );
  if( !fp ) perror("Failed to open test2.txt"),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the buffer */
  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  /* compare */
  if(pad_depad(buffer))
    printf("%s\n", "FAILURE");
  else
    printf("%s\n", "SUCCESS");

  fclose(fp);
  free(buffer);
}

void test_padding_invertible3() {
  printf("\n%s\n", "Running test_padding_invertible3");
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( "test3.txt" , "rb" );
  if( !fp ) perror("Failed to open test3.txt"),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the buffer */
  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  /* compare */
  if(pad_depad(buffer))
    printf("%s\n", "FAILURE");
  else
    printf("%s\n", "SUCCESS");

  fclose(fp);
  free(buffer);
}

int main(void) {
  test_padding_invertible1();
  test_padding_invertible2();
  test_padding_invertible3();
  return 0;
}
