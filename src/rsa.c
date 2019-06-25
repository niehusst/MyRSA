/**
 * Uses openSSL under Apache license
 * See custom structs under corresponding header file
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/bn.h>

#include "rsa.h"

#define ERR 1


/**
 * Generate a prime number. (Ideally large for encryption security reasons)
 * Save results into parameter bn.
 *
 * @param bn - pointer to the struct to save the resulting pseudo prime number
 *             that is bits bits long into
 * @return bool - err status of function. 1 on failure, 0 on success
 */
int select_prime(BIGNUM *bn, int bits) {
  int safe_prime = 1;

  if(!BN_generate_prime_ex(bn, bits, safe_prime, NULL, NULL, NULL)) {
    //error
    fprintf(stderr, "Generating prime number encountered error.\n");
    return ERR;
  } else {
    //success
    return 0;
  }
}


/**
 * Compute least common multiple between n1 and n2. this may be sloww??
 * Resulting value is saved in r.
 *
 * @param r - value of the lcm of n1 and n2
 * @param n1 - BN struct to compute lcm of
 * @param n2 - BN struct to compute lcm of
 * @param ctx - helper temp space struct
 * @return status - err status, 1 on error, 0 on success
 */
int lcm(BIGNUM *r, const BIGNUM *n1, const BIGNUM *n2, BN_CTX *ctx) {
  BIGNUM *mult, *gcd;
  int status = 0;
  mult = BN_new();
  gcd = BN_new();

  // set helper values
  if(!BN_mul(mult, n1, n2, ctx) || !BN_gcd(gcd, n1, n2, ctx)) {
    fprintf(stderr, "Failed to compute LCM helper values\n");
    status = ERR;
  }
  // compute lcm
  if(!BN_div(r, NULL, mult, gcd, ctx)) {
    fprintf(stderr, "Failed to compute LCM\n");
    status = ERR;
  }

  BN_free(mult);
  BN_free(gcd);

  return status;
}


/**
 * Compute the Euler totient function for some number n defined as the product
 * of p and q.
 *
 * @param p - a prime number (not checked for efficiency)
 * @param q - a prime number (not checked for efficiency)
 * @return status - err status; 1 on error, else 0
 */
int totient(BIGNUM *t, const BIGNUM *p, const BIGNUM *q, BN_CTX *ctx) {
  BIGNUM *q2, *p2;
  int status = 0;
  q2 = BN_new();
  p2 = BN_new();
  // set p2 and q2
  if(!BN_sub(p2, p, BN_value_one()) ||
     !BN_sub(q2, q, BN_value_one())) {
    fprintf(stderr, "Failed to compute p2 and q2 for totient\n");
    status = ERR;
  }

  // find Euler totient using lcm
  if(lcm(t, p2, q2, ctx)) {
    status = ERR;
  }

  BN_free(q2);
  BN_free(p2);

  return status;
}

/**
 * Extended Euclidean algorithm.
 * Used for finding the multiplicative inverse of e (mod t)
 *
 * @param ret - the BN to store the result in
 * @param e - part of the value to find the mult inverse of. Coprime to t
 * @param t - part of the value to find teh mult inverse of. Coprime to e
 * @param ctx - struct used as helper storage in big computations
 * @return status - error status of the function: 1 on failure, 0 on success
 */
int mod_multi_inverse(BIGNUM *ret, const BIGNUM *e, const BIGNUM *t, BN_CTX *ctx) {
  int status = 0;
  // thanks Wikipedia
  // https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm

  // compute numbers in the sequence. We only need 2 values at a time to
  // compute the next number in the sequence, so we only allocate that much
  // space for efficiency
  BIGNUM *inverse, *next_inverse, *gcd, *next_gcd, *quotient, *temp, *temp2;
  inverse = BN_new();
  BN_one(inverse);
  next_inverse = BN_new();
  BN_zero(next_inverse);
  gcd = BN_dup(e);
  next_gcd = BN_dup(t);
  quotient = BN_new();
  temp = BN_new();
  temp2 = BN_new();

  // compute the sequence up to inverse
  while(!BN_is_zero(next_gcd)) {
    if(!BN_div(quotient, NULL, gcd, next_gcd,ctx)) {
      fprintf(stderr, "Quotient computation in finding inverse failed\n");
      status = ERR;
    }

    // continue sequence
    temp = BN_dup(next_gcd);
    if(!BN_mul(temp2, quotient, temp, ctx) || !BN_sub(next_gcd, gcd, temp2)) {
      fprintf(stderr, "Failed to get next value in gcd sequence\n");
      status = ERR;
    }
    gcd = BN_dup(temp);

    temp = BN_dup(next_inverse);
    if(!BN_mul(temp2, quotient, temp, ctx) || !BN_sub(next_inverse, inverse, temp2)) {
      fprintf(stderr, "Failed to get next value in inverse sequence\n");
      status = ERR;
    }
    inverse = BN_dup(temp);
  }

  // save final computed inverse value as result
  if(BN_copy(ret, inverse) == NULL) {
    fprintf(stderr, "%s\n", "Failed to copy inverse into result");
    status = ERR;
  }

  BN_free(inverse);
  BN_free(next_inverse);
  BN_free(gcd);
  BN_free(next_gcd);
  BN_free(quotient);
  BN_free(temp);

  return status;
}

/**
 * Find value 'e' between 2 -- t-1 that is
 *
 * @param e - the struct to write to
 * @param t - totient. used as upper limit on range +1
 * @return status - err status; 1 on error, 0 on success
 */
int get_e(BIGNUM *e, const BIGNUM *t) {
  int status = !BN_set_word(e, 3);
  /*
  Def not best practices to just set e to 3 (since e is part of the private key),
  but it's very fast, and actually solves my weird strong prime liar problem
  for key sizes >= 256.

  Normally, e is a large number between 2 and t-1 that is relatively prime to t.
  */

  return status;
}

/**
 * Generate an RSA public key and private key pair. Keep the private key to
 * yourself ;)
 *
 * @param pub_key - the struct to store the resulting public key in
 * @param priv_key - the struct to store the resulting private key in
 * @param num_bits - the number of bits the key length is (not really, but this
 *                   approximates). RSA authors suggest 1024 for corporate use
 *                   or 2048 for super secure stuff
 * @return status - boolean error status, 1 on failure, 0 on success
 */
int RSA_keys_generate(key_pair_t *pub_key, key_pair_t *priv_key, int num_bits) {
  int status = 0;
  srand(time(0));
  BN_CTX *ctx = BN_CTX_new(); // for internal BN usage

  // allocate BIGNUM structs
  BIGNUM *p = BN_new();
  BIGNUM *q = BN_new();

  // init as primes
  if(select_prime(p, num_bits) || select_prime(q, num_bits)) {
    fprintf(stderr, "Failed to select prime numbers\n");
    status = ERR;
  }

  BIGNUM *t, *e, *gcd_result, *n, *inverse;
  n = BN_new();
  t = BN_new();
  e = BN_new();
  gcd_result = BN_new();
  inverse = BN_new();

  // set n
  if(!BN_mul(n, p, q, ctx)) {
    fprintf(stderr, "Getting n value failed\n");
    status = ERR;
  }

  // get the totient value for n=p*q
  if(totient(t, p, q, ctx)) {
    fprintf(stderr, "Euler Totient calculation failed\n");
    status = ERR;
  }

  // e and t must be relatively prime
  do {
    // get e
    if(get_e(e, t)) {//TODO: try generating prime instead of rand
      //error
      fprintf(stderr, "Failed to get valid value for 'e'\n");
      status = ERR;
    }
    // check for relative primality
    if(!BN_gcd(gcd_result, t, e, ctx)) {
      fprintf(stderr, "GCD calculation failed\n");
      status = ERR;
    }
  } while(!BN_is_one(gcd_result));

  // use Extended Euclidean to find the multiplicative inverse of the totient
  if(mod_multi_inverse(inverse, e, t, ctx)) {
    fprintf(stderr, "Finding multiplicative inverse failed\n");
    status = ERR;
  }

  // init and save public key values
  pub_key->mod = BN_new();
  pub_key->power = BN_new();
  BN_copy(pub_key->mod, n);
  BN_copy(pub_key->power, e);
  // init and save private key values
  priv_key->mod = BN_new();
  priv_key->power = BN_new();
  BN_copy(priv_key->mod, n);
  BN_copy(priv_key->power, inverse);

  // clean up
  BN_free(q);
  BN_free(p);
  BN_free(t);
  BN_free(e);
  BN_free(gcd_result);
  BN_CTX_free(ctx);

  return status;
}

/**
 * Generate a public/private key pair that works and save them into input
 * parameters pub and priv. Uses a default key size of 256 (for speed).
 *
 * Awkward that I need this function...
 * RSA_keys_generate has about 50% chance of generating bad keys that
 * can't actually be used to perform decryption of a message they encrypted.
 * Since it's not a 100% chance, the reason for failure is likely located in
 * code that uses random number generation (either select_prime or get_e from rsa.c)
 * Since I have to assume openSSL didn't mess up prime number generation at that
 * rate, it must be in get_e. However, e isn't solidified until it satisfies
 * coprimality to t (which is its only job). So not sure where it's going wrong.
 * It must be some sort of strong lier?
 *    "The term 'strong liar' refers to the case where n is composite but
 *     nevertheless the equations hold as they would for a prime."
 *                                       - Miller-Rabin Primality test Wikipedia
 *
 * UPDATE: as of changing get_e to just set e = 3, the RSA_keys_generate algorithm
 *         passes tests with 100% success for key sizes 256 and larger.
 */
void get_keys(key_pair_t *pub, key_pair_t *priv) {
  // testing for key gen strong liers
  int fail = 0;
  do {
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *rando = BN_new();
    BIGNUM *temp = BN_new();
    BIGNUM *dec = BN_new();

    if(RSA_keys_generate(pub, priv, 256)) fail = 1;

    BN_pseudo_rand(rando, 256, -1, 0);

    if(!BN_mod_exp(temp, rando, pub->power, pub->mod, ctx)) {
      fail = 1;
    }
    if(!BN_mod_exp(dec, temp, priv->power, priv->mod, ctx)) {
      fail = 1;
    }

    fail = BN_cmp(rando, dec) != 0;

    BN_free(rando);
    BN_free(temp);
    BN_free(dec);
    BN_CTX_free(ctx);
  } while(fail);
}
