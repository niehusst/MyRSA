/**
 * Uses openSSL under Apache licesnse
 *
 * Compile with:
 * clang -lm -lssl -lcrypto -o rsa rsa.c
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/bn.h>
//#include <openssl/bio.h>


// Print a BIGNUM struct
void bn_print(BIGNUM *bn) {
  char* debug;
  debug = BN_bn2dec(bn);
  printf("%s\n", debug);

  OPENSSL_free(debug);
}


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
    return 1;
  } else {
    //success
    return 0;
  }
}

/**
 * Compute the Euler totient function for some number n defined as the product
 * of p and q.
 *
 * @param p - a prime number (not checked for efficiency)
 * @param q - a prime number (not checked for efficiency)
 * @return - the totient of n where n=p*q
 */
void totient(BIGNUM *t, const BIGNUM *p, const BIGNUM *q, BN_CTX *ctx) {
  BIGNUM *q2, *p2;
  q2 = BN_new();
  p2 = BN_new();
  // set p2 and q2
  BN_sub(p2, p, BN_value_one());
  BN_sub(q2, q, BN_value_one());

  // find Euler totient using lcm
  lcm(t, p2, q2, ctx);

  BN_free(q2);
  BN_free(p2);
}

/**
 * Extended Euclidean algorithm.
 * For getting the multiplicative inverse
 */
void mod_multi_inverse(BIGNUM *ret) {
  //TODO:!!!

}

/**
 * Set 'e' randomly between 2 -- t-1
 *
 * @param e - the struct to write to
 * @param t - totient. used as upper limit on range +1
 * @return bool - err status; 1 on error, 0 on success
 */
int get_e(BIGNUM *e, const BIGNUM *t) {
  BIGNUM *minimum = BN_new(); //2
  BIGNUM *maximum = BN_new(); //t-1
  if(!BN_set_word(minimum, 2)) {
    fprintf(stderr, "Failed to set word, 2\n");
    exit(2);
  }
  BN_sub(maximum, t, BN_value_one());

  int ret = !BN_pseudo_rand_range(e, maximum); // only computes range 0-max
  // do lower bound check manually
  if(BN_cmp(minimum, e) == -1) {
    ret = 1; //failure to fall in valid range
  }

  // clean up
  BN_free(maximum);
  BN_free(minimum);

  return ret;
}

/**
 * Compute least common multiple between n1 and n2. this may be sloww??
 * Resulting value is saved in ret.
 *
 *
 */
void lcm(BIGNUM *ret, const BIGNUM *n1, const BIGNUM *n2, BN_CTX *ctx) {
  BIGNUM *mult, *gcd;
  mult = BN_new();
  gcd = BN_new();

  BN_free(mult);
  BN_free(gcd);

}

//TODO: figure out how to pad a str message (aka turn it into a number) and back

int main(void) {
  srand(time(0));
  BN_CTX *ctx = BN_CTX_new(); // for internal BN usage

  // allocate BIGNUM structs
  BIGNUM *p = BN_new();
  BIGNUM *q = BN_new();

  // num bits in the primes to generate
  int num_bits = 256;

  // init as primes
  select_prime(p, num_bits);
  select_prime(q, num_bits);

  BIGNUM *t, *e, *gcd_result, *n;
  n = BN_new();
  t = BN_new();
  e = BN_new();
  gcd_result = BN_new();

  // set n
  BN_mul(n, p, q, ctx);

  // get the totient value for n=p*q
  totient(t, p, q);

  // e and t must be relatively prime
  do {
    // get e
    if(get_e(e, t)) {
      //error
      fprintf(stderr, "Failed to get valid value for 'e'\n");
      exit(2);
    }

    // check for relative primality
    BN_gcd(gcd_result, t, e, ctx);
  } while(!BN_is_one(gcd_result));

  // use EE to find the multiplicative inverse of the totient
  // assert (e*d) % t == 1

  // use Extended Euclidean to find the multiplicative inverse
  printf("made it and chose a number\n");

  // clean up
  BN_free(q);
  BN_free(p);
  BN_free(t);
  BN_free(e);
  BN_free(gcd_result);
  BN_CTX_free(ctx);

  return 0;
}


/* Useful BN functions

int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
int BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
int BN_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b, BN_CTX *ctx);
int BN_sqr(BIGNUM *r, BIGNUM *a, BN_CTX *ctx);
int BN_div(BIGNUM *dv, BIGNUM *rem, const BIGNUM *a, const BIGNUM *d,
        BN_CTX *ctx);
int BN_mod(BIGNUM *rem, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx);
int BN_nnmod(BIGNUM *rem, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx);
int BN_mod_add(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m,
        BN_CTX *ctx);
int BN_mod_sub(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m,
        BN_CTX *ctx);
int BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m,
        BN_CTX *ctx);
int BN_mod_sqr(BIGNUM *ret, BIGNUM *a, const BIGNUM *m, BN_CTX *ctx);
int BN_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BN_CTX *ctx);
int BN_mod_exp(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
        const BIGNUM *m, BN_CTX *ctx);
int BN_gcd(BIGNUM *r, BIGNUM *a, BIGNUM *b, BN_CTX *ctx);

int BN_cmp(BIGNUM *a, BIGNUM *b);
int BN_ucmp(BIGNUM *a, BIGNUM *b);
int BN_is_zero(BIGNUM *a);
int BN_is_one(BIGNUM *a);
int BN_is_word(BIGNUM *a, BN_ULONG w);
int BN_is_odd(BIGNUM *a);

int BN_zero(BIGNUM *a);
int BN_one(BIGNUM *a);

BIGNUM* BN_value_one(void);
*/
