#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/bn.h>

#include "rsa.h"


// test for strong liars in Key generation
void test_keys_work(int rounds, int bits) {
  printf("Running test_keys_work");
  fflush(stdout);
  key_pair_t *pub = malloc(sizeof(key_pair_t));
  key_pair_t *priv = malloc(sizeof(key_pair_t));
  // testing for key gen strong liers
  int fail = 0;
  for(int i = 0; i < rounds; i++) {
    // progress
    int print = (i % (rounds/10)) == 0;
    if(print) printf(".");
    fflush(stdout);

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *rando = BN_new();
    BIGNUM *temp = BN_new();
    BIGNUM *dec = BN_new();

    // get som keys
    RSA_keys_generate(pub, priv, bits);

    // generate random "message" BIGNUM
    BN_pseudo_rand(rando, 256, -1, 0);

    // encrypt and decrypt
    BN_mod_exp(temp, rando, pub->power, pub->mod, ctx);
    BN_mod_exp(dec, temp, priv->power, priv->mod, ctx);

    // compare before and after "message"
    if(BN_cmp(rando, dec) != 0) {
      fail++;
    }

    BN_free(rando);
    BN_free(temp);
    BN_free(dec);
    BN_CTX_free(ctx);
  }
  printf("DONE\n");
  double percent = (double)(rounds-fail) / (double)rounds;
  printf("Pass rate: %d%%\n", (int)(percent * 100));
}

// test that keys work for larger bit values. wont bother testing bit values
// result in what parameter is; they almost certainly wont and I didn't intend
// them to
void test_keys_bits() {
  printf("%s\n", "Testing Key Functionality at different key sizes");
  int bit_values[] = {64, 128, 256, 512};//, 1024};//uncommenting this will make tests take 30min
  int len = sizeof(bit_values) / sizeof(bit_values[0]);
  for(int i = 0; i < len; i++) {
    printf("Test key length %d\n", bit_values[i]);
    test_keys_work(10, bit_values[i]);
  }
}

int main(void) {
  printf("%s\n", "Running Key Test Suite\n");
  test_keys_work(100, 256);
  test_keys_bits();
  return 0;
}
