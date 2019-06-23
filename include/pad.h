#ifndef __PAD_HEADER__
#define __PAD_HEADER__

#include <openssl/bn.h>

/*
idea:
convert each char in plaintext to its ascii value (make sure has 3
digits; add leading 0s if need be). So that first letter isn't mis
interpreted if leading 0s are chopped off by BIGNUM, pad text with
the first letter of BIGNUM padded text as a 100.

e.g.
|         B     U     G
v  100   066   085   071  = 100066085071

appending the numbers as a big string will clearly create a real
value that can be operated on, and the inverse is to convert each
3 digit chunck into 1 letter.

Since this approximately triples the length of the input plaintext,
and requires iteration over the entire plaintext, this is defenitly
not the most efficient algorithm. Good thing this is only a personal
project. :)
*/

/**
 * Following the idea above, convert plaintext to an integer using an
 * invertable operation.
 *
 * @param padded - the BIGNUM to save the resulting padded text to
 * @param plaintext - c_string plain text to convert to an integer
 * @return status - error status of the function; 1 on error, else 0
 */
int text_to_num(BIGNUM *padded, const char *plaintext);

/**
 * Following the idea above, convert an integer to plaintext using an
 * invertable operation.
 *
 * @param padded - the BIGNUM to save the resulting padded text to
 * @param plaintext - c_string plain text to convert to an integer
 * @return status - error status of the function; 1 on error, else 0
 */
int num_to_text(char **plaintext, const BIGNUM *padded);

#endif
