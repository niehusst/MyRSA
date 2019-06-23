/**
 * A file containing methods to convert plaintext string to
 * an integer (aka padding) so the text can be operated
 * on via the RSA algorithm
 */

/*
idea:
convert each char in plaintext to its ascii value (make sure has 3
digits; add leading 0s if need be). So that first letter isn't mis
interpreted if leading 0s are chopped off by BIGNUM, pad text with
the first letter of BIGNUM padded text as a 100.

e.g.
|         B     U     G
v  100   102   125   107

appending the numbers as a big string will clearly create a real
value that can be operated on, and the inverse is to convert each
3 digit chunck into 1 letter.

BUT: will BIGNUM have any of this shit?
*/
