/**
 * A file containing methods to convert plaintext string to
 * an integer (aka padding) so the text can be operated
 * on via the RSA algorithm
 */
 #include <stdio.h>
 #include <string.h>
 #include <openssl/bn.h>

 #include "pad.h"
 

/**
 * Following the idea in the header, convert plaintext to an integer using an
 * invertable operation. The user is responsible for defining, initing and
 * freeing padded.
 *
 * @param padded - the BIGNUM to save the resulting padded text to
 * @param plaintext - c_string plain text to convert to an integer
 * @return status - error status of the function; 1 on error, else 0
 */
int text_to_num(BIGNUM *padded, const char *plaintext) {
  int status = 0;
  char* buf = malloc(sizeof(char) * ((strlen(plaintext)+1)*3)+1);

  if(buf == NULL) {
    fprintf(stderr, "Failed to allocate space for padded text\n");
    status = 1;
  }

  // convert plaintext to decimal string
  memset(buf, '0', ((strlen(plaintext)+1)*3));
  buf[0] = '1'; // add front pad
  for(int ch = 0; ch < strlen(plaintext); ch++) {
    int buf_pos = (ch+1) * 3;
    char *temp = malloc(sizeof(char) * 4);

    snprintf(temp, 4, "%d", plaintext[ch]);
    if(plaintext[ch] < 100) {
      // copy 2 digits, leaving leading 0
      memcpy(buf+buf_pos+1, temp, sizeof(char)*2);
    } else {
      // copy 3 digits
      memcpy(buf+buf_pos, temp, sizeof(char)*3);
    }
    free(temp);
  }

  // write to BIGNUM struct that can be operated on;
  if(!BN_dec2bn(&padded, buf)) {
    fprintf(stderr, "Failed to pad plaintext\n");
    status = 1;
  }

  free(buf);
  return status;
}

/**
 * Following the idea in the header, convert an integer to plaintext using an
 * invertable operation. The user is responsible for defining and freeing
 * plaintext, but not allocating space for it.
 *
 * @param padded - the BIGNUM to save the resulting padded text to
 * @param plaintext - c_string plain text to convert to an integer
 * @return status - error status of the function; 1 on error, else 0
 */
int num_to_text(char **plaintext, const BIGNUM *padded) {
  int status = 0;
  char* buf = BN_bn2dec(padded);
  int len = strlen(buf) / 3;
  *plaintext = malloc(sizeof(char) * len);

  if(*plaintext == NULL) {
    fprintf(stderr, "Failed to allocate space for plaintext\n");
    status = 1;
  }

  // convert decimal string in buf to plaintext
  for(int ch = 0; ch < len - 1; ch++) {
    int buf_pos = (ch+1) * 3;
    char* temp = malloc(sizeof(char) * 4);
    // cast each chunck of 3 digits (ascii code) back to char
    memcpy(temp, buf+buf_pos, sizeof(char)*3);
    (*plaintext)[ch] = atoi(temp);

    free(temp);
  }

  free(buf);
  return status;
}
