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
 * Generate a prime number. (Ideally large)
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
 * Find value 'e' randomly between 2 -- t-1
 *
 * @param e - the struct to write to
 * @param t - totient. used as upper limit on range +1
 * @return status - err status; 1 on error, 0 on success
 */
int get_e(BIGNUM *e, const BIGNUM *t) {
  BIGNUM *minimum = BN_new(); //2
  BIGNUM *maximum = BN_new(); //t-1
  int status = 0;
  // set range limit values
  if(!BN_set_word(minimum, 2)) {
    fprintf(stderr, "Failed to set word, 2\n");
    status = ERR;
  }
  if(!BN_sub(maximum, t, BN_value_one())) {
    fprintf(stderr, "Failed to compute t-ERR\n");
    status = ERR;
  }

  // compute the range
  // only computes range 0-max
  if(!BN_pseudo_rand_range(e, maximum)) {
    fprintf(stderr, "Range computation failed\n");
    status = ERR;
  }
  // do lower bound check manually
  if(BN_cmp(e, minimum) == -1) {
    fprintf(stderr, "e is less than min\n");
    status = ERR; //failure to fall in valid range
  }

  // clean up
  BN_free(maximum);
  BN_free(minimum);

  return status;
}

/**
 * Generate an RSA public key and private key pair. Keep the private key to
 * yourself ;)
 *
 * @param pub_key - the struct to store the resulting public key in
 * @param priv_key - the struct to store the resulting private key in
 * @return status - boolean error status, 1 on failure, 0 on success
 */
int RSA_keys_generate(key_t *pub_key, key_t *priv_key) {
  int status = 0;
  srand(time(0));
  BN_CTX *ctx = BN_CTX_new(); // for internal BN usage

  // allocate BIGNUM structs
  BIGNUM *p = BN_new();
  BIGNUM *q = BN_new();

  // num bits in the primes to generate
  int num_bits = 256;

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
  BN_mul(n, p, q, ctx);

  // get the totient value for n=p*q
  if(totient(t, p, q, ctx)) {
    fprintf(stderr, "Euler Totient calculation failed\n");
    status = ERR;
  }

  // e and t must be relatively prime
  do {
    // get e
    if(get_e(e, t)) {
      //error
      fprintf(stderr, "Failed to get valid value for 'e'\n");
      status = ERR;
    }
    // check for relative primality
    BN_gcd(gcd_result, t, e, ctx);
  } while(!BN_is_one(gcd_result));

  // use Extended Euclidean to find the multiplicative inverse of the totient
  if(mod_multi_inverse(inverse, e, t, ctx)) {
    fprintf(stderr, "Finding multiplicative inverse failed\n");
    status = ERR;
  }

  //DEBUG assert (e*d) % t == 1
  BIGNUM *test = BN_new();
  BN_mod_mul(test, e, inverse, t, ctx);
  if(!BN_is_one(test)) {
    fprintf(stderr, "Mathematical tautology not upheld: Math broke\n");
    status = ERR;
  }
  BN_free(test);

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
